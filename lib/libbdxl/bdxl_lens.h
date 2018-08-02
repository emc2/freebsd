/*-
 * Copyright (c) 2018 Eric McCorkle,
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _BDXL_LENS_H_
#define _BDXL_LENS_H_

#include <stddef.h>

typedef struct bdxl_lens_t bdxl_lens_t;

struct bdxl_lens_t {
        int (*get)(bdxl_lens_t *this, const void *con, void **absp);
        int (*put)(bdxl_lens_t *this, const void *abs, void *oldcon,
            void** newconp);
        bdxl_lens_t *(*copy)(bdxl_lens_t *this);
        void (*free)(bdxl_lens_t *this);
        char extra[];
};

extern bdxl_lens_t * bdxl_lens_compose(bdxl_lens_t *lens, ...);
extern bdxl_lens_t * bdxl_lens_composev(size_t nlenses, bdxl_lens_t *lenses[]);

#endif
