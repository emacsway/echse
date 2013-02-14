/*** echse.c -- testing echse concept
 *
 * Copyright (C) 2013 Sebastian Freundt
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
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#include "echse.h"


/* christmas stream */
static echs_event_t
xmas_next(echs_instant_t i)
{
	static const struct echs_state_s on = {1, "XMAS"};
	static const struct echs_state_s off = {0, "XMAS"};
	static struct {
		struct echs_event_s e;
		echs_state_t st;
	} res;

	if (i.m < 12U || i.d < 25U) {
		res.e.when = (echs_instant_t){i.y, 12U, 25U},
		res.e.nwhat = 1U;
		res.st.s = &on;
	} else if (i.d < 26U) {
		res.e.when = (echs_instant_t){i.y, 12U, 26U};
		res.e.nwhat = 1U;
		res.st.s = &off;
	} else {
		res.e.when = (echs_instant_t){i.y + 1, 12U, 25U};
		res.e.nwhat = 1U;
		res.st.s = &on;
	}
	return &res.e;
}


/* easter stream */
static echs_instant_t
__easter(unsigned int y)
{
	/* compute gregorian easter date first */
	unsigned int a = y % 19U;
	unsigned int b = y / 4U;
	unsigned int c = b / 25U + 1;
	unsigned int d = 3U * c / 4U;
	unsigned int e;

	e = 19U * a + -((8U * c + 5) / 25U) + d + 15U;
	e %= 30U;
	e += (29578U - a - 32U * e) / 1024U;
	e = e - ((y % 7U) + b - d + e + 2) % 7U;
	return (echs_instant_t){y, e <= 31 ? 3U : 4U, e <= 31U ? e : e - 31U};
}

static echs_event_t
easter_next(echs_instant_t i)
{
	static const struct echs_state_s on = {1, "EASTER"};
	static const struct echs_state_s off = {0, "EASTER"};
	static struct {
		struct echs_event_s e;
		echs_state_t st;
	} res;

	if (i.m >= 5U) {
	next_year:
		/* compute next years easter sunday right away */
		res.e.when = __easter(i.y + 1);
		res.e.nwhat = 1U;
		res.st.s = &on;
	} else {
		echs_instant_t easter = __easter(i.y);

		if (i.m < easter.m || i.d < easter.d) {
			res.e.when = easter;
			res.e.nwhat = 1U;
			res.st.s = &on;
		} else if (i.m > easter.m || i.d > easter.d) {
			goto next_year;
		} else {
			/* compute end of easter */
			if (++easter.d > 31U) {
				easter.d = 1U;
				easter.m = 4U;
			}
			res.e.when = easter;
			res.e.nwhat = 1U;
			res.st.s = &off;
		}
	}
	return &res.e;
}


int
main(void)
{
	echs_instant_t start = {2000, 1, 1};

	for (size_t i = 0; i < 4; i++) {
		const struct echs_event_s *nx = easter_next(start);

		printf("nx %04u-%02u-%02u: %c%s\n",
		       nx->when.y, nx->when.m, nx->when.d,
		       nx->what[0].s->w ? ' ' : '~', nx->what[0].s->d);
		start = nx->when;
	}
	return 0;
}

/* echse.c ends here */
