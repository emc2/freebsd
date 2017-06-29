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

#ifndef _LIBMINIPKI_CMS_H_
#define _LIBMINIPKI_CMS_H_

#include <sys/types.h>

#include "common.h"

enum cms_msg_kind
{
        CMS_MSG_SIGNED_DATA
};

struct cms_signer
{
        enum digest_kind si_digest;
        enum digest_kind si_alg;
        u_int si_nnameelems;
        struct dname_elem *si_nameelems;
        u_int64_t _cmsig_serial;
        void *si_sig;
};

#define cmsig_data cms_payload._cms_signed._cmsig_data
#define cmsig_nsiginfos cms_payload._cms_signed._cmsig_nsiginfos
#define cmsig_siginfos cms_payload._cms_signed._cmsig_siginfos

struct cms_signed
{
        void *_cmsig_data;
        u_int _cmsig_nsiginfos;
        struct cms_signer *_cmsig_siginfos;
};

union cms_payload
{
        struct cms_signed _cms_signed;
};

struct cms_msg
{
        enum cms_msg_kind cms_kind;
        union cms_payload _cms_payload;
};

#endif
