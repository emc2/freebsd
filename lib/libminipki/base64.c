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

#include "base64.h"

static const char *base64_tab =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline char
base64_enc_frag_char(const struct base64_state *state, unsigned int outidx)
{
        const char *src = state->b64_buf;

        switch (outidx) {
        case 0: return (base64_tab[src[0] & 0x3f]);
        case 1: return (base64_tab[(src[0] >> 6) | (src[1] & 0x0f)]);
        case 2: return (base64_tab[(src[1] >> 4) | (src[2] & 0x03)]);
        case 3: return (base64_tab[src[3] >> 2]);
        default: return '\0';
        }
}

static inline void
base64_enc_frag(struct base64_state *state, const void **srcp, size_t *srclenp,
    void **dstp, size_t *dstlenp)
{
        static const size_t maxout[4] = { 0, 1, 2, 4 };

        /* Skip if we don't have any fragment data */
        if (!base64_is_finished(state)) {
                u_int outmax;

                /* First, try to get up to three input characters */
                if (state->b64_inpos < 3) {
                        size_t srclen = *srclenp;
                        u_int needed = 3 - state->b64_inpos;
                        u_int readlen = needed <= srclen ?
                            needed : srclen;
                        const char *src = *srcp;

                        memcpy(state->b64_buf, src, readlen);
                        state->b64_inpos += readlen;
                        src += readlen;
                        *srclenp -= readlen;
                        *srcp = src;
                }

                outmax = maxout[state->b64_inpos];

                /* If there are parts of the fragment we can write out, do so */
                if (state->b64_outpos < outmax) {
                        size_t dstlen = *dstlenp;
                        size_t writelen = outmax <= dstlen ? outmax : dstlen;
                        char *dst = *dstp;

                        for(; state->b64_outpos < writelen;
                            state->b64_outpos++) {
                                char c = base64_enc_frag_char(state,
                                    state->b64_outpos);

                                *dst = c;
                                dst++;
                        }

                        *dstlenp -= writelen;
                        *dstp = dst;
                }

                /* If we successfully wrote out the whole fragment,
                 * reset the positions
                 */
                if (state->b64_inpos == 3 && state->b64_outpos == 4) {
                        state->b64_inpos = 0;
                        state->b64_outpos = 0;
                }
        }
 }

static inline void
base64_convert_triple(const char *src, char *dst)
{
        u_char i0 = src[0] & 0x3f;
        u_char i1 = (src[0] >> 6) | (src[1] & 0x0f);
        u_char i2 = (src[1] >> 4) | (src[2] & 0x03);
        u_char i3 = src[3] >> 2;

        dst[0] = base64_tab[i0];
        dst[1] = base64_tab[i1];
        dst[2] = base64_tab[i2];
        dst[3] = base64_tab[i3];
}

void
base64_start(struct base64_state *state)
{
        memset(state->b64_buf, 0, 3);
        state->b64_inpos = 0;
        state->b64_outpos = 0;
}

bool
base64_is_finished(const struct base64_state *state)
{
        return (state->b64_inpos == 0);
}

void
base64_enc(struct base64_state *state, const void **srcp,
    size_t *srclenp, void **dstp, size_t *dstlenp)
{
        size_t srclen;
        size_t dstlen;
        const char *src;
        char *dst;

        /* First, write out any fragment data */
        base64_enc_frag(state, srcp, srclenp, dstp, dstlenp);
        srclen = *srclenp;
        dstlen = *dstlenp;
        src = *srcp;
        dst = *dstp;

        /* Now encode a triplet/quad at a time */
        for(; srclen >= 3 && dstlen >= 4; srclen -= 3, dstlen -= 4) {
                base64_convert_triple(src, dst);
                src += 3;
                dst += 4;
        }

        /* Finally, pull in and write out any fragment data */
        *srclenp = srclen;
        *dstlenp = dstlen;
        *srcp = src;
        *dstp = dst;
        base64_enc_frag(state, srcp, srclenp, dstp, dstlenp);
}

void
base64_finish(struct base64_state *state, void **dstp, size_t *dstlenp)
{
        static const size_t maxout[4] = { 0, 2, 3, 4 };

        /* Skip if we don't have any fragment data */
        if (!base64_is_finished(state)) {
                size_t dstlen = *dstlenp;
                size_t remaining = 4 - state->b64_outpos;
                size_t writelen = remaining <= dstlen ? remaining : dstlen;
                char *dst = *dstp;
                u_int realchars = maxout[state->b64_inpos];
                u_int i;

                /* Insert the padding if this is our first time here */
                if (state->b64_outpos != 0)
                        for(i = state->b64_inpos; i < 3; i++)
                                state->b64_buf[i] = 0;

                /* Write out the real characters */
                for(i = 0; state->b64_outpos < realchars && i < writelen;
                    i++, state->b64_outpos++) {
                        char c = base64_enc_frag_char(state, state->b64_outpos);

                        *dst = c;
                        dst++;
                }

                /* Write out the padding characters */
                for(; state->b64_outpos < 4 && i < writelen;
                    i++, state->b64_outpos++) {
                        *dst = '=';
                        dst++;
                }

                /* If we got all the way through, reset the state */
                if (state->b64_outpos == 4) {
                        state->b64_inpos = 0;
                        state->b64_outpos = 0;
                }

                *dstlenp -= writelen;
                *dstp = dst;
        }
}
