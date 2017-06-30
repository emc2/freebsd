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

#ifndef _LIBMINIPKI_COMMON_H_
#define _LIBMINIPKI_COMMON_H_

#include <sys/types.h>

enum digest_kind
{
        DIGEST_KIND_SHA256,
        DIGEST_KIND_SHA384,
        DIGEST_KIND_SHA512
};

enum sig_kind
{
        SIG_KIND_RSA,
        SIG_KIND_ED25519
};

/* Distinguished names.  Since we are relying on the DER encoding and
 * we don't care about the contents of distinguished names aside
 * from equality checks, we can just treat them as bitstrings
 */
struct dname
{
        u_int dn_len;
        void *dn_data;
};

struct serial_num
{
        u_int sn_len;
        void *sn_data;
};

struct signer
{
        enum digest_kind si_digest;
        enum digest_kind si_alg;
        struct dname si_dname;
        struct serial_num si_serial;
        u_int si_siglen;
        void *si_sig;
};

#endif
