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

#ifndef _LIBMINIPKI_BASE64_
#define _LIBMINIPKI_BASE64_

#include <stdbool.h>
#include <sys/types.h>

struct base64_state
{
        char b64_buf[3];
        u_char b64_inpos;
        u_char b64_outpos;
};

/* Initialize the base64 encoding state. */
extern void base64_start(struct base64_state *state);

extern bool base64_is_finished(const struct base64_state *state);

/* Encode as much of the source data as possible and write it out to
 * dst.  The state argument is necessary to hold fragment data across
 * multiple calls to base64_enc and/or base64_finish
 */
extern void base64_enc(struct base64_state *state, const void **srcp,
    size_t *srclen, void **dstp, size_t *dstlen);

/* Finish writing out base64 data to the destination, including any
 * padding.  This should be called once all source data has been
 * written out with base64_enc.
 */
extern void base64_finish(struct base64_state *state, void **dstp,
    size_t *dstlen);

#endif
