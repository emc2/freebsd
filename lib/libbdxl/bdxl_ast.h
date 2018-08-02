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
#ifndef _BDXL_AST_H_
#define _BDXL_AST_H_

#include <stdint.h>
#include <stdbool.h>
#include "bdxl_type.h"

typedef struct pos_t pos_t;
typedef struct bdxl_sint_node_data_t bdxl_sint_node_data_t;
typedef struct bdxl_uint_node_data_t bdxl_uint_node_data_t;
typedef struct bdxl_real_node_data_t bdxl_real_node_data_t;
typedef struct bdxl_seq_node_data_t bdxl_seq_node_data_t;
typedef struct bdxl_seq_field_node_t bdxl_seq_field_node_t;
typedef struct bdxl_choice_node_data_t bdxl_choice_node_data_t;
typedef struct bdxl_choice_opt_node_t bdxl_choice_opt_node_t;
typedef struct bdxl_seq_of_node_data_t bdxl_seq_of_node_data_t;
typedef union bdxl_type_node_data_t bdxl_type_node_data_t;
typedef struct bdxl_type_node_t bdxl_type_node_t;

/*!
 * \brief File positions for a tree element.
 */
struct pos_t {
        /*!
         * \brief Name of the file.
         */
        const char *fname;
        /*!
         * \brief Line number.
         */
        unsigned int line;
        /*!
         * \brief Inclusive starting column.
         */
        unsigned int startcol;
        /*!
         * \brief Inclusive ending column.
         */
        unsigned int endcol;
};

/*!
 * \brief Signed integer data.
 */
struct bdxl_sint_node_data_t {
        /*!
         * \brief Lower bound on the signed integer.
         */
        int64_t lo;
        /*!
         * \brief Upper bound on the signed integer.
         */
        int64_t hi;
};

/*!
 * \brief Unsigned integer data.
 */
struct bdxl_uint_node_data_t {
        /*!
         * \brief Lower bound on the unsigned integer.
         */
        uint64_t lo;
        /*!
         * \brief Upper bound on the unsigned integer.
         */
        uint64_t hi;
};

/*!
 * \brief Real-approximation types (double, float, etc.).
 */
struct bdxl_real_node_data_t {
        /*!
         * \brief Lower bound on the real value.
         */
        double lo;
        /*!
         * \brief Upper bound on the real value.
         */
        double hi;
};

/*!
 * \brief Fields for sequential types.
 */
struct bdxl_seq_field_node_t {
        /*!
         * \brief Position in source that this node represents.
         */
        pos_t pos;
        /*!
         * \brief The field's name.
         */
        const char *name;
        /*!
         * \brief The field's type descriptor.
         */
        bdxl_type_node_t *type;
};

/*!
 * \brief Sequential types.
 *
 * These include structures, sequences, and sets.
 */
struct bdxl_seq_node_data_t {
        /*!
         * \brief Whether the fields are ordered.
         */
        bool ordered;
        /*!
         * \brief Number of field descriptors.
         */
        unsigned int nfields;
        /*!
         * \brief Field descriptors.
         */
        bdxl_seq_field_node_t fields[];
};

/*!
 * \brief Fields for choice types.
 */
struct bdxl_choice_opt_node_t {
        /*!
         * \brief Position in source that this node represents.
         */
        pos_t pos;
        /*!
         * \brief The option's name.
         */
        const char *name;
        /*!
         * \brief The option's type descriptor.
         */
        bdxl_type_node_t *type;
};

/*!
 * \brief Choice types.  These are tagged, disjoint unions.
 */
struct bdxl_choice_node_data_t {
        /*!
         * \brief Number of option descriptors.
         */
        unsigned int noptions;
        /*!
         * \brief Option descriptors.
         */
        bdxl_choice_opt_node_t options[];
};

/*!
 * \brief Sequence-of types.
 */
struct bdxl_seq_of_node_data_t {
        /*!
         * \brief Minimum number of elements (can be 0).
         */
        unsigned int minsize;
        /*!
         * \brief Minimum number of elements (can be 0).
         */
        unsigned int maxsize;
        /*!
         * \brief Whether or not elements are unique.
         */
        bool unique;
        /*!
         * \brief Whether or not the ordering of elements matters.
         *
         * Note: this does NOT imply that the sequence is sorted.
         */
        bool ordered;
        /*!
         * \brief The type of sequence elements.
         */
        bdxl_type_node_t *elemtype;
};

/*!
 * \brief Family-specific type descriptors.
 */
union bdxl_type_node_data_t {
        /*!
         * \brief Data for BDXL_TYPE_SINT.
         */
        bdxl_sint_node_data_t sint_data;
        /*!
         * \brief Data for BDXL_TYPE_UINT.
         */
        bdxl_uint_node_data_t uint_data;
        /*!
         * \brief Data for BDXL_TYPE_REAL.
         */
        bdxl_real_node_data_t real_data;
        /*!
         * \brief Data for BDXL_TYPE_SEQ.
         */
        bdxl_seq_node_data_t seq_data;
        /*!
         * \brief Data for BDXL_TYPE_CHOICE.
         */
        bdxl_choice_node_data_t choice_data;
        /*!
         * \brief Data for BDXL_TYPE_SEQ_OF.
         */
        bdxl_seq_of_node_data_t seq_of_data;
};

/*!
 * \brief Type descriptor.
 */
struct bdxl_type_node_t {
        /*!
         * \brief Position in source that this node represents.
         */
        pos_t pos;
        /*!
         * \brief Tag indicating the family of the type.
         */
        bdxl_type_tag_t tag;
        /*!
         * \brief Tag-specific metadata about the type.
         */
        bdxl_type_node_data_t data;
};

/*!
 * \brief Recursively free a bdxl_seq_field_node_t.
 * \param node The bdxl_seq_field_node_t to free.
 */
extern void bdxl_seq_field_node_free(bdxl_seq_field_node_t *node);

/*!
 * \brief Recursively free a bdxl_choice_opt_node_t.
 * \param node The bdxl_choice_opt_node_t to free.
 */
extern void bdxl_choice_opt_node_free(bdxl_choice_opt_node_t *node);

/*!
 * \brief Recursively free a bdxl_type_node_t.
 * \param node The bdxl_type_node_t to free.
 */
extern void bdxl_type_node_free(bdxl_type_node_t *node);

#endif
