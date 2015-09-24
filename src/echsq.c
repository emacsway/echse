/*** echsq.c -- echse queue manager
 *
 * Copyright (C) 2013-2015 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of echse.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#if defined HAVE_VERSION_H
# include "version.h"
#endif	/* HAVE_VERSION_H */
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <fcntl.h>
#include <poll.h>
#include <assert.h>
#if defined HAVE_PATHS_H
# include <paths.h>
#endif	/* HAVE_PATHS_H */
#include <pwd.h>
#include "nifty.h"
#include "evical.h"
#include "fdprnt.h"

extern char **environ;

#if !defined _PATH_TMP
# define _PATH_TMP	"/tmp/"
#endif	/* _PATH_TMP */


static __attribute__((format(printf, 1, 2))) void
serror(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return;
}

static inline size_t
xstrlncpy(char *restrict dst, size_t dsz, const char *src, size_t ssz)
{
	if (UNLIKELY(dsz == 0U)) {
		return 0U;
	} else if (UNLIKELY(ssz > dsz)) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}

static size_t
xstrlcpy(char *restrict dst, const char *src, size_t dsz)
{
	return xstrlncpy(dst, dsz, src, strlen(src));
}


static const char*
get_esock(bool systemp)
{
/* return a candidate for the echsd socket, system-wide or user local */
	static const char rundir[] = "/var/run";
	static const char sockfn[] = "echse/=echsd";
	struct stat st;
	static char d[PATH_MAX];
	size_t di;

	if (systemp) {
		di = xstrlcpy(d, rundir, sizeof(d));
		d[di++] = '/';
		di += xstrlcpy(d + di, sockfn, sizeof(d) - di);

		if (stat(d, &st) < 0 || !S_ISSOCK(st.st_mode)) {
			return NULL;
		}
		/* success */
		return d;
	} else {
		uid_t u = getuid();

		di = xstrlcpy(d, rundir, sizeof(d));
		d[di++] = '/';
		di += snprintf(d + di, sizeof(d) - di, "user/%u", u);
		d[di++] = '/';
		di += xstrlcpy(d + di, sockfn, sizeof(d) - di);

		if (stat(d, &st) < 0 || !S_ISSOCK(st.st_mode)) {
			goto tmpdir;
		}
		/* success */
		return d;

	tmpdir:
		di = xstrlcpy(d, _PATH_TMP, sizeof(d));
		di += xstrlcpy(d + di, sockfn, sizeof(d) - di);

		if (stat(d, &st) < 0 || !S_ISSOCK(st.st_mode)) {
			/* no more alternatives */
			return NULL;
		}
		/* success */
		return d;
	}
}

