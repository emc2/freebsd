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
#include "bdxl_ast.h"
#include "bdxl_ast_private.h"
#include "internstr.h"

bdxl_type_node_t * bdxl_unit_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol)
{
        bdxl_type_node_t *out = malloc(sizeof(bdxl_type_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_UNIT;
        }

        return (out);
}

bdxl_type_node_t * bdxl_bool_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol)
{
        bdxl_type_node_t *out = malloc(sizeof(bdxl_type_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_BOOL;
        }

        return (out);
}

bdxl_type_node_t * bdxl_sint_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol, int64_t lo, int64_t hi)
{
        bdxl_type_node_t *out = malloc(sizeof(bdxl_type_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_SINT;
                out->data.sint_data.lo = lo;
                out->data.sint_data.hi = hi;
        }

        return (out);
}

bdxl_type_node_t * bdxl_uint_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol, int64_t lo, int64_t hi)
{
        bdxl_type_node_t *out = malloc(sizeof(bdxl_type_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_UINT;
                out->data.uint_data.lo = lo;
                out->data.uint_data.hi = hi;
        }

        return (out);
}

bdxl_type_node_t * bdxl_real_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol, double lo, double hi)
{
        bdxl_type_node_t *out = malloc(sizeof(bdxl_type_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_UINT;
                out->data.real_data.lo = lo;
                out->data.real_data.hi = hi;
        }

        return (out);
}

bdxl_type_node_t * bdxl_string_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol)
{
        bdxl_type_node_t *out = malloc(sizeof(bdxl_type_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_STRING;
        }

        return (out);
}

bdxl_type_node_t * bdxl_seq_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol, bool ordered,
    bdxl_seq_field_list_t *list)
{
        bdxl_type_node_t *out;
        bdxl_seq_field_list_t *curr;
        size_t size;
        int i;

        for(size = 0, curr = list; curr != NULL; size++, curr = curr->next);

        out = malloc(offsetof(bdxl_type_node_t, data) +
            offsetof(bdxl_type_node_data_t, seq_data) +
            offsetof(bdxl_seq_node_data_t, fields) +
            (size * sizeof(bdxl_seq_field_node_t)));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_SEQ;
                out->data.seq_data.ordered = ordered;
                out->data.seq_data.nfields = size;

                for(i = 0, curr = list; curr != NULL; i++, curr = curr->next)
                        memcpy(out->data.seq_data.fields + (size - 1 - i),
                            &(curr->data), sizeof(bdxl_seq_field_node_t));
        }

        return (out);
}

bdxl_type_node_t * bdxl_choice_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol, bdxl_choice_opt_list_t *list)
{
        bdxl_type_node_t *out;
        bdxl_choice_opt_list_t *curr;
        size_t size;
        int i;

        for(size = 0, curr = list; curr != NULL; size++, curr = curr->next);

        out = malloc(offsetof(bdxl_type_node_t, data) +
            offsetof(bdxl_type_node_data_t, seq_data) +
            offsetof(bdxl_seq_node_data_t, fields) +
            (size * sizeof(bdxl_choice_opt_node_t)));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_CHOICE;
                out->data.seq_data.nfields = size;

                for(i = 0, curr = list; curr != NULL; i++, curr = curr->next)
                        memcpy(out->data.seq_data.fields + (size - 1 - i),
                            &(curr->data), sizeof(bdxl_choice_opt_node_t));
        }

        return (out);
}

