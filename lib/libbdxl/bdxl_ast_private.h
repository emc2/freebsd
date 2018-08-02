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
#ifndef _BDXL_AST_PRIVATE_H_
#define _BDXL_AST_PRIVATE_H_

#include "bdxl_ast.h"

typedef struct bdxl_seq_field_list_t bdxl_seq_field_list_t;
typedef struct bdxl_choice_opt_list_t bdxl_choice_opt_list_t;

/*!
 * This is a list of data used to construct a bdxl_seq_field_node_t.
 * This used to build up lists of bdxl_seq_field_node_t's to be used
 * to then construct a bdxl_seq_node_t.
 *
 * \brief A list node for bdxl_seq_field_node_t data.
 */
struct bdxl_seq_field_list_t {
        /*!
         * \brief The node data.
         */
        bdxl_seq_field_node_t data;
        /*!
         * \brief The next node in the list, or NULL.
         */
        bdxl_seq_field_list_t *next;
};

/*!
 * This is a list of data used to construct a bdxl_choice_opt_node_t.
 * This used to build up lists of bdxl_choice_opt_node_t's to be used
 * to then construct a bdxl_choice_node_t.
 *
 * \brief A list node for bdxl_choice_opt_node_t data.
 */
struct bdxl_choice_opt_list_t {
        /*!
         * \brief The node data.
         */
        bdxl_choice_opt_node_t data;
        /*!
         * \brief The next node in the list, or NULL.
         */
        bdxl_choice_opt_list_t *next;
};

/*!
 * \brief Create an AST node representing a unit type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_unit_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol);

/*!
 * \brief Create an AST node representing a boolean type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_bool_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol);

/*!
 * \brief Create an AST node representing a signed integer type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param lo The lower bound on the integer value.
 * \param lo The upper bound on the integer value.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_sint_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    int64_t lo, int64_t hi);

/*!
 * \brief Create an AST node representing at unsigned integer type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param lo The lower bound on the integer value.
 * \param lo The upper bound on the integer value.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_uint_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    int64_t lo, int64_t hi);

/*!
 * \brief Create an AST node representing at real-like type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param lo The lower bound on the integer value.
 * \param lo The upper bound on the integer value.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_real_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    double lo, double hi);

/*!
 * \brief Create an AST node representing a string type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_string_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol);

/*!
 * \brief Create an AST node representing a sequence type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param ordered Whether or not the order of the elements is preserved.
 * \param fields A list of fields in reverse order.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_seq_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    bool ordered, bdxl_seq_field_list_t *list);

/*!
 * \brief Create an AST node representing a choice type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param fields A list of options in reverse order.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_choice_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    bdxl_choice_opt_list_t *list);

/*!
 * \brief Create an AST node representing a sequence-of type.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param minsize The minimum number of elements in the sequence.
 * \param maxsize The maximum number of elements in the sequence.
 * \param unique Whether or not the sequence elements are unique.
 * \param ordered Whether or not the order of the elements is preserved.
 * \param elemtype The type of sequence elements.
 * \return The newly-created node.
 */
extern bdxl_type_node_t * bdxl_seq_of_type_node_t(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    size_t minsize, size_t maxsize, bool unique, bool ordered,
    bdxl_type_node_t *elemtype);

/*!
 * \brief Create an AST node representing a sequence field.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param name The field name.
 * \param type The field type.
 * \return The newly-created node.
 */
extern bdxl_seq_field_node_t * bdxl_seq_field_node(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    const char *name, bdxl_type_node_t *type);

/*!
 * \brief Create an AST node representing a choice option.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param name The option name.
 * \param type The option type.
 * \return The newly-created node.
 */
extern bdxl_choice_opt_node_t * bdxl_choice_opt_node(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    const char *name, bdxl_type_node_t *type);

/*!
 * \brief Create an AST node representing a sequence field.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param name The field name.
 * \param type The field type.
 * \param next The list to which to prepend.
 * \return The newly-created node.
 */
extern bdxl_seq_field_list_t * bdxl_seq_field_list(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    const char *name, bdxl_type_node_t *type, bdxl_seq_field_list_t *next);

/*!
 * \brief Create an AST node representing a choice option.
 * \param fname The file name for the position.
 * \param line The line number for the position.
 * \param startcol The inclusive starting column for the position.
 * \param endcol The inclusize ending column for the position.
 * \param name The option name.
 * \param type The option type.
 * \param next The list to which to prepend.
 * \return The newly-created node.
 */
extern bdxl_choice_opt_list_t * bdxl_choice_opt_list(const char *fname,
    unsigned int line, unsigned int startcol, unsigned int endcol,
    const char *name, bdxl_type_node_t *type, bdxl_choice_opt_list_t *next);

/*!
 * \brief Recursively free a bdxl_seq_field_list_t, but not its type fields.
 * \param node The bdxl_seq_field_list_t to free.
 */
extern void bdxl_seq_field_list_free(bdxl_seq_field_list_t *list);

/*!
 * \brief Recursively free a bdxl_choice_opt_list_t, but not its type fields.
 * \param node The bdxl_choice_opt_list_t to free.
 */
extern void bdxl_choice_opt_list_free(bdxl_choice_opt_list_t *list);
#endif
