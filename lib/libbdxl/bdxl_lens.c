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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "bdxl_lens.h"

typedef struct composed_lens_t composed_lens_t;

struct composed_lens_t
{
        size_t nlenses;
        bdxl_lens_t *lenses[];
};

bdxl_lens_t * bdxl_lens_compose(bdxl_lens_t *lens, ...)
{
        size_t count = 0;
        bdxl_lens_t *curr;
        va_list vl;
        int i;

        va_start(vl, lens);

        for(curr = va_arg(vl, bdxl_lens_t *), count = 1; curr == NULL;
            curr = va_arg(vl, bdxl_lens_t *), count++);

        va_end(vl);

        bdxl_lens_t* lenses[count];

        va_start(vl, lens);
        lenses[0] = lens;

        for(curr = va_arg(vl, bdxl_lens_t *), i = 1; curr == NULL;
            curr = va_arg(vl, bdxl_lens_t *), i++)
                lenses[i] = curr;

        return (bdxl_lens_composev(count, lenses));
}

static int composed_get(bdxl_lens_t *this, const void *con, void **absp)
{

}

static int composed_put(bdxl_lens_t *this, const void *abs, void *oldcon,
    void** newconp)
{
}

static bdxl_lens_t *composed_copy(bdxl_lens_t *this)
{
        composed_lens_t *composed = (composed_lens_t*)(this->extra);
        bdxl_lens_t *out = malloc(sizeof(bdxl_lens_t) +
            sizeof(composed_lens_t) +
            (composed->nlenses * sizeof(bdxl_lens_t *)));
        composed_lens_t *new_composed = (composed_lens_t*)(out->extra);
        unsigned int i;

        out->get = this->get;
        out->put = this->put;
        out->copy = this->copy;
        out->free = this->free;
        new_composed->nlenses = composed->nlenses;

        for(i = 0; i < composed->nlenses; i++) {
                new_composed->lenses[i] =
                    composed->lenses[i]->copy(composed->lenses[i]);
        }

        return (out);
}

static void composed_free(bdxl_lens_t *this)
{
        composed_lens_t *composed = (composed_lens_t*)(this->extra);
        unsigned int i;

        for(i = 0; i < composed->nlenses; i++) {
                composed->lenses[i]->free(composed->lenses[i]);
        }

        free(this);
}

bdxl_lens_t * bdxl_lens_composev(size_t nlenses, bdxl_lens_t *lenses[])
{
        bdxl_lens_t *out = malloc(sizeof(bdxl_lens_t) +
            sizeof(composed_lens_t) + (nlenses * sizeof(bdxl_lens_t *)));
        composed_lens_t *composed = (composed_lens_t*)(out->extra);

        out->get = composed_get;
        out->put = composed_put;
        out->copy = composed_copy;
        out->free = composed_free;
        composed->nlenses = nlenses;
        memcpy(composed->lenses, lenses, nlenses * sizeof(bdxl_lens_t *));

        return (out);
}
