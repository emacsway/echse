/*** evfilt.c -- filtering streams by events from other streams
 *
 * Copyright (C) 2014-2015 Sebastian Freundt
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
#include <stdbool.h>
#include "evfilt.h"
#include "range.h"
#include "nifty.h"

/* generic stream with exceptions */
struct evfilt_s {
	echs_evstrm_class_t class;

	/* normal events */
	echs_evstrm_t e;
	/* exception events */
	echs_evstrm_t x;
	/* the next event from X */
	echs_event_t ex;
};

static echs_event_t next_evfilt(echs_evstrm_t);
static void free_evfilt(echs_evstrm_t);
static echs_evstrm_t clone_evfilt(echs_const_evstrm_t);
static void send_evfilt(int whither, echs_const_evstrm_t s);
static echs_range_t set_valid_evfilt(echs_evstrm_t s, echs_range_t v);
static echs_range_t valid_evfilt(echs_const_evstrm_t s);

static const struct echs_evstrm_class_s evfilt_cls = {
	.next = next_evfilt,
	.free = free_evfilt,
	.clone = clone_evfilt,
	.seria = send_evfilt,
	.set_valid = set_valid_evfilt,
	.valid = valid_evfilt,
};

static echs_event_t
next_evfilt(echs_evstrm_t s)
{
	struct evfilt_s *this = (struct evfilt_s*)s;
	echs_event_t e = echs_evstrm_next(this->e);
	bool ex_in_past_p = false;

check:
	if (UNLIKELY(echs_nul_event_p(this->ex))) {
		/* no more exceptions */
		return e;
	}

	/* otherwise check if the current exception overlaps with E */
	if ((echs_instant_le_p(e.from, this->ex.from) &&
	     !echs_instant_le_p(e.till, this->ex.from)) ||
	    ((ex_in_past_p = echs_instant_le_p(this->ex.from, e.from)) &&
	     !echs_instant_le_p(this->ex.till, e.from))) {
		/* yes it does */
		e = echs_evstrm_next(this->e);
		goto check;
	} else if (ex_in_past_p) {
		/* we can't say for sure because there could be
		 * another exception in the range of E */
		this->ex = echs_evstrm_next(this->x);
		ex_in_past_p = false;
		goto check;
	}
	/* otherwise it's certainly safe */
	return e;
}

static void
free_evfilt(echs_evstrm_t s)
{
	struct evfilt_s *this = (struct evfilt_s*)s;

	free_echs_evstrm(this->e);
	free_echs_evstrm(this->x);
	return;
}

static echs_evstrm_t
clone_evfilt(echs_const_evstrm_t s)
{
	const struct evfilt_s *that = (const struct evfilt_s*)s;
	struct evfilt_s *this;

	if (UNLIKELY((this = malloc(sizeof(*this))) == NULL)) {
		return NULL;
	}
	this->class = &evfilt_cls;
	this->e = clone_echs_evstrm(that->e);
	this->x = clone_echs_evstrm(that->x);
	this->ex = that->ex;
	return (echs_evstrm_t)this;
}

static void
send_evfilt(int whither, echs_const_evstrm_t s)
{
	const struct evfilt_s *this = (const struct evfilt_s*)s;

	echs_evstrm_seria(whither, this->e);
	return;
}

static echs_range_t
set_valid_evfilt(echs_evstrm_t s, echs_range_t v)
{
	struct evfilt_s *restrict this = (struct evfilt_s*)s;

	return echs_evstrm_set_valid(this->e, v);
}

static echs_range_t
valid_evfilt(echs_const_evstrm_t s)
{
	const struct evfilt_s *this = (const struct evfilt_s*)s;

	return echs_evstrm_valid(this->e);
}


echs_evstrm_t
make_evfilt(echs_evstrm_t e, echs_evstrm_t x)
{
	struct evfilt_s *res;

	if (UNLIKELY(e == NULL)) {
		/* filtering out no events will result in no events */
		return NULL;
	} else if (UNLIKELY(x == NULL)) {
		/* a filter that doesn't filter stuff adds nothing */
		return e;
	} else if (UNLIKELY((res = malloc(sizeof(*res))) == NULL)) {
		return NULL;
	}
	res->class = &evfilt_cls;
	res->e = e;
	res->x = x;
	res->ex = echs_evstrm_next(x);
	return (echs_evstrm_t)res;
}

/* evfilt.c ends here */
