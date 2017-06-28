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

#include <string.h>

#include "pem.h"

#define BEGIN_STR "----- BEGIN CERTIFICATE -----\n"
#define END_STR "----- END CERTIFICATE -----\n"
#define BEGIN_LENGTH ((sizeof BEGIN_STR) - 1)
#define END_LENGTH ((sizeof END_STR) - 1)

static void pem_enter_payload(struct pem_state *state)
{
        state->pem_state = PEM_STATE_PAYLOAD;
        state->pem_remaining = 64;
}

static void pem_enter_end(struct pem_state *state)
{
        state->pem_state = PEM_STATE_END;
        state->pem_remaining = END_LENGTH;
}

static void
pem_complete_begin(struct pem_state *state, void **dstp, size_t *dstlenp)
{
        static const char *begin_str = BEGIN_STR;
        size_t dstlen = *dstlenp;
        size_t writelen = state->pem_remaining <= dstlen ?
            state->pem_remaining : dstlen;
        size_t off = BEGIN_LENGTH - state->pem_remaining;
        char *dst = *dstp;

        memcpy(dst, begin_str + off, writelen);
        dst += writelen;
        state->pem_remaining -= writelen;
        *dstlenp -= writelen;
        *dstp = dst;

        /* If we exhausted the beginning line, enter the payload state */
        if (state->pem_remaining == 0)
                pem_enter_payload(state);
}

static void
pem_complete_payload(struct pem_state *state, void **dstp, size_t *dstlenp)
{
        /* Finish the current payload */
        if (!base64_is_finished(&state->pem_b64)) {
                base64_finish(&state->pem_b64, dstp, dstlenp);

                /* If we're still not finished after calling finish,
                 * we must have run out of output space, so we stop.
                 */
                if (!base64_is_finished(&state->pem_b64))
                        return;
        }

        /* If we have space for a newline, write it and then transition */
        if (*dstlenp != 0) {
                char *dst = *dstp;

                *dst = '\n';
                dst++;
                *dstp = dst;
                *dstlenp -= 1;
                pem_enter_end(state);
        }
}

static void
pem_complete_end(struct pem_state *state, void **dstp, size_t *dstlenp)
{
        static const char *end_str = BEGIN_STR;
        size_t dstlen = *dstlenp;
        size_t writelen = state->pem_remaining <= dstlen ?
            state->pem_remaining : dstlen;
        size_t off = END_LENGTH - state->pem_remaining;
        char *dst = *dstp;

        memcpy(dst, end_str + off, writelen);
        state->pem_remaining -= writelen;
        dst += writelen;
        *dstlenp -= writelen;
        *dstp = dst;
}

static void
pem_write_line(struct pem_state *state, const void **srcp, size_t *srclenp,
    void **dstp, size_t *dstlenp)
{
        size_t dstlen = *dstlenp;
        size_t init = state->pem_remaining <= dstlen ?
            state->pem_remaining : dstlen;
        size_t remaining = init;
        size_t written;

        base64_enc(&state->pem_b64, srcp, srclenp, dstp, &remaining);
        written = init - remaining;
        state->pem_remaining -= written;
        *dstlenp -= written;
}

static void
pem_write_lines(struct pem_state *state, const void **srcp, size_t *srclenp,
    void **dstp, size_t *dstlenp)
{
        size_t dstlen = *dstlenp;
        size_t srclen = *srclenp;

        while (dstlen != 0 && srclen != 0) {
                if (state->pem_remaining != 0)
                        pem_write_line(state, srcp, &srclen, dstp, &dstlen);

                if (state->pem_remaining == 0 && dstlen > 0) {
                        const void* buf = "\n";
                        size_t len = 1;

                        base64_enc(&state->pem_b64, &buf, &len, dstp, &dstlen);
                }
        }

        *dstlenp = dstlen;
        *srclenp = srclen;
}

void
pem_start(struct pem_state *state)
{
        base64_start(&state->pem_b64);
        state->pem_state = PEM_STATE_BEGIN;
        state->pem_remaining = BEGIN_LENGTH;
}

bool
pem_is_finished(const struct pem_state *state)
{
        return (state->pem_state == PEM_STATE_END && state->pem_remaining == 0);
}

void
pem_enc(struct pem_state *state, const void **srcp, size_t *srclenp,
    void **dstp, size_t *dstlenp)
{
        switch (state->pem_state) {
        case PEM_STATE_BEGIN:
                pem_complete_begin(state, dstp, dstlenp);

                if (state->pem_state == PEM_STATE_BEGIN)
                        return;

                /* FALLTHROUGH */
        case PEM_STATE_PAYLOAD:
                pem_write_lines(state, srcp, srclenp, dstp, dstlenp);
                break;

        default: break;
        }
}


void
pem_finish(struct pem_state *state, void **dstp, size_t *dstlen)
{
        switch (state->pem_state) {
        case PEM_STATE_BEGIN:
                pem_complete_begin(state, dstp, dstlen);

                if (state->pem_state == PEM_STATE_BEGIN)
                        return;

                /* FALLTHROUGH */
        case PEM_STATE_PAYLOAD:
                pem_complete_payload(state, dstp, dstlen);

                if (state->pem_state == PEM_STATE_PAYLOAD)
                        return;

                /* FALLTHROUGH */
        case PEM_STATE_END:
                pem_complete_end(state, dstp, dstlen);
                break;

        default: break;
        }
}
