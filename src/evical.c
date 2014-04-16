/*** evical.c -- simple icalendar parser for echse
 *
 * Copyright (C) 2013-2014 Sebastian Freundt
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
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "evical.h"
#include "intern.h"
#include "bufpool.h"
#include "bitint.h"
#include "dt-strpf.h"
#include "evrrul.h"
#include "nifty.h"
#include "evical-gp.c"
#include "evrrul-gp.c"

#if !defined assert
# define assert(x)
#endif	/* !assert */

typedef struct vearr_s *vearr_t;
typedef uintptr_t goptr_t;

struct dtlst_s {
	size_t ndt;
	goptr_t dt;
};

struct rrlst_s {
	size_t nr;
	goptr_t r;
};

struct ical_vevent_s {
	echs_event_t ev;
	/* pointers into the global rrul/xrul array */
	struct rrlst_s rr;
	struct rrlst_s xr;
	/* points into the global xdat/rdat array */
	struct dtlst_s rd;
	struct dtlst_s xd;
};

struct vearr_s {
	size_t nev;
	struct ical_vevent_s ev[];
};


/* global rrule array */
static size_t zgrr;
static goptr_t ngrr;
static struct rrulsp_s *grr;

/* global exrule array */
static size_t zgxr;
static goptr_t ngxr;
static struct rrulsp_s *gxr;

/* global exdate array */
static size_t zgxd;
static goptr_t ngxd;
static echs_instant_t *gxd;

/* global rdate array */
static size_t zgrd;
static goptr_t ngrd;
static echs_instant_t *grd;

