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
#ifndef _BDXL_MIRROR_H_
#define _BDXL_MIRROR_H_

/*!
 * \file bdxl.h
 * \brief Mirrors for reflecting on structures described as BDXL types.
 */

#include <bdxl/bdxl_type.h>

typedef struct bdxl_field_mirror_t bdxl_field_mirror_t;
typedef struct bdxl_seq_mirror_t bdxl_seq_mirror_t;
typedef struct bdxl_option_mirror_t bdxl_option_mirror_t;
typedef struct bdxl_choice_mirror_t bdxl_choice_mirror_t;
typedef struct bdxl_seq_of_mirror_t bdxl_seq_of_mirror_t;
typedef union bdxl_mirror_data_t bdxl_mirror_data_t;
typedef struct bdxl_mirror_t bdxl_mirror_t;

/*!
 * \brief Mirror for reflecting on fields.
 */
struct bdxl_field_mirror_t {
        /*!
         * \brief Get the field descriptor.
         */
        const bdxl_seq_field_t * (*get_field_desc)(const void *ptr);
        /*!
         * \brief Get the mirror for the field data.
         */
        const bdxl_mirror_t * (*get_mirror)(const void *ptr);
};

/*!
 * \brief Family-specific mirror data for sequence types.
 */
struct bdxl_seq_mirror_t {
        /*!
         * \brief Get the mirrors for the fields.
         */
        const bdxl_field_mirror_t * const *
            (*get_field_mirrors)(const void *ptr);
};

/*!
 * \brief Mirror for reflecting on options.
 */
struct bdxl_option_mirror_t {
        /*!
         * \brief Get the option descriptor.
         */
        const bdxl_choice_opt_t * (*get_option_desc)(const void *ptr);
        /*!
         * \brief Get the mirror for the option data.
         */
        const bdxl_mirror_t * (*get_mirror)(const void *ptr);
};

/*!
 * \brief Family-specific mirror data for choice types.
 */
struct bdxl_choice_mirror_t {
        /*!
         * \brief Get the mirrors for the options.
         */
        const bdxl_option_mirror_t * const *
            (*get_option_mirrors)(const void *ptr);
};

/*!
 * \brief Mirror for sequence-of types.
 */
struct bdxl_seq_of_mirror_t {
        /*!
         * \brief Get a mirror for the elements.
         */
        const bdxl_mirror_t * (*get_elem_mirror)(const void *ptr);
};

/*!
 * \brief Family-specific mirror data.
 */
union bdxl_mirror_data_t {
        /*!
         * \brief Data for BDXL_TYPE_SEQ.
         */
        bdxl_seq_mirror_t seq_data;
        /*!
         * \brief Data for BDXL_TYPE_CHOICE.
         */
        bdxl_choice_mirror_t choice_data;
        /*!
         * \brief Data for BDXL_TYPE_SEQ_OF.
         */
        bdxl_seq_of_mirror_t seq_of_data;
};

/*!
 * \brief Mirror for reflecting on values.
 */
struct bdxl_mirror_t {
        /*!
         * \brief Get the type descriptor.
         */
        const bdxl_type_t * (*get_type_desc)(const void *ptr);
        /*!
         * \brief Type family metadata.
         *
         * The specific union element is determined by the tag of the
         * type descriptor.
         */
        bdxl_mirror_data_t data;
};

#endif
