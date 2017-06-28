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

#ifndef _LIBMINIPKI_PEM_
#define _LIBMINIPKI_PEM_

#include <sys/types.h>

#include "base64.h"

enum pem_state_code
{
  /* Writing out "----- BEGIN CERTIFICATE -----" */
  PEM_STATE_BEGIN,
  /* Writing out a line of base64-encoded data */
  PEM_STATE_PAYLOAD,
  /* Writing out "----- END CERTIFICATE -----" */
  PEM_STATE_END,
};

struct pem_state
{
        struct base64_state pem_b64;
        enum pem_state_code pem_state;
        u_int pem_remaining;
};

extern void pem_start(struct pem_state *state);

extern bool pem_is_finished(const struct pem_state *state);

extern void pem_enc(struct pem_state *state, const void **srcp,
    size_t *srclen, void **dstp, size_t *dstlen);

extern void pem_finish(struct pem_state *state, void **dstp, size_t *dstlen);

#endif
