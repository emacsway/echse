/*** dt-strpf.c -- parser and formatter funs for echse
 *
 * Copyright (C) 2011-2014 Sebastian Freundt
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
 **/
/* implementation part of date-core-strpf.h */
#if !defined INCLUDED_dt_strpf_c_
#define INCLUDED_dt_strpf_c_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdint.h>
#include "dt-strpf.h"
#include "nifty.h"

static size_t
ui32tostr(char *restrict buf, size_t bsz, uint32_t d, int pad)
{
/* all strings should be little */
#define C(x, d)	(char)((x) / (d) % 10 + '0')
	size_t res;

	if (UNLIKELY(d > 1000000000)) {
		return 0;
	}
	switch ((res = (size_t)pad) < bsz ? res : bsz) {
	case 9:
		/* for nanoseconds */
		buf[pad - 9] = C(d, 100000000);
		buf[pad - 8] = C(d, 10000000);
		buf[pad - 7] = C(d, 1000000);
	case 6:
		/* for microseconds */
		buf[pad - 6] = C(d, 100000);
		buf[pad - 5] = C(d, 10000);
	case 4:
		/* for western year numbers */
		buf[pad - 4] = C(d, 1000);
	case 3:
		/* for milliseconds or doy et al. numbers */
		buf[pad - 3] = C(d, 100);
	case 2:
		/* hours, mins, secs, doms, moys, etc. */
		buf[pad - 2] = C(d, 10);
	case 1:
		buf[pad - 1] = C(d, 1);
		break;
	default:
	case 0:
		res = 0;
		break;
	}
	return res;
}


echs_instant_t
dt_strp(const char *str)
{
/* code dupe, see __strpd_std() */
	echs_instant_t res = {.u = 0U};
	unsigned int tmp = 0U;
	const char *sp;

	if (UNLIKELY((sp = str) == NULL)) {
		goto nul;
	}
	/* read the year */
	for (; *sp >= '0' && *sp <= '9'; sp++) {
		tmp *= 10U;
		tmp += *sp - '0';
	}
	switch (*sp++) {
	case '-':
		/* just the year then? */
		if (tmp < 1583U || tmp > 4095U) {
			goto nul;
		}
		/* yep, just the year */
		res.y = tmp;

		/* snarf month */
		switch (*sp++) {
		case '0':
			if (*sp > '0' && *sp <= '9') {
				tmp = *sp++ - '0';
				break;
			}
			goto nul;
		case '1':
			if (*sp >= '0' && *sp <= '2') {
				tmp = 10U + *sp++ - '0';
				break;
			}
			goto nul;
		default:
			goto nul;
		}
		res.m = tmp;

		/* snarf mday, ... if there's a separator */
		if (*sp++ != '-') {
			goto nul;
		}
		switch (*sp++) {
		case '0':
			tmp = 0U;
			break;
		case '1':
			tmp = 10U;
			break;
		case '2':
			tmp = 20U;
			break;
		case '3':
			tmp = 30U;
			break;
		default:
			goto nul;
		}
		if (*sp >= '0' && *sp <= '9') {
			tmp += *sp++ - '0';
			res.d = tmp;
		} else {
			goto nul;
		}
		if (*sp != 'T' && *sp != ' ') {
			res.H = ECHS_ALL_DAY;
			goto res;
		}
		sp++;
		/* fallthrough */
	case 'T':
	case ' ':
		if (tmp >= 15830101U && tmp <= 40951231U) {
			/* in case we had no separators */
			res.y = tmp / 10000U;
			res.m = (tmp / 100U) % 100U;
			res.d = (tmp % 100U);
		}
		break;

	default:
		if (tmp >= 15830101U && tmp <= 40951231U) {
			res.y = tmp / 10000U;
			res.m = (tmp / 100U) % 100U;
			res.d = (tmp % 100U);
			res.H = ECHS_ALL_DAY;
		}
		goto res;
	}

	/* and now parse the time */
	for (tmp = 0U; *sp >= '0' && *sp <= '9'; sp++) {
		tmp *= 10U;
		tmp += *sp - '0';
	}
	switch (*sp++) {
	case ':':
		/* oh we're doing it the slow way */
		if (tmp > 23U) {
			goto nul;
		}
		res.H = tmp;

		/* parse minute */
		if (sp[0] >= '0' && sp[0] <= '5' &&
		    sp[1] >= '0' && sp[1] <= '9') {
			res.M = 10 * (sp[0] - '0') + sp[1] - '0';
		} else {
			goto nul;
		}
		if (sp[2] != ':') {
			goto nul;
		}
		sp += 3;
		/* parse second */
		if ((sp[0] >= '0' && sp[0] <= '5' &&
		     sp[1] >= '0' && sp[1] <= '9') ||
		    (sp[0] == '6' && sp[1] == '0')) {
			res.S = 10 * (sp[0] - '0') + sp[1] - '0';
		} else {
			goto nul;
		}
		if (sp[2] != '.') {
			res.ms = ECHS_ALL_SEC;
			goto res;
		}
		/* fallthrough */
		sp += 3;
	case '.':
		if (tmp <= 235960U) {
			res.H = tmp / 10000U;
			res.M = (tmp / 100U) % 100U;
			res.S = tmp % 100U;
			res.ms = ECHS_ALL_SEC;
		}
		break;
	default:
		if (tmp <= 235960U) {
			res.H = tmp / 10000U;
			res.M = (tmp / 100U) % 100U;
			res.S = tmp % 100U;
			res.ms = ECHS_ALL_SEC;
		}
		goto res;
	}

	/* millisecond part */
	for (tmp = 0U; tmp < 100U && *sp >= '0' && *sp <= '9'; sp++) {
		tmp *= 10U;
		tmp += *sp - '0';
	}
	res.ms = tmp;
res:
	return res;
nul:
	return (echs_instant_t){.u = 0U};
}

size_t
dt_strf(char *restrict buf, size_t bsz, echs_instant_t inst)
{
	char *restrict bp = buf;
#define bz	(bsz - (bp - buf))

	bp += ui32tostr(bp, bz, inst.y, 4);
	*bp++ = '-';
	bp += ui32tostr(bp, bz, inst.m, 2);
	*bp++ = '-';
	bp += ui32tostr(bp, bz, inst.d, 2);

	if (LIKELY(!echs_instant_all_day_p(inst))) {
		*bp++ = 'T';
		bp += ui32tostr(bp, bz, inst.H, 2);
		*bp++ = ':';
		bp += ui32tostr(bp, bz, inst.M, 2);
		*bp++ = ':';
		bp += ui32tostr(bp, bz, inst.S, 2);
		if (LIKELY(!echs_instant_all_sec_p(inst))) {
			*bp++ = '.';
			bp += ui32tostr(bp, bz, inst.ms, 3);
		}
	}
	*bp = '\0';
	return bp - buf;
}

#endif	/* INCLUDED_dt_strpf_c_ */