#define CHECK_RESIZE(id, iniz, nitems)					\
	if (UNLIKELY(!z##id)) {						\
		/* leave the first one out */				\
		id = malloc((z##id = iniz) * sizeof(*id));		\
		memset(id, 0, sizeof(*id));				\
	}								\
	if (UNLIKELY(n##id + nitems > z##id)) {				\
		do {							\
			id = realloc(id, (z##id *= 2U) * sizeof(*id));	\
		} while (n##id + nitems > z##id);			\
	}

static goptr_t
add1_to_grr(struct rrulsp_s rr)
{
	goptr_t res;

	CHECK_RESIZE(grr, 16U, 1U);
	grr[res = ngrr++] = rr;
	return res;
}

static goptr_t
add_to_grr(const struct rrulsp_s *rr, size_t nrr)
{
	goptr_t res;

	CHECK_RESIZE(grr, 16U, nrr);
	memcpy(grr + (res = ngrr), rr, nrr * sizeof(*rr));
	ngrr += nrr;
	return res;
}

static struct rrulsp_s*
get_grr(goptr_t d)
{
	if (UNLIKELY(grr == NULL)) {
		return NULL;
	}
	return grr + d;
}

static goptr_t
add1_to_gxr(struct rrulsp_s xr)
{
	goptr_t res;

	CHECK_RESIZE(gxr, 16U, 1U);
	gxr[res = ngxr++] = xr;
	return res;
}

static goptr_t
add_to_gxr(const struct rrulsp_s *xr, size_t nxr)
{
	goptr_t res;

	CHECK_RESIZE(gxr, 16U, nxr);
	memcpy(gxr + (res = ngxr), xr, nxr * sizeof(*xr));
	ngxr += nxr;
	return res;
}

static struct rrulsp_s*
get_gxr(goptr_t d)
{
	if (UNLIKELY(gxr == NULL)) {
		return NULL;
	}
	return gxr + d;
}

static goptr_t
add_to_gxd(struct dtlst_s xdlst)
{
	const echs_instant_t *dp = (const echs_instant_t*)xdlst.dt;
	const size_t nd = xdlst.ndt;
	goptr_t res;

	CHECK_RESIZE(gxd, 64U, nd);
	memcpy(gxd + ngxd, dp, nd * sizeof(*dp));
	res = ngxd, ngxd += nd;
	return res;
}

static echs_instant_t*
get_gxd(goptr_t d)
{
	if (UNLIKELY(gxd == NULL)) {
		return NULL;
	}
	return gxd + d;
}

static goptr_t
add_to_grd(struct dtlst_s rdlst)
{
	const echs_instant_t *dp = (const echs_instant_t*)rdlst.dt;
	const size_t nd = rdlst.ndt;
	goptr_t res;

	CHECK_RESIZE(grd, 64U, nd);
	memcpy(grd + ngrd, dp, nd * sizeof(*dp));
	res = ngrd, ngrd += nd;
	return res;
}


static echs_instant_t
snarf_value(const char *s)
{
	echs_instant_t res = {.u = 0U};
	const char *sp;

	if (LIKELY((sp = strchr(s, ':')) != NULL)) {
		res = dt_strp(sp + 1);
	}
	return res;
}

static echs_freq_t
snarf_freq(const char *spec)
{
	switch (*spec) {
	case 'Y':
		return FREQ_YEARLY;
	case 'M':
		if (UNLIKELY(spec[1] == 'I')) {
			return FREQ_MINUTELY;
		}
		return FREQ_MONTHLY;
	case 'W':
		return FREQ_WEEKLY;
	case 'D':
		return FREQ_DAILY;
	case 'H':
		return FREQ_HOURLY;
	case 'S':
		return FREQ_SECONDLY;
	default:
		break;
	}
	return FREQ_NONE;
}

static echs_wday_t
snarf_wday(const char *s)
{
	switch (*s) {
	case 'M':
		return MON;
	case 'W':
		return WED;
	case 'F':
		return FRI;

	case 'R':
		return THU;
	case 'A':
		return SAT;

	case 'T':
		if (s[1U] == 'H') {
			return THU;
		}
		return TUE;
	case 'S':
		if (s[1U] == 'A') {
			return SAT;
		}
		return SUN;
	default:
		break;
	}
	return MIR;
}

static struct rrulsp_s
snarf_rrule(const char *s, size_t z)
{
	struct rrulsp_s rr = {
		.freq = FREQ_NONE,
		.count = -1U,
		.inter = 1U,
		.until = echs_max_instant(),
	};

	for (const char *sp = s, *const ep = s + z, *eofld;
	     sp < ep; sp = eofld + 1) {
		const struct rrul_key_cell_s *c;
		const char *kv;
		size_t kz;

		if (UNLIKELY((eofld = strchr(sp, ';')) == NULL)) {
			eofld = ep;
		}
		/* find the key-val separator (=) */
		if (UNLIKELY((kv = strchr(sp, '=')) == NULL)) {
			kz = eofld - sp;
		} else {
			kz = kv - sp;
		}
		/* try a lookup */
		if (UNLIKELY((c = __evrrul_key(sp, kz)) == NULL)) {
			/* not found */
			continue;
		}
		/* otherwise do a bit of inspection */
		switch (c->key) {
			long int tmp;
			char *on;
		case KEY_FREQ:
			/* read the spec as well */
			rr.freq = snarf_freq(++kv);
			break;

		case KEY_COUNT:
		case KEY_INTER:
			if (!(tmp = atol(++kv))) {
				goto bogus;
			}
			switch (c->key) {
			case KEY_COUNT:
				rr.count = (unsigned int)tmp;
				break;
			case KEY_INTER:
				rr.inter = (unsigned int)tmp;
				break;
			}
			break;

		case KEY_UNTIL:
			rr.until = dt_strp(++kv);
			break;

		case BY_WDAY:
			/* this one's special in that weekday names
			 * are allowed to follow the indicator number */
			do {
				echs_wday_t w;

				tmp = strtol(++kv, &on, 10);
				if ((w = snarf_wday(on)) == MIR) {
					continue;
				}
				/* otherwise assign */
				ass_bi447(&rr.dow, pack_cd(CD(tmp, w)));
			} while ((kv = strchr(on, ',')) != NULL);
			break;
		case BY_MON:
		case BY_HOUR:
		case BY_MIN:
		case BY_SEC:
			do {
				tmp = strtoul(++kv, &on, 10);
				switch (c->key) {
				case BY_MON:
					rr.mon = ass_bui31(rr.mon, tmp);
					break;
				case BY_HOUR:
					rr.H = ass_bui31(rr.H, tmp);
					break;
				case BY_MIN:
					rr.M = ass_bui63(rr.M, tmp);
					break;
				case BY_SEC:
					rr.S = ass_bui63(rr.S, tmp);
					break;
				}
			} while (*(kv = on) == ',');
			break;

		case BY_MDAY:
		case BY_WEEK:
		case BY_YDAY:
		case BY_POS:
		case BY_EASTER:
		case BY_ADD:
			/* these ones take +/- values */
			do {
				tmp = strtol(++kv, &on, 10);
				switch (c->key) {
				case BY_MDAY:
					rr.dom = ass_bi31(rr.dom, tmp - 1);
					break;
				case BY_WEEK:
					rr.wk = ass_bi63(rr.wk, tmp);
					break;
				case BY_YDAY:
					ass_bi383(&rr.doy, tmp);
					break;
				case BY_POS:
					ass_bi383(&rr.pos, tmp);
					break;
				case BY_EASTER:
					ass_bi383(&rr.easter, tmp);
					break;
				case BY_ADD:
					ass_bi383(&rr.add, tmp);
					break;
				}
			} while (*(kv = on) == ',');
			break;

		default:
		case KEY_UNK:
			break;
		}
	}
	return rr;
bogus:
	return (struct rrulsp_s){FREQ_NONE};
}

static struct dtlst_s
snarf_dtlst(const char *line, size_t llen)
{
	static size_t zdt;
	static echs_instant_t *dt;
	size_t ndt = 0UL;

	for (const char *sp = line, *const ep = line + llen, *eod;
	     sp < ep; sp = eod + 1U) {
		echs_instant_t in;

		if (UNLIKELY((eod = strchr(sp, ',')) == NULL)) {
			eod = ep;
		}
		if (UNLIKELY(echs_instant_0_p(in = dt_strp(sp)))) {
			continue;
		}
		CHECK_RESIZE(dt, 64U, 1U);
		dt[ndt++] = in;
	}
	return (struct dtlst_s){ndt, (goptr_t)dt};
}

static void
snarf_fld(struct ical_vevent_s ve[static 1U], const char *line, size_t llen)
{
	const char *lp;
	const char *ep = line + llen;
	const struct ical_fld_cell_s *c;

	if (UNLIKELY((lp = strpbrk(line, ":;")) == NULL)) {
		return;
	} else if (UNLIKELY((c = __evical_fld(line, lp - line)) == NULL)) {
		return;
	}
	/* do more string inspection here */
	if (LIKELY(ep[-1] == '\n')) {
		ep--;
	}
	if (UNLIKELY(ep[-1] == '\r')) {
		ep--;
	}

	switch (c->fld) {
	default:
	case FLD_UNK:
		/* how did we get here */
		return;
	case FLD_DTSTART:
	case FLD_DTEND:
		with (echs_instant_t i = snarf_value(lp)) {
			switch (c->fld) {
			case FLD_DTSTART:
				ve->ev.from = i;
				break;
			case FLD_DTEND:
				ve->ev.till = i;
				break;
			}
		}
		break;
	case FLD_XDATE:
	case FLD_RDATE:
		if (UNLIKELY(*lp++ != ':' && (lp = strchr(lp, ':')) == NULL)) {
			break;
		}
		/* otherwise snarf */
		with (struct dtlst_s l = snarf_dtlst(lp, ep - lp)) {
			if (l.ndt == 0UL) {
				break;
			}
			switch (c->fld) {
				goptr_t go;
			case FLD_XDATE:
				go = add_to_gxd(l);
				if (!ve->xd.ndt) {
					ve->xd.dt = go;
				}
				ve->xd.ndt += l.ndt;
				break;
			case FLD_RDATE:
				go = add_to_grd(l);
				if (!ve->rd.ndt) {
					ve->rd.dt = go;
				}
				ve->xd.ndt += l.ndt;
				break;
			}
		}
		break;
	case FLD_RRULE:
	case FLD_XRULE:
		if (UNLIKELY(*lp++ != ':' && (lp = strchr(lp, ':')) == NULL)) {
			break;
		}
		/* otherwise snarf him */
		for (struct rrulsp_s r;
		     (r = snarf_rrule(lp, ep - lp)).freq != FREQ_NONE;) {
			goptr_t x;

			switch (c->fld) {
			case FLD_RRULE:
				/* bang to global array */
				x = add1_to_grr(r);

				if (!ve->rr.nr++) {
					ve->rr.r = x;
				}
				break;
			case FLD_XRULE:
				/* bang to global array */
				x = add1_to_gxr(r);

				if (!ve->xr.nr++) {
					ve->xr.r = x;
				}
				break;
			}
			/* this isn't supposed to be a for-loop */
			break;
		}
		break;
	case FLD_UID:
	case FLD_SUMM:
		if (UNLIKELY(*lp++ != ':' && (lp = strchr(lp, ':')) == NULL)) {
			break;
		}
		with (obint_t ob = intern(lp, ep - lp)) {
			switch (c->fld) {
			case FLD_UID:
				ve->ev.uid = ob;
				break;
			case FLD_SUMM:
				ve->ev.sum = ob;
				break;
			}
		}
		break;
	case FLD_DESC:
		if (UNLIKELY(*lp++ != ':' && (lp = strchr(lp, ':')) == NULL)) {
			break;
		}
#if 0
/* we used to have a desc slot, but that went in favour of a uid slot */
		ve->ev.desc = bufpool(lp, ep - lp).str;
#endif	/* 0 */
		break;
	}
	return;
}

static vearr_t
read_ical(const char *fn)
{
	FILE *fp;
	char *line = NULL;
	size_t llen = 0U;
	enum {
		ST_UNK,
		ST_VEVENT,
	} st = ST_UNK;
	struct ical_vevent_s ve;
	size_t nve = 0UL;
	vearr_t a = NULL;

	if ((fp = fopen(fn, "r")) == NULL) {
		return NULL;
	}

	/* little probe first
	 * luckily BEGIN:VCALENDAR\n is exactly 16 bytes */
	with (ssize_t nrd) {
		static const char hdr[] = "BEGIN:VCALENDAR";

		if (UNLIKELY((nrd = getline(&line, &llen, fp)) <= 0)) {
			/* must be bollocks then */
			goto clo;
		} else if ((size_t)nrd < sizeof(hdr)) {
			/* still looks like bollocks */
			goto clo;
		} else if (memcmp(line, hdr, sizeof(hdr) - 1)) {
			/* also bollocks */
			goto clo;
		}
		/* otherwise, this looks legit
		 * oh, but keep your fingers crossed anyway */
	}

	for (ssize_t nrd; (nrd = getline(&line, &llen, fp)) > 0;) {
		switch (st) {
			static const char beg[] = "BEGIN:VEVENT";
			static const char end[] = "END:VEVENT";
		default:
		case ST_UNK:
			/* check if line is a new vevent */
			if (!strncmp(line, beg, sizeof(beg) - 1)) {
				/* yep, set state to vevent */
				st = ST_VEVENT;
				/* and rinse our bucket */
				memset(&ve, 0, sizeof(ve));
			}
			break;
		case ST_VEVENT:
			if (!strncmp(line, end, sizeof(end) - 1)) {
				/* oh, ok, better stop the parsing then
				 * and reset the state machine */
				if (a == NULL || nve >= a->nev) {
					/* resize */
					const size_t nu = 2 * nve ?: 64U;
					size_t nz = nu * sizeof(*a->ev);

					a = realloc(a, nz + sizeof(a));
					a->nev = nu;
				}
				/* assign */
				a->ev[nve++] = ve;
				/* reset to unknown state */
				st = ST_UNK;
				break;
			}
			/* otherwise interpret the line */
			snarf_fld(&ve, line, nrd);
			break;
		}
	}
	/* massage result array */
	if (LIKELY(a != NULL)) {
		a->nev = nve;
	}
	/* clean up reader resources */
	free(line);
clo:
	fclose(fp);
	return a;
}

static void
prnt_ical_hdr(void)
{
	static time_t now;
	static char stmp[32U];

	puts("BEGIN:VEVENT");
	if (LIKELY(now)) {
		;
	} else {
		struct tm tm[1U];

		if (LIKELY((now = time(NULL), gmtime_r(&now, tm) != NULL))) {
			echs_instant_t nowi = {
				.y = tm->tm_year + 1900,
				.m = tm->tm_mon + 1,
				.d = tm->tm_mday,
				.H = tm->tm_hour,
				.M = tm->tm_min,
				.S = tm->tm_sec,
				.ms = ECHS_ALL_SEC,
			};

			dt_strf_ical(stmp, sizeof(stmp), nowi);
		} else {
			/* screw up the singleton */
			now = 0;
			return;
		}
	}
	fputs("DTSTAMP:", stdout);
	puts(stmp);
	return;
}

static void
prnt_ical_ftr(void)
{
	puts("END:VEVENT");
	return;
}

static void
prnt_cd(struct cd_s cd)
{
	static const char *w[] = {
		"MI", "MO", "TU", "WE", "TH", "FR", "SA", "SU"
	};

	if (cd.cnt) {
		fprintf(stdout, "%d", cd.cnt);
	}
	fputs(w[cd.dow], stdout);
	return;
}

static void
prnt_rrul(rrulsp_t rr)
{
	switch (rr->freq) {
	case FREQ_YEARLY:
		fputs("FREQ=YEARLY", stdout);
		break;
	default:
		break;
	}
	if (rr->inter > 1U) {
		fprintf(stdout, ";INTERVAL=%u", rr->inter);
	}
	with (unsigned int m) {
		bitint_iter_t i = 0UL;

		if (!bui31_has_bits_p(rr->mon)) {
			break;
		}
		m = bui31_next(&i, rr->mon);
		fprintf(stdout, ";BYMONTH=%u", m);
		while (m = bui31_next(&i, rr->mon), i) {
			fprintf(stdout, ",%u", m);
		}
	}

	with (int yw) {
		bitint_iter_t i = 0UL;

		if (!bi63_has_bits_p(rr->wk)) {
			break;
		}
		yw = bi63_next(&i, rr->wk);
		fprintf(stdout, ";BYWEEKNO=%d", yw);
		while (yw = bi63_next(&i, rr->wk), i) {
			fprintf(stdout, ",%d", yw);
		}
	}

	with (int yd) {
		bitint_iter_t i = 0UL;

		if (!bi383_has_bits_p(&rr->doy)) {
			break;
		}
		yd = bi383_next(&i, &rr->doy);
		fprintf(stdout, ";BYYEARDAY=%d", yd);
		while (yd = bi383_next(&i, &rr->doy), i) {
			fprintf(stdout, ",%d", yd);
		}
	}

	with (int d) {
		bitint_iter_t i = 0UL;

		if (!bi31_has_bits_p(rr->dom)) {
			break;
		}
		d = bi31_next(&i, rr->dom);
		fprintf(stdout, ";BYMONTHDAY=%d", d + 1);
		while (d = bi31_next(&i, rr->dom), i) {
			fprintf(stdout, ",%d", d + 1);
		}
	}

	with (int e) {
		bitint_iter_t i = 0UL;

		if (!bi383_has_bits_p(&rr->easter)) {
			break;
		}
		e = bi383_next(&i, &rr->easter);
		fprintf(stdout, ";BYEASTER=%d", e);
		while (e = bi383_next(&i, &rr->easter), i) {
			fprintf(stdout, ",%d", e);
		}
	}

	with (struct cd_s cd) {
		bitint_iter_t i = 0UL;

		if (!bi447_has_bits_p(&rr->dow)) {
			break;
		}
		cd = unpack_cd(bi447_next(&i, &rr->dow));
		fputs(";BYDAY=", stdout);
		prnt_cd(cd);
		while (cd = unpack_cd(bi447_next(&i, &rr->dow)), i) {
			fputc(',', stdout);
			prnt_cd(cd);
		}
	}

	with (int a) {
		bitint_iter_t i = 0UL;

		if (!bi383_has_bits_p(&rr->add)) {
			break;
		}
		a = bi383_next(&i, &rr->add);
		fprintf(stdout, ";BYADD=%d", a);
		while (a = bi383_next(&i, &rr->add), i) {
			fprintf(stdout, ",%d", a);
		}
	}

	with (int p) {
		bitint_iter_t i = 0UL;

		if (!bi383_has_bits_p(&rr->pos)) {
			break;
		}
		p = bi383_next(&i, &rr->pos);
		fprintf(stdout, ";BYPOS=%d", p);
		while (p = bi383_next(&i, &rr->pos), i) {
			fprintf(stdout, ",%d", p);
		}
	}

	if ((int)rr->count > 0) {
		fprintf(stdout, ";COUNT=%u", rr->count);
	}
	if (rr->until.u < -1ULL) {
		char until[32U];

		dt_strf_ical(until, sizeof(until), rr->until);
		fprintf(stdout, ";UNTIL=%s", until);
	}

	fputc('\n', stdout);
	return;
}


/* our event class */
struct evical_s {
	echs_evstrm_class_t class;
	/* our iterator state */
	size_t i;
	/* array size and data */
	size_t nev;
	echs_event_t ev[];
};

static echs_event_t next_evical_vevent(echs_evstrm_t);
static void free_evical_vevent(echs_evstrm_t);
static echs_evstrm_t clone_evical_vevent(echs_evstrm_t);
static void prnt_evical_vevent(echs_evstrm_t);

static const struct echs_evstrm_class_s evical_cls = {
	.next = next_evical_vevent,
	.free = free_evical_vevent,
	.clone = clone_evical_vevent,
	.prnt1 = prnt_evical_vevent,
};

static const echs_event_t nul;

static echs_evstrm_t
make_evical_vevent(const echs_event_t *ev, size_t nev)
{
	const size_t zev = nev * sizeof(*ev);
	struct evical_s *res = malloc(sizeof(*res) + zev);

	res->class = &evical_cls;
	res->i = 0U;
	res->nev = nev;
	memcpy(res->ev, ev, zev);
	return (echs_evstrm_t)res;
}

static void
free_evical_vevent(echs_evstrm_t s)
{
	struct evical_s *this = (struct evical_s*)s;

	free(this);
	return;
}

static echs_evstrm_t
clone_evical_vevent(echs_evstrm_t s)
{
	struct evical_s *this = (struct evical_s*)s;

	return make_evical_vevent(this->ev + this->i, this->nev - this->i);
}

static echs_event_t
next_evical_vevent(echs_evstrm_t s)
{
	struct evical_s *this = (struct evical_s*)s;

	if (UNLIKELY(this->i >= this->nev)) {
		return nul;
	}
	return this->ev[this->i++];
}

static void
prnt_evical_vevent(echs_evstrm_t s)
{
	const struct evical_s *this = (struct evical_s*)s;

	for (size_t i = 0UL; i < this->nev; i++) {
		const echs_event_t ev = this->ev[i];
		char from[32U];
		char till[32U];

		prnt_ical_hdr();
		dt_strf_ical(from, sizeof(from), ev.from);
		dt_strf_ical(till, sizeof(till), ev.till);
		fputs("DTSTART:", stdout);
		puts(from);
		fputs("DTEND:", stdout);
		puts(till);
		if (ev.uid) {
			fputs("UID:", stdout);
			puts(obint_name(ev.uid));
		}
		if (ev.sum) {
			fputs("SUMMARY:", stdout);
			puts(obint_name(ev.sum));
		}
		prnt_ical_ftr();
	}
	return;
}


struct evrrul_s {
	echs_evstrm_class_t class;
	/* the actual complete vevent (includes proto-event) */
	struct ical_vevent_s ve;
	/* proto-duration */
	echs_idiff_t dur;
	/* iterator state */
	size_t rdi;
	/* unrolled cache */
	size_t ncch;
	echs_instant_t cch[64U];
};

static echs_event_t next_evrrul(echs_evstrm_t);
static void free_evrrul(echs_evstrm_t);
static echs_evstrm_t clone_evrrul(echs_evstrm_t);
static void prnt_evrrul1(echs_evstrm_t);
static void prnt_evrrulm(const echs_evstrm_t s[], size_t n);

static const struct echs_evstrm_class_s evrrul_cls = {
	.next = next_evrrul,
	.free = free_evrrul,
	.clone = clone_evrrul,
	.prnt1 = prnt_evrrul1,
	.prntm = prnt_evrrulm,
};

static echs_evstrm_t
__make_evrrul(const struct ical_vevent_s *ve)
{
	struct evrrul_s *res = malloc(sizeof(*res));

	res->class = &evrrul_cls;
	res->ve = *ve;
	res->rdi = 0UL;
	res->ncch = 0UL;
	return (echs_evstrm_t)res;
}

static echs_evstrm_t
make_evrrul(const struct ical_vevent_s *ve)
{
	switch (ve->rr.nr) {
	case 0:
		return NULL;
	case 1:
		return __make_evrrul(ve);
	default:
		break;
	}
	with (echs_evstrm_t s[ve->rr.nr]) {
		size_t nr = 0UL;

		for (size_t i = 0U; i < ve->rr.nr; i++) {
			struct ical_vevent_s ve_tmp = *ve;

			ve_tmp.rr.r += i;
			ve_tmp.rr.nr = 1U;
			s[nr++] = __make_evrrul(&ve_tmp);
		}

		return make_echs_evmux(s, nr);
	}
	return NULL;
}

static void
free_evrrul(echs_evstrm_t s)
{
	struct evrrul_s *this = (struct evrrul_s*)s;

	free(this);
	return;
}

static echs_evstrm_t
clone_evrrul(echs_evstrm_t s)
{
	struct evrrul_s *this = (struct evrrul_s*)s;
	struct evrrul_s *clon = malloc(sizeof(*this));

	*clon = *this;
	/* we have to clone rrules and exrules though */
	if (this->ve.rr.nr) {
		const size_t nr = this->ve.rr.nr;
		const struct rrulsp_s *r = get_grr(this->ve.rr.r);

		clon->ve.rr.r = add_to_grr(r, nr);
	}
	if (this->ve.xr.nr) {
		const size_t nr = this->ve.xr.nr;
		const struct rrulsp_s *r = get_gxr(this->ve.xr.r);

		clon->ve.xr.r = add_to_gxr(r, nr);
	}
	return (echs_evstrm_t)clon;
}

/* this should be somewhere else, evrrul.c maybe? */
static size_t
refill(struct evrrul_s *restrict strm)
{
/* useful table at:
 * http://icalevents.com/2447-need-to-know-the-possible-combinations-for-repeating-dates-an-ical-cheatsheet/
 * we're trying to follow that one closely. */
	struct rrulsp_s *restrict rr;

	assert(strm->vr.rr.nr > 0UL);
	if (UNLIKELY((rr = get_grr(strm->ve.rr.r)) == NULL)) {
		return 0UL;
	} else if (UNLIKELY(!rr->count)) {
		return 0UL;
	}

	/* fill up with the proto instant */
	for (size_t j = 0U; j < countof(strm->cch); j++) {
		strm->cch[j] = strm->ve.ev.from;
	}

	/* now go and see who can help us */
	switch (rr->freq) {
	default:
		strm->ncch = 0UL;
		break;

	case FREQ_YEARLY:
		/* easiest */
		strm->ncch = rrul_fill_yly(strm->cch, countof(strm->cch), rr);
		break;
	}

	if (strm->ncch < countof(strm->cch)) {
		rr->count = 0;
	} else {
		/* update the rrule to the partial set we've got */
		rr->count -= --strm->ncch;
		strm->ve.ev.from = strm->cch[strm->ncch];
	}
	if (UNLIKELY(strm->ncch == 0UL)) {
		return 0UL;
	}
	/* otherwise sort the array, just in case */
	echs_instant_sort(strm->cch, strm->ncch);

	/* also check if we need to rid some dates due to xdates */
	if (UNLIKELY(strm->ve.xd.ndt > 0UL)) {
		const echs_instant_t *xd = get_gxd(strm->ve.xd.dt);
		echs_instant_t *restrict rd = strm->cch;
		const echs_instant_t *const exd = xd + strm->ve.xd.ndt;
		const echs_instant_t *const erd = rd + strm->ncch;

		assert(xd != NULL);
		for (; xd < exd; xd++, rd++) {
			/* fast forward to xd (we assume it's sorted) */
			for (; rd < erd && echs_instant_lt_p(*rd, *xd); rd++);
			/* now we're either on xd or past it */
			if (echs_instant_eq_p(*rd, *xd)) {
				/* leave rd out then */
				*rd = echs_nul_instant();
			}
		}
	}
	return strm->ncch;
}

static echs_event_t
next_evrrul(echs_evstrm_t s)
{
	struct evrrul_s *restrict this = (struct evrrul_s*)s;
	echs_event_t res;
	echs_instant_t in;

	/* it's easier when we just have some precalc'd rdates */
	if (this->rdi >= this->ncch) {
	refill:
		/* we have to refill the rdate cache */
		if (refill(this) == 0UL) {
			goto nul;
		}
		/* reset counter */
		this->rdi = 0U;
	}
	/* get the starting instant */
	while (UNLIKELY(echs_instant_0_p(in = this->cch[this->rdi++]))) {
		/* we might have run out of steam */
		if (UNLIKELY(this->rdi >= this->ncch)) {
			goto refill;
		}
	}
	/* construct the result */
	res = this->ve.ev;
	res.from = in;
	res.till = echs_instant_add(in, this->dur);
	return res;
nul:
	return nul;
}

static void
prnt_evrrul1(echs_evstrm_t s)
{
	const struct evrrul_s *this = (struct evrrul_s*)s;
	char from[32U];
	char till[32U];

	prnt_ical_hdr();

	dt_strf_ical(from, sizeof(from), this->ve.ev.from);
	dt_strf_ical(till, sizeof(till), this->ve.ev.till);
	fputs("DTSTART:", stdout);
	puts(from);
	fputs("DTEND:", stdout);
	puts(till);

	for (size_t i = 0UL; i < this->ve.rr.nr; i++) {
		rrulsp_t rr = get_grr(this->ve.rr.r + i);

		fputs("RRULE:", stdout);
		prnt_rrul(rr);
	}
	if (this->ve.ev.uid) {
		fputs("UID:", stdout);
		puts(obint_name(this->ve.ev.uid));
	}
	if (this->ve.ev.sum) {
		fputs("SUMMARY:", stdout);
		puts(obint_name(this->ve.ev.sum));
	}
	prnt_ical_ftr();
	return;
}

static void
prnt_evrrulm(const echs_evstrm_t s[], size_t n)
{
/* we know that they all come from one ical_event_s originally,
 * just merge them temporarily in a evrrul_s object and use prnt1() */
	struct evrrul_tmp_s {
		echs_evstrm_class_t class;
		/* the actual complete vevent (includes proto-event) */
		struct ical_vevent_s ve;
		/* proto-duration */
		echs_idiff_t dur;
	};
	struct evrrul_tmp_s this = *(struct evrrul_tmp_s*)*s;

	/* have to fiddle with the nr slot, but we know rules are consecutive */
	this.ve.rr.nr = n;
	prnt_evrrul1((echs_evstrm_t)&this);
	return;
}


echs_evstrm_t
make_echs_evical(const char *fn)
{
	vearr_t a;

	if ((a = read_ical(fn)) == NULL) {
		return NULL;
	}
	/* now split into vevents and rrules */
	with (echs_evstrm_t s[a->nev]) {
		echs_event_t ev[a->nev];
		size_t nev = 0UL;
		size_t ns = 0UL;

		/* rearrange so that pure vevents sit in ev,
		 * and rrules somewhere else */
		for (size_t i = 0U; i < a->nev; i++) {
			const struct ical_vevent_s *ve = a->ev + i;
			echs_evstrm_t tmp;
			echs_instant_t *xd;

			if (!ve->rr.nr && !ve->rd.ndt) {
				/* not an rrule but a normal vevent
				 * just him to the list */
				ev[nev++] = a->ev[i].ev;
				continue;
			}
			/* it's an rrule, we won't check for
			 * exdates or exrules because they make
			 * no sense without an rrule to go with */
			/* check for exdates here, and sort them */
			if (UNLIKELY(ve->xd.ndt > 1UL &&
				     (xd = get_gxd(ve->xd.dt)) != NULL)) {
				echs_instant_sort(xd, ve->xd.ndt);
			}

			if ((tmp = make_evrrul(a->ev + i)) != NULL) {
				s[ns++] = tmp;
			}
		}

		if (nev) {
			/* sort them */
			echs_event_sort(ev, nev);
			/* and materialise into event stream */
			s[ns++] = make_evical_vevent(ev, nev);
		}
		if (UNLIKELY(!ns)) {
			break;
		} else if (ns == 1UL) {
			return *s;
		}
		return make_echs_evmux(s, ns);
	}
	return NULL;
}

/* evical.c ends here */
