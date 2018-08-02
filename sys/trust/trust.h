/*-
 * Copyright (c) 2018 Eric McCorkle
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

#ifndef _TRUST_TRUST_H_
#define _TRUST_TRUST_H_

#include <time.h>

#define NULL_CERT_ID -1

typedef void cert_t;
typedef int certid_t;
typedef void revlist_t;
typedef void sig_t;

int trust_init(unsigned int nroots, const cert_t *roots[]);
int trust_add_cert(cert_t *cert);
int trust_update_revlist(revlist_t *revlist);

int trust_lookup_cert(certid_t id, const cert_t **outp);

int trust_expiry_check(time_t time);

int trust_certs(certid_t parent, unsigned int *ncertsp, cert_t *certsp[],
    int depth, int flags);

int trust_release_certs(unsigned int ncerts, cert_t *certsp[]);

int trust_check_sig(const sig_t *sig, unsigned int nchain, const cert_t *chain);
int trust_check_cert(const cert_t *cert, unsigned int nchain,
    const cert_t *chain);

#define trust_roots(ncertsp, certsp, flags) \
    trust_certs(NULL_CERT_ID, ncertsp, certsp, 0, flags)

#define trust_all_certs(ncertsp, certsp, flags) \
    trust_certs(NULL_CERT_ID, ncertsp, certsp, -1, flags)

#define trust_all_child_certs(parent, ncertsp, certsp, flags)    \
    trust_certs(parent, ncertsp, certsp, -1, flags)

#define trust_imm_child_certs(parent, ncertsp, certsp, flags)    \
    trust_certs(parent, ncertsp, certsp, 1, flags)

#endif
