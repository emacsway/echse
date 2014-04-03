/*** evrrul.c -- recurrence rules
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
#include <stdlib.h>
#include <string.h>
#include "evrrul.h"
#include "nifty.h"


/* generic date converters */
static const unsigned int mdays[] = {
	0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U,
};

static echs_wday_t
__get_wday(echs_instant_t i)
{
/* sakamoto method, stolen from dateutils */
	static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	unsigned int year = i.y;
	unsigned int res;

	year -= i.m < 3;
	res = year + year / 4U - year / 100U + year / 400U;
	res += t[i.m - 1U] + i.d;
	return (echs_wday_t)(((unsigned int)res % 7U) ?: SUN);
}

static echs_instant_t
__ymcw_to_inst(echs_instant_t y)
{
/* convert instant with CW coded D slot in Y to Gregorian insants. */
	unsigned int c = y.d >> 4U;
	unsigned int w = y.d & 0x07U;
	echs_instant_t res = y;
	/* get month's first's weekday */
	echs_wday_t wd1 = __get_wday((res.d = 1U, res));
	unsigned int add;
	unsigned int tgtd;

	/* now just like in the __wday_after() code, we want the number of days
	 * to add so that wd(res + X) == W above,
	 * this is a simple modulo subtraction */
	add = ((unsigned int)w + 7U - (unsigned int)wd1) % 7;
	if ((tgtd = 1U + add + (c - 1) * 7U) > mdays[res.m]) {
		if (UNLIKELY(tgtd == 29U && !(res.y % 4U))) {
			/* ah, leap year innit
			 * no need to check for Feb because months are
			 * usually longer than 29 days and we wouldn't be
			 * here if it was a different month */
			;
		} else {
			tgtd -= 7U;
		}
	}
	res.d = tgtd;
	return res;
}


/* recurrence helpers */
struct fill_1yly_s {
	unsigned int y;
	const unsigned int m;
	const int d;

	const unsigned int inter;
	const unsigned int count;
	const echs_instant_t until;
};

static size_t
fill_1yly(echs_instant_t *restrict tgt, size_t nti, struct fill_1yly_s param)
{
	size_t tries = nti;
	echs_instant_t until = param.until;
	size_t res;

	if (UNLIKELY(param.count < nti)) {
		nti = param.count;
	}
	for (res = 0U; res < nti && --tries > 0U; param.y += param.inter) {
		int d;

		d = param.d;
		if (d > 0 && (unsigned int)d <= mdays[param.m]) {
			;
		} else if (d < 0 && mdays[param.m] + 1U + d > 0) {
			d += mdays[param.m] + 1U;
		} else if (UNLIKELY(param.m == 2U && !(param.y % 4U))) {
			/* fix up leap years */
			switch (d) {
			case -29:
				d = 1;
			case 29:
				break;
			default:
				goto invalid;
			}
		} else {
		invalid:
			continue;
		}
		/* assign and increment */
		tgt[res].y = param.y;
		tgt[res].m = param.m;
		tgt[res].d = (unsigned int)d;

		if (UNLIKELY(echs_instant_lt_p(until, tgt[res]))) {
			break;
		}
		res++;
		tries = nti;
	}
	return res;
}

size_t
rrul_fill_yly(echs_instant_t *restrict tgt, size_t nti, rrulsp_t rr)
{
	unsigned int y = tgt->y;
	unsigned int m[12U];
	size_t nm;
	int d[12U];
	size_t nd;
	size_t res = 0UL;

	with (unsigned int tmpm) {
		nm = 0UL;
		for (bitint_iter_t mi = 0U;
		     nm < countof(m) && (tmpm = bui31_next(&mi, rr->mon), mi);
		     m[nm++] = tmpm);

		/* fill up with a default */
		if (!nm) {
			m[nm++] = tgt->m;
		}
	}
	with (int tmpd) {
		nd = 0UL;
		for (bitint_iter_t di = 0U;
		     nd < nti && (tmpd = bi31_next(&di, rr->dom), di);
		     d[nd++] = tmpd + 1);

		/* fill up with the default */
		if (!nd) {
			d[nd++] = tgt->d;
		}
	}

	if (LIKELY(nd == 1UL && nm == 1UL)) {
		/* resort to standard refiller */
		struct fill_1yly_s param = {
			y, *m, *d,
			.count = rr->count,
			.inter = rr->inter,
			.until = rr->until,
		};
		return fill_1yly(tgt, nti, param);
	}

	/* now just fill up the array */
	for (;; y++) {
		for (size_t i = 0UL; i < nm; i++) {
			for (size_t j = 0UL; j < nd; j++) {
				tgt[res].y = y;
				tgt[res].m = m[i];
				if (d[j] > 0) {
					tgt[res].d = d[j];
				} else {
					tgt[res].d = mdays[m[i]] + 1U + d[j];
				}
				tgt[res] = echs_instant_fixup(tgt[res]);
				if (++res >= nti) {
					goto fin;
				}
			}
		}
	}
fin:
	return res;
}

/* evrrul.c ends here */