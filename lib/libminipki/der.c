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

#include <errno.h>

#include "der.h"

#define DER_CONTENT_END 0x00
#define DER_CONTENT_BOOL 0x01
#define DER_CONTENT_INT 0x02
#define DER_CONTENT_BITSTR 0x03
#define DER_CONTENT_OID 0x06
#define DER_CONTENT_SEQ 0x30

/* Read the content code and length */
static inline int
der_read_content_and_len(const void **datap, size_t *datalenp, u_int *contentp,
    size_t *lenp)
{
        const void *data = *datap;
        size_t datalen = *datalenp;
        const u_char *cptr = data;
        u_char cval;
        u_int content;
        size_t len;
        u_int i;

        if (datalen < 2)
                return (ERANGE);

        cval = *cptr;
        content = cval;
        cptr++;
        datalen--;
        cval = *cptr;
        cptr++;
        datalen--;

        if (cval & 0x80) {
                /* Short form */
                len = cval & 0x7f;
        } else {
                u_int lenbytes = cval & 0x7f;

                /* Lengths over 2^32 aren't supported by this implementation */
                if (lenbytes <= 4)
                         return (EINVAL);

                if (datalen < lenbytes)
                         return (ERANGE);

                len = 0;

                for (i = 0; i < lenbytes; i++) {
                         cval = *cptr;
                         cptr++;
                         datalen--;
                         len <<= 8;
                         len |= cval;
                }
        }

        *datap = cptr;
        *datalenp = datalen;
        *contentp = content;
        *lenp = len;

        return (0);
}

static inline int
der_expect_content_read_len(const void **datap, size_t *datalenp,
    u_int expected, size_t *lenp)
{
         const void* data = *datap;
         size_t datalen = *datalenp;
         u_int actual;
         int err;

         if ((err = der_read_content_and_len(&data, &datalen, &actual,
             lenp)) != 0)
                  return (err);

         if (expected != actual)
                 return (EINVAL);

         *datap = data;
         *datalenp = datalen;

         return (0);
}

static inline int
der_expect_content_and_len(const void **datap, size_t *datalenp,
    u_int content, size_t expected)
{
         const void* data = *datap;
         size_t datalen = *datalenp;
         size_t actual;
         int err;

         if ((err = der_expect_content_read_len(&data, &datalen, content,
             &actual)) != 0)
                  return (err);

         if (expected != actual)
                 return (EINVAL);

         *datap = data;
         *datalenp = datalen;

         return (0);
}

int
der_read_seq_len(const void **datap, size_t *datalenp, size_t *outp) {
         return (der_expect_content_read_len(datap, datalenp,
             DER_CONTENT_SEQ, outp));
}

int
der_peek_seq_len(const void *data, size_t datalen, size_t *outp)
{
        return (der_read_seq_len(&data, &datalen, outp));
}

int
der_read_small_uint(const void **datap, size_t *datalenp, u_int64_t *outp)
{
         int err;
         const void* data = *datap;
         size_t datalen = *datalenp;
         size_t intlen;
         const char *cptr;
         u_int64_t out = 0;
         u_int i;

         /* Expect to see an int type */
         if ((err = der_expect_content_read_len(&data, &datalen,
             DER_CONTENT_INT, &intlen)) != 0)
                  return (err);

         /* Check it fits into 8 bytes and we have enough data */
         if (intlen > 8 || datalen < intlen)
                  return (ERANGE);

         cptr = data;

         for (i = 0; i < intlen; i++) {
                 out <<= 8;
                 out |= *cptr;
                 cptr++;
                 datalen--;
         }

         *datap = cptr;
         *datalenp = datalen;
         *outp = out;

         return (0);
}

int
der_read_small_int(const void **datap, size_t *datalenp, int64_t *outp)
{
         int err;
         const void* data = *datap;
         size_t datalen = *datalenp;
         size_t intlen;
         const char *cptr;
         int64_t out = 0;
         u_int i;

         /* Expect to see an int type */
         if ((err = der_expect_content_read_len(&data, &datalen,
             DER_CONTENT_INT, &intlen)) != 0)
                  return (err);

         /* Check it fits into 8 bytes and we have enough data */
         if (intlen > 8 || datalen < intlen)
                  return (ERANGE);

         cptr = data;

         for (i = 0; i < intlen; i++) {
                 out <<= 8;
                 out |= *cptr;
                 cptr++;
                 datalen--;
         }

         *datap = cptr;
         *datalenp = datalen;
         *outp = out;

         return (0);
}

int
der_expect_small_uint(const void **datap, size_t *datalenp,
    u_int64_t expected)
{
         int err;
         const void* data = *datap;
         size_t datalen = *datalenp;
         u_int64_t actual;

         if ((err = der_read_small_uint(&data, &datalen, &actual)) != 0)
                  return (err);

         if (expected != actual)
                  return (EINVAL);

         *datap = data;
         *datalenp = datalen;

         return (0);
}

int
der_expect_small_int(const void **datap, size_t *datalenp, int64_t expected)
{
         int err;
         const void* data = *datap;
         size_t datalen = *datalenp;
         int64_t actual;

         if ((err = der_read_small_int(&data, &datalen, &actual)) != 0)
                  return (err);

         if (expected != actual)
                  return (EINVAL);

         *datap = data;
         *datalenp = datalen;

         return (0);
}

static inline int
der_read_raw(const void **datap, size_t *datalenp, u_int content,
    size_t *rawlenp, const void **rawp)
{
         int err;
         const void* data = *datap;
         size_t datalen = *datalenp;
         size_t rawlen;

         /* Expect to see an int type */
         if ((err = der_expect_content_read_len(&data, &datalen, content,
             &rawlen)) != 0)
                  return (err);

         /* Check it fits into 8 bytes and we have enough data */
         if (datalen < rawlen)
                  return (ERANGE);

         *rawp = data;
         *rawlenp = rawlen;
         *datalenp = datalen;
         *datap = ((const char *)data) + datalen;

         return (0);
}


int
der_read_unbounded_int(const void **datap, size_t *datalenp, size_t *intlenp,
    const void **intp)
{
         return (der_read_raw(datap, datalenp, DER_CONTENT_INT, intlenp, intp));
}

int
der_read_bounded_int(const void **datap, size_t *datalenp, size_t expected,
    const void **intp)
{
         int err;
         const void* data = *datap;
         size_t datalen = *datalenp;
         u_int64_t actual;

         if ((err = der_read_unbounded_int(&data, &datalen, &actual,
             intp)) != 0)
                  return (err);

         if (expected != actual)
                  return (EINVAL);

         *datap = data;
         *datalenp = datalen;

         return (0);
}

int
der_read_oid(const void **datap, size_t *datalenp, size_t *oidlenp,
    const void **oidp)
{
         return (der_read_raw(datap, datalenp, DER_CONTENT_OID, oidlenp, oidp));
}

int
der_read_bitstr(const void **datap, size_t *datalenp, size_t *bitstrlenp,
    const void **bitstrp)
{
         return (der_read_raw(datap, datalenp, DER_CONTENT_BITSTR, bitstrlenp,
             bitstrp));
}

int
der_read_opaque_seq(const void **datap, size_t *datalenp, size_t *seqlenp,
    const void **seqp)
{
         return (der_read_raw(datap, datalenp, DER_CONTENT_SEQ, seqlenp, seqp));
}

int
der_expect_end_of_content(const void **datap, size_t *datalenp)
{
         return (der_expect_content_and_len(datap, datalenp,
             DER_CONTENT_END, 0));
}
