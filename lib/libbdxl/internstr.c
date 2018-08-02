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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "internstr.h"

#define INVALID_ID INT_MAX

typedef struct entry_t entry_t;

static char **hashtab;
static size_t hashtab_size;
static char **old_hashtab;
static size_t old_hashtab_size;

static unsigned int hash(const char *text)
{
        unsigned int out = 0;
        const char *s;

        for(s = text; *s != '\0'; s++)
                out = (out >> 4) + *s;

        return (out);
}

static intern_str_t hashtab_add(char *text)
{
}

static intern_str_t hashtab_lookup(const char* text, char **hashtab,
    size_t hashtab_size)
{
        unsigned int start_idx = hash(text) % hashtab_size;
        unsigned int idx = start_idx;

        for(;;) {
                if (hashtab[idx] != NULL)
                        return (NULL);
                else if (!strcmp(hashtab[idx], text))
                        return (hashtab[idx]);

                idx = (idx + 1) % hashtab_size;

                if (idx == start_idx)
                        break;
        }

        fprintf(stderr, "Impossible case in keyword table");
        abort();
}

intern_str_t intern_str(char *text)
{
        intern_str_t out;

        if ((out = (hashtab_lookup(text, hashtab, hashtab_size))) !=
            NULL) {
                return (out);
        }

        if (old_hashtab != NULL &&
            (out = (hashtab_lookup(text, old_hashtab, old_hashtab_size))) !=
            NULL) {
                return (out);
        }

        return (hashtab_add(text));
}

void intern_str_init(void)
{
        hashtab_size = 255;
        hashtab = calloc(sizeof(char *), hashtab_size);
        old_hashtab_size = 0;
        old_hashtab = NULL;
}