bdxl_type_node_t * bdxl_seq_of_type_node_t(const char *fname, unsigned int line,
    unsigned int startcol, unsigned int endcol, size_t minsize, size_t maxsize,
    bool unique, bool ordered, bdxl_type_node_t *elemtype)
{
        bdxl_type_node_t *out = malloc(sizeof(bdxl_type_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->tag = BDXL_TYPE_SEQ_OF;
                out->data.seq_of_data.minsize = minsize;
                out->data.seq_of_data.maxsize = maxsize;
                out->data.seq_of_data.unique = unique;
                out->data.seq_of_data.ordered = ordered;
                out->data.seq_of_data.elemtype = elemtype;
        }

        return (out);
}

bdxl_seq_field_node_t * bdxl_seq_field_node(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    intern_str_t name, bdxl_type_node_t *type)
{
        bdxl_seq_field_node_t *out = malloc(sizeof(bdxl_seq_field_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->name = name;
                out->type = type;
        }

        return (out);
}

bdxl_choice_opt_node_t * bdxl_choice_opt_node(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    intern_str_t name, bdxl_type_node_t *type)
{
        bdxl_choice_opt_node_t *out = malloc(sizeof(bdxl_choice_opt_node_t));

        if (out != NULL) {
                out->pos.fname = fname;
                out->pos.line = line;
                out->pos.startcol = startcol;
                out->pos.endcol = endcol;
                out->name = name;
                out->type = type;
        }

        return (out);
}

bdxl_seq_field_list_t * bdxl_seq_field_list(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    intern_str_t name, bdxl_type_node_t *type, bdxl_seq_field_list_t *next)
{
        bdxl_seq_field_list_t *out = malloc(sizeof(bdxl_seq_field_list_t));

        if (out != NULL) {
                out->data.pos.fname = fname;
                out->data.pos.line = line;
                out->data.pos.startcol = startcol;
                out->data.pos.endcol = endcol;
                out->data.name = name;
                out->data.type = type;
                out->next = next;
        }

        return (out);
}

bdxl_choice_opt_list_t * bdxl_choice_opt_list(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    intern_str_t name, bdxl_type_node_t *type, bdxl_choice_opt_list_t *next)
{
        bdxl_choice_opt_list_t *out = malloc(sizeof(bdxl_choice_opt_list_t));

        if (out != NULL) {
                out->data.pos.fname = fname;
                out->data.pos.line = line;
                out->data.pos.startcol = startcol;
                out->data.pos.endcol = endcol;
                out->data.name = name;
                out->data.type = type;
                out->next = next;
        }

        return (out);
}

void bdxl_seq_field_node_free(bdxl_seq_field_node_t *node)
{
        bdxl_type_node_free(node->type);
        free(node);
}

void bdxl_choice_opt_node_free(bdxl_choice_opt_node_t *node)
{
        bdxl_type_node_free(node->type);
        free(node);
}

void bdxl_type_node_free(bdxl_type_node_t *node)
{
        unsigned int i;

        switch(node->tag) {
        default:
                fprintf(stderr, "Invalid type code %u\n", node->tag);
                abort();

        case BDXL_TYPE_UNIT:
        case BDXL_TYPE_BOOL:
        case BDXL_TYPE_UINT:
        case BDXL_TYPE_SINT:
        case BDXL_TYPE_REAL:
        case BDXL_TYPE_STRING:
                break;

        case BDXL_TYPE_SEQ:
                for(i = 0; i < node->data.seq_data.nfields; i++)
                        bdxl_seq_field_node_free(
                            node->data.seq_data.fields + i);
                break;

        case BDXL_TYPE_CHOICE:
                for(i = 0; i < node->data.choice_data.noptions; i++)
                        bdxl_choice_opt_node_free(
                            node->data.choice_data.options + i);
                break;

        case BDXL_TYPE_SEQ_OF:
                bdxl_type_node_free(node->data.seq_of_data.elemtype);
                break;
        }

        free(node);
}

void bdxl_seq_field_list_free(bdxl_seq_field_list_t *list)
{
        if (list->next != NULL)
                bdxl_seq_field_list_free(list->next);

        free(list);
}

void bdxl_choice_opt_list_free(bdxl_choice_opt_list_t *list)
{
        if (list->next != NULL)
                bdxl_choice_opt_list_free(list->next);

        free(list);
}
