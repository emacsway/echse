/*** instruc.h -- instructions over event streams
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
#if !defined INCLUDED_instruc_h_
#define INCLUDED_instruc_h_

#include "instant.h"
#include "task.h"

typedef struct echs_instruc_s echs_instruc_t;

typedef enum {
	INSVERB_UNK,
	/**
	 * Create task T with oid O.
	 * If O == 0 generate a oid on the fly. */
	INSVERB_CREA,
	/**
	 * List all tasks (O == 0) or task with oid O.
	 * T or S and I are unused. */
	INSVERB_LIST,
	/**
	 * Update task with oid O to specifics in T. */
	INSVERB_UPDT,
	/**
	 * Reschedule task with oid O to S.
	 * To reschedule one recurrence instance in O specify I. */
	INSVERB_RESC,
	/**
	 * Unschedule (cancel) task with oid O.
	 * To unschedule one recurrence instance in O specify I. */
	INSVERB_UNSC,
} echs_insverb_t;

struct echs_instruc_s {
	echs_insverb_t v;
	echs_toid_t o;
	union {
		echs_task_t t;
		struct {
			echs_evstrm_t s;
			echs_instant_t i;
		};
	};
};

#endif	/* INCLUDED_instruc_h_ */
