/*-
 * Copyright (c) 2017 Eric McCorkle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _LIBMINIPKI_DER_H_
#define _LIBMINIPKI_DER_H_

#include <sys/types.h>

extern int der_expect_end_of_content(const void **datap, size_t *datalenp);

extern int der_peek_seq_len(const void *datap, size_t datalenp, size_t *outp);

extern int der_read_seq_len(const void **datap, size_t *datalenp, size_t *outp);

extern int der_read_small_uint(const void **datap, size_t *datalenp,
    u_int64_t *intp);

extern int der_expect_small_uint(const void **datap, size_t *datalenp,
    u_int64_t expected);

extern int der_read_small_int(const void **datap, size_t *datalenp,
    int64_t *intp);

extern int der_expect_small_int(const void **datap, size_t *datalenp,
    int64_t expected);

extern int der_read_unbounded_int(const void **datap, size_t *datalenp,
    size_t *intlenp, const void **intp);

extern int der_read_bounded_int(const void **datap, size_t *datalenp,
    size_t maxlen, const void **intp);

extern int der_read_oid(const void **datap, size_t *datalenp,
    size_t *oidlenp, const void **oidp);

extern int der_read_bitstr(const void **datap, size_t *datalenp,
    size_t *bitstrlenp, const void **bitstrp);

extern int der_read_opaque_seq(const void **datap, size_t *datalenp,
    size_t *seqlenp, const void **seqstrp);

#endif
