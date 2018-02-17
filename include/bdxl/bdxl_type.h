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
#ifndef _BDXL_TYPE_H_
#define _BDXL_TYPE_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum bdxl_type_tag_t bdxl_type_tag_t;
typedef struct bdxl_sint_data_t bdxl_sint_data_t;
typedef struct bdxl_uint_data_t bdxl_uint_data_t;
typedef struct bdxl_real_data_t bdxl_real_data_t;
typedef struct bdxl_seq_data_t bdxl_seq_data_t;
typedef struct bdxl_seq_field_t bdxl_seq_field_t;
typedef struct bdxl_choice_data_t bdxl_choice_data_t;
typedef struct bdxl_choice_opt_t bdxl_choice_opt_t;
typedef struct bdxl_seq_of_data_t bdxl_seq_of_data_t;
typedef union bdxl_type_data_t bdxl_type_data_t;
typedef struct bdxl_type_t bdxl_type_t;

/*!
 * \brief Type tags for BDXL.
 */
enum bdxl_type_tag_t {
        /*!
         * \brief Unit type.
         *
         * This is a similar to a C void, except it can be declared.
         * It is often used with BDXL_TYPE_CHOICE to implement
         * variants that have no data (enums can be implemented this
         * way).
         */
        BDXL_TYPE_UNIT,
        /*!
         * \brief Boolean type.
         */
        BDXL_TYPE_BOOL,
        /*!
         * \brief Signed integer types.
         */
        BDXL_TYPE_SINT,
        /*!
         * \brief Unsigned integer types.
         */
        BDXL_TYPE_UINT,
        /*!
         * \brief Real-like types (float, double).
         */
        BDXL_TYPE_REAL,
        /*!
         * \brief String types.
         */
        BDXL_TYPE_STRING,
        /*!
         * \brief Sequences, consisting of ordered named unique fields.
         */
        BDXL_TYPE_SEQ,
        /*!
         * \brief Choices, consisting of one or more named variants.
         */
        BDXL_TYPE_CHOICE,
        /*!
         * \brief  Unordered sequences of elements having the same type.
         */
        BDXL_TYPE_SEQ_OF,
};

/*!
 * \brief Signed integer data.
 */
struct bdxl_sint_data_t {
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
struct bdxl_uint_data_t {
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
struct bdxl_real_data_t {
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
struct bdxl_seq_field_t {
        /*!
         * \brief The field's name.
         */
        char* name;
        /*!
         * \brief The field's type descriptor.
         */
        bdxl_type_t *type;
};

/*!
 * \brief Sequential types.
 *
 * These include structures, sequences, and sets.
 */
struct bdxl_seq_data_t {
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
        bdxl_seq_field_t fields[];
};

/*!
 * \brief Fields for choice types.
 */
struct bdxl_choice_opt_t {
        /*!
         * \brief The option's name.
         */
        char* name;
        /*!
         * \brief The option's type descriptor.
         */
        bdxl_type_t *type;
};

/*!
 * \brief Choice types.  These are tagged, disjoint unions.
 */
struct bdxl_choice_data_t {
        /*!
         * \brief Number of option descriptors.
         */
        unsigned int noptions;
        /*!
         * \brief Option descriptors.
         */
        bdxl_choice_opt_t options[];
};

/*!
 * \brief Sequence-of types.
 */
struct bdxl_seq_of_data_t {
        unsigned int minlen;
        unsigned int maxlen;
        bool unique;
        bool ordered;
        bdxl_type_t *elemtype;
};

/*!
 * \brief Family-specific type descriptors.
 */
union bdxl_type_data_t {
        /*!
         * \brief Data for BDXL_TYPE_SINT.
         */
        bdxl_sint_data_t sint_data;
        /*!
         * \brief Data for BDXL_TYPE_UINT.
         */
        bdxl_uint_data_t uint_data;
        /*!
         * \brief Data for BDXL_TYPE_REAL.
         */
        bdxl_real_data_t real_data;
        /*!
         * \brief Data for BDXL_TYPE_SEQ.
         */
        bdxl_seq_data_t seq_data;
        /*!
         * \brief Data for BDXL_TYPE_CHOICE.
         */
        bdxl_choice_data_t choice_data;
        /*!
         * \brief Data for BDXL_TYPE_SEQ_OF.
         */
        bdxl_seq_of_data_t seq_of_data;
};

/*!
 * \brief Type descriptor.
 */
struct bdxl_type_t {
        /*!
         * \brief Tag indicating the family of the type.
         */
        bdxl_type_tag_t tag;
        /*!
         * \brief Tag-specific metadata about the type.
         */
        bdxl_type_data_t data;
};

#endif