static int
make_conn(const char *path)
{
/* run LIST command on socket PATH */
	struct sockaddr_un sa = {AF_UNIX};
	int s;
	size_t z;

	if (UNLIKELY((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)) {
		return -1;
	}
	z = xstrlcpy(sa.sun_path, path, sizeof(sa.sun_path));
	z += sizeof(sa.sun_family);
	if (UNLIKELY(connect(s, (struct sockaddr*)&sa, z) < 0)) {
		goto fail;
	}
	return s;
fail:
	close(s);
	return -1;
}

static int
free_conn(int s)
{
	return close(s);
}


/* counter for outstanding requests */
static size_t nout;

static int
poll1(int fd, int timeo)
{
	static ical_parser_t rp;
	struct pollfd rfd[] = {{.fd = fd, .events = POLLIN}};
	echs_instruc_t ins;
	char buf[4096U];
	ssize_t nrd;

	if (fd < 0) {
		/* secret message to clear up the reader */
		goto clear;
	}
	/* just read them replies */
	if (poll(rfd, countof(rfd), timeo) <= 0) {
		return -1;
	}

	/* it MUST be our fd */
	assert(rfd[0U].revents);

	switch ((nrd = read(fd, buf, sizeof(buf)))) {
	default:
		if (echs_evical_push(&rp, buf, nrd) < 0) {
			/* pushing more is insane */
			break;
		}
		/*@fallthrough@*/
	case 0:
	drain:
		ins = echs_evical_pull(&rp);
		switch (ins.v) {
		case INSVERB_RESC:
			/* that means success */
			nout--;
			puts("SUCCESS");
			/* I'm sure there's more */
			goto drain;
		case INSVERB_UNSC:
			nout--;
			puts("FAILURE");
			/* there might be more */
			goto drain;
		default:
			break;
		}
		break;
	case -1:
	clear:
		ins = echs_evical_last_pull(&rp);
		switch (ins.v) {
		case INSVERB_RESC:
			/* that means success */
			nout--;
			puts("SUCCESS");
			break;
		case INSVERB_UNSC:
			nout--;
			puts("FAILURE");
			break;
		default:
			break;
		}
		break;
	}
	return 0;
}

static void
proc1(int tgt_fd, int src_fd)
{
	char buf[32768U];
	ical_parser_t pp = NULL;
	size_t nrd;

more:
	switch ((nrd = read(src_fd, buf, sizeof(buf)))) {
		echs_instruc_t ins;

	default:
		if (echs_evical_push(&pp, buf, nrd) < 0) {
			/* pushing more brings nothing */
			break;
		}
		/*@fallthrough@*/
	case 0:
		do {
			ins = echs_evical_pull(&pp);

			if (UNLIKELY(ins.v != INSVERB_SCHE)) {
				break;
			} else if (UNLIKELY(ins.t == NULL)) {
				continue;
			}
			/* and otherwise inject him */
			echs_task_icalify(tgt_fd, ins.t);
			nout++;

			poll1(tgt_fd, 0);
		} while (1);
		if (LIKELY(nrd > 0)) {
			goto more;
		}
		/*@fallthrough@*/
	case -1:
		/* last ever pull this morning */
		ins = echs_evical_last_pull(&pp);

		if (UNLIKELY(ins.v != INSVERB_SCHE)) {
			break;
		} else if (UNLIKELY(ins.t != NULL)) {
			/* that can't be right, we should have got
			 * the last task in the loop above, this means
			 * this is a half-finished thing and we don't
			 * want no half-finished things */
			free_echs_task(ins.t);
		}

		poll1(tgt_fd, 0);
		break;
	}
	return;
}


#if defined STANDALONE
#include "echsq.yucc"

static int
cmd_list(const struct yuck_cmd_list_s argi[static 1U])
{
/* try and request the queues */
	char buf[4096U];
	char cmd[64];
	ssize_t cmz;
	/* assume failure */
	int rc = 1;

	/* just build the request */
	if (argi->user_arg == NULL) {
		static const char req[] = "GET /echsq.ics HTTP/1.1\r\n\r\n";
		cmz = xstrlncpy(cmd, sizeof(cmd), req, strlenof(req));
	} else {
		uid_t u;
		char *on = NULL;

		u = strtoul(argi->user_arg, &on, 10);
		if (on == NULL || *on) {
			struct passwd *p;

			if ((p = getpwnam(argi->user_arg)) == NULL) {
				serror("\
Error: cannot resolve user name `%s'", argi->user_arg);
				return rc;
			}
			u = p->pw_uid;
		}
		cmz = snprintf(
			cmd, sizeof(cmd),
			"GET /echsq_%u.ics HTTP/1.1\r\n\r\n", u);
	}

	/* let's try the local echsd and then the system-wide one */
	with (int s) {
		const char *e;

		if (UNLIKELY((e = get_esock(false)) == NULL)) {
			break;
		} else if (UNLIKELY((s = make_conn(e)) < 0)) {
			serror("Error: cannot connect to `%s'", e);
			break;
		}
		write(s, cmd, cmz);

		for (ssize_t nrd; (nrd = read(s, buf, sizeof(buf))) > 0;) {
			write(STDOUT_FILENO, buf, nrd);
		}
		free_conn(s);
		rc = 0;
	}

	/* global queue */
	with (int s) {
		const char *e;

		if (UNLIKELY((e = get_esock(true)) == NULL)) {
			break;
		} else if (UNLIKELY((s = make_conn(e)) < 0)) {
			serror("Error: cannot connect to `%s'", e);
			break;
		}
		write(s, cmd, cmz);

		for (ssize_t nrd; (nrd = read(s, buf, sizeof(buf))) > 0;) {
			write(STDOUT_FILENO, buf, nrd);
		}
		free_conn(s);
		rc = 0;
	}
	return rc;
}

static int
cmd_next(const struct yuck_cmd_next_s argi[static 1U])
{
/* try and request the schedules */
	static const char verb[] = "GET /";
	static const char vers[] = " HTTP/1.1\r\n\r\n";
	static const char rout[] = "sched";
	char buf[4096U];
	size_t bix;
	size_t i = 0U;
	const char *e;
	uid_t u;

#define BUF	(buf + bix)
#define BSZ	(sizeof(buf) - bix)
#define CHK_BIX()				\
	if (UNLIKELY(bix > sizeof(buf))) {	\
		goto reqstr_err;		\
	} else (void)0				\

	/* first of all transmogrify user name to uid */
	if (argi->user_arg) {
		char *on = NULL;
		u = strtoul(argi->user_arg, &on, 10);

		if (on == NULL || *on) {
			struct passwd *p;

			if ((p = getpwnam(argi->user_arg)) == NULL) {
				goto pwnam_err;
			}
			u = p->pw_uid;
		}
	}

more:
	bix = 0U;
	/* right lets start with the http verb and stuff */
	bix += xstrlncpy(BUF, BSZ, verb, strlenof(verb));
	CHK_BIX();

	if (argi->user_arg) {
		bix += snprintf(BUF, BSZ, "u/%u/", u);
		CHK_BIX();
	}

	/* paste the routine we want invoked */
	bix += xstrlncpy(BUF, BSZ, rout, strlenof(rout));
	CHK_BIX();

	/* now if there's tasks put a ?tuid=x,y,z */
	if (i < argi->nargs) {
		bix += snprintf(BUF, BSZ, "?tuid=%s", argi->args[i++]);
		CHK_BIX();
	}
	for (int n; i < argi->nargs; i++, bix += n) {
		n = snprintf(BUF, BSZ, "&tuid=%s", argi->args[i]);
		if (UNLIKELY(bix + n >= sizeof(buf))) {
			/* just do him next time */
			break;
		} else if (UNLIKELY(bix + strlenof(vers) >= sizeof(buf))) {
			/* we can't have no space for the version suffix */
			break;
		}
	}
	/* version and stuff */
	bix += xstrlncpy(BUF, BSZ, vers, strlenof(vers));

	/* let's try the local echsd and then the system-wide one */
	with (int s) {
		if (UNLIKELY((e = get_esock(false)) == NULL)) {
			break;
		} else if (UNLIKELY((s = make_conn(e)) < 0)) {
			serror("Error: cannot connect to `%s'", e);
			goto conn_err;
		}
		write(s, buf, bix);

		for (ssize_t nrd; (nrd = read(s, buf, sizeof(buf))) > 0;) {
			write(STDOUT_FILENO, buf, nrd);
		}
		free_conn(s);
	}

	/* global queue */
	with (int s) {
		if (UNLIKELY((e = get_esock(true)) == NULL)) {
			break;
		} else if (UNLIKELY((s = make_conn(e)) < 0)) {
			goto conn_err;
		}
		write(s, buf, bix);

		for (ssize_t nrd; (nrd = read(s, buf, sizeof(buf))) > 0;) {
			write(STDOUT_FILENO, buf, nrd);
		}
		free_conn(s);
	}
	if (UNLIKELY(i < argi->nargs)) {
		goto more;
	}
	return 0;

pwnam_err:
	serror("\
Error: cannot resolve user name `%s'", argi->user_arg);
	return 1;

conn_err:
	serror("\
Error: cannot connect to `%s'", e);
	return 1;

reqstr_err:
	serror("\
Error: cannot build request string");
	return 1;
}

static int
cmd_add(const struct yuck_cmd_add_s argi[static 1U])
{
/* scan for BEGIN:VEVENT/END:VEVENT pairs */
	static const char hdr[] = "\
BEGIN:VCALENDAR\n\
VERSION:2.0\n\
PRODID:-//GA Financial Solutions//echse//EN\n\
METHOD:PUBLISH\n\
CALSCALE:GREGORIAN\n";
	static const char ftr[] = "\
END:VCALENDAR\n";
	const char *e;
	int s = -1;

	/* let's try the local echsd and then the system-wide one */
	if (!((e = get_esock(false)) || (e = get_esock(true)))) {
		errno = 0, serror("Error: cannot connect to echsd");
		return 1;
	} else if ((s = make_conn(e)) < 0) {
		serror("Error: cannot connect to `%s'", e);
		return 1;
	}
	/* otherwise it's time for a `yay' */
	errno = 0, serror("connected to %s ...", e);

	write(s, hdr, strlenof(hdr));
	for (size_t i = 0U; i < argi->nargs; i++) {
		int fd;

		if (UNLIKELY((fd = open(argi->args[i], O_RDONLY)) < 0)) {
			serror("\
Error: cannot open file `%s'", argi->args[i]);
			continue;
		}

		proc1(s, fd);
		close(fd);
	}
	write(s, ftr, strlenof(ftr));
	while (nout && !(poll1(s, 5000) < 0));

	free_conn(s);
	return 0;
}

static int
cmd_cancel(const struct yuck_cmd_cancel_s argi[static 1U])
{
/* scan for BEGIN:VEVENT/END:VEVENT pairs */
	static const char hdr[] = "\
BEGIN:VCALENDAR\n\
VERSION:2.0\n\
PRODID:-//GA Financial Solutions//echse//EN\n\
METHOD:CANCEL\n\
CALSCALE:GREGORIAN\n";
	static const char ftr[] = "\
END:VCALENDAR\n";
	static const char beg[] = "BEGIN:VEVENT\n";
	static const char end[] = "END:VEVENT\n";
	static const char sta[] = "STATUS:CANCELLED\n";
	const char *e;
	int s = -1;

	/* let's try the local echsd and then the system-wide one */
	if (!((e = get_esock(false)) || (e = get_esock(true)))) {
		errno = 0, serror("Error: cannot connect to echsd");
		return 1;
	} else if ((s = make_conn(e)) < 0) {
		serror("Error: cannot connect to `%s'", e);
		return 1;
	}
	/* otherwise it's time for a `yay' */
	errno = 0, serror("connected to %s ...", e);
	/* we'll be writing to S, better believe it */
	fdbang(s);

	fdwrite(hdr, strlenof(hdr));
	for (size_t i = 0U; i < argi->nargs; i++) {
		const char *tuid = argi->args[i];

		fdwrite(beg, strlenof(beg));
		fdprintf("UID:%s\n", tuid);
		fdwrite(sta, strlenof(sta));
		fdwrite(end, strlenof(end));
		nout++;
	}
	fdwrite(ftr, strlenof(ftr));

	(void)fdputc;
	fdflush();
	while (nout && !(poll1(s, 5000) < 0));

	free_conn(s);
	return 0;
}

int
main(int argc, char *argv[])
{
	/* command line options */
	yuck_t argi[1U];
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 1;
		goto out;
	}

	switch (argi->cmd) {
	case ECHSQ_CMD_NONE:
	case ECHSQ_CMD_LIST:
		/* with no command just list queues */
		rc = cmd_list((struct yuck_cmd_list_s*)argi);
		break;


	case ECHSQ_CMD_ADD:
		rc = cmd_add((struct yuck_cmd_add_s*)argi);
		break;

	case ECHSQ_CMD_CANCEL:
		rc = cmd_cancel((struct yuck_cmd_cancel_s*)argi);
		break;

	case ECHSQ_CMD_NEXT:
		rc = cmd_next((struct yuck_cmd_next_s*)argi);
		break;

	default:
		break;
	}
out:
	yuck_free(argi);
	return rc;
}
#endif	/* STANDALONE */

/* echsq.c ends here */
