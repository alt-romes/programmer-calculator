#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "xmalloc.h"

// Static functions

static exprtree parse_expr(parser_t);
static exprtree parse_or_expr(parser_t);
static exprtree parse_xor_expr(parser_t);
static exprtree parse_and_expr(parser_t);
static exprtree parse_shift_expr(parser_t);
static exprtree parse_add_expr(parser_t);
static exprtree parse_mult_expr(parser_t);
static exprtree parse_prefix_expr(parser_t);
static exprtree parse_atom_expr(parser_t);
static exprtree parse_number(parser_t);

static exprtree parse_stdop_expr(parser_t, char*, exprtree (*) (parser_t));

static exprtree create_exprtree(int, void*, exprtree, exprtree);

int total_trees_created = 0;
int total_trees_freed = 0;
int total_parsers_created = 0;
int total_parsers_freed = 0;
int total_tokens_created = 0;
int total_tokens_freed = 0;


// For a simpler version of this parser check github.com/alt-romes/calculator-c-parser

/**
 * @brief Sanitize input to only allowed characters
 *
 * Mallocs a new string with only allowed characters
 */
char* sanitize(const char* in) {

    char* output = xmalloc(sizeof(char) * MAX_CHARS);

    int in_len = strlen(in);
    int token_pos = 0;
    for (int i = 0; i < in_len; i++)
        if (strchr(VALID_TOKENS, in[i]))
            output[token_pos++] = in[i];

    output[token_pos] = '\0';

    total_tokens_created++;
    return output;
}


/**
 * @brief Parse sanitized input into an expression tree
 *
 * Entry point to the parser. Frees input after parsing.
 */
exprtree parse(char* input) {

    // TODO: How to stop with errors?

    // attention: allocate size for *struct parser_t*, because *parser_t* is type defined as a pointer to *struct parser_t*
    parser_t parser = xmalloc(sizeof(struct parser_t));
    total_parsers_created++;

    assert(input != NULL);
    parser->tokens = input;

    int ntokens = strlen(input);
    assert(ntokens > 0);
    parser->ntokens = ntokens;

    parser->pos = 0;

    exprtree expression = parse_expr(parser);

    free(parser->tokens);
    free(parser);
    total_parsers_freed++;

    total_tokens_freed++;
    return expression;
}

/**
 * @brief Calculate a numeric value from an expression tree
 */
uint64_t calculate(exprtree expr) {

    // expr shouldn't be null if being calculated.
    assert(expr != NULL);
    // all expressions have 2 operands because 1 operand operators are taken care of immediately

    if (expr->type == OP_TYPE) {

        uint64_t left_value = calculate(expr->left);

        uint64_t right_value = calculate(expr->right);

        // Execute takes the operands switched because the stack inverts the order of the numbers
        uint64_t value = expr->op->execute(right_value, left_value);

        return value & globalmask;

    }
    else {

        // Expression is a leaf (is a number) - so return the number directly

        return *(expr->value) & globalmask;
    }

}

/**
 * @brief Free an expression tree and all its children
 */
void free_exprtree(exprtree expr) {

    if (expr) {

        if (expr->left)
            free_exprtree(expr->left);
        if (expr->right)
            free_exprtree(expr->right);


        if (expr->type != OP_TYPE)
            free(expr->value);

        free(expr);

        total_trees_freed++;

    }

}

/**
 * @brief Parse sub-string in parser into expression tree
 *
 * Works by recursively calling different parse functions until all tokens gets consumed.
 * Sarts with lowest precedence.
 * Inner recursive calls, which get executed first, are higher precedence.
 */
static exprtree parse_expr(parser_t parser) {

    // Grammar rule: expression := or_exp

    return parse_or_expr(parser);
}

static exprtree parse_or_expr(parser_t parser) {

    // Grammar rule: or_exp := xor_exp ( (| | $) xor_exp )*

    char ops[] = {OR_SYMBOL, NOR_SYMBOL, '\0'};

    return parse_stdop_expr(parser, ops, parse_xor_expr);

}

static exprtree parse_xor_expr(parser_t parser) {

    // Grammar rule: xor_exp := and_exp (^ and_exp)*

    char ops[] = {XOR_SYMBOL, '\0'};

    return parse_stdop_expr(parser, ops, parse_and_expr);

}

static exprtree parse_and_expr(parser_t parser) {

    // Grammar rule: and_exp := shift_exp (& shift_exp)*

    char ops[] = {AND_SYMBOL, '\0'};

    return parse_stdop_expr(parser, ops, parse_shift_expr);

}

static exprtree parse_shift_expr(parser_t parser) {

    // Grammar rule: shift_exp := add_exp ((<< | >> | ror | rol) add_exp)*

    char ops[] = {SHR_SYMBOL, SHL_SYMBOL, ROR_SYMBOL, ROL_SYMBOL, '\0'};

    return parse_stdop_expr(parser, ops, parse_add_expr);

}

static exprtree parse_add_expr(parser_t parser) {

    // Grammar rule: add_exp := mult_exp ((+ | -) mult_exp)*

    char ops[] = {ADD_SYMBOL, SUB_SYMBOL, '\0'};

    return parse_stdop_expr(parser, ops, parse_mult_expr);

}

static exprtree parse_mult_expr(parser_t parser) {

    // Grammar rule: mult_exp := not_exp ((* | / | %) not_exp)*

    char ops[] = {MUL_SYMBOL, DIV_SYMBOL, MOD_SYMBOL, '\0'};

    return parse_stdop_expr(parser, ops, parse_prefix_expr);

}

static exprtree parse_prefix_expr(parser_t parser) {

    // Grammar rule: prefix_exp := (~ | + | - | @)? atom_exp

    // TODO: Display input invalid instead of using a zero-val expression
    if (!(parser->pos < parser->ntokens)) {

        uint64_t zerov = 0;
        return create_exprtree(DEC_TYPE, &zerov, NULL, NULL);
    }


    char prefixes[] = {ADD_SYMBOL, SUB_SYMBOL, NOT_SYMBOL, TWOSCOMPLEMENT_SYMBOL, SWAPENDIANNESS_SYMBOL, '\0'};

    // If we've exceeded the number of tokens we should detect an error
    assert(parser->pos < parser->ntokens);

    char prefix = 0;
    if (strchr(prefixes, parser->tokens[parser->pos])) {

        // Find + or - before the number (to make numbers positive or negative)

        // When the symbol found is +, there's no need to do anything

        prefix = parser->tokens[parser->pos];

        parser->pos++; // Consume token
    }

    exprtree expr = parse_atom_expr(parser);

    if (prefix == 0 || prefix == ADD_SYMBOL) // Do nothing to expression
        return expr;
    else {

        // Prefix is either SUB_SYMBOL, NOT_SYMBOL, TWOSCOMPLEMENT_SYMBOL or SWAPENDIANNESS_SYMBOL

        // We get the operation to use in the expression
        operation* op = getopcode(prefix);

        // SUB sets the symmetric of number with the expression (0 - expression), so we create a
        // subtraction expression with 0 as the left tree

        // Bitwise NOT of a number - only one parameter is used:
        // And because the order is changed in execute(), put a number in the right expr instead of the left one
        // apply the NOT operation to the number on the right branch. The other branch doesn't matter so we set it as the same as SUB

        // Two's complement serves the same logic - since it only uses one parameter we can set the other as anything

        // So we create an expression with 0 on the left, and the correct op, and it works
        uint64_t zero_val = 0;
        exprtree zero_val_expr = create_exprtree(DEC_TYPE, &zero_val, NULL, NULL);
        return create_exprtree(OP_TYPE, op, zero_val_expr, expr);
    }

}

/**
 * @brief Tries to parse next expression which may consist of parentheses or a number
 */
static exprtree parse_atom_expr(parser_t parser) {

    // Grammar rule: atom_expr := number | left_parenthesis expression right_parenthesis

    // TODO: An error should be displayed here instead of return a zero value expression.
    // This assertion fails if the only token is a prefix, the prefix is read, and this function is called without enough tokens
    // to be parsed. There are possibly more cases
    if (!(parser->pos < parser->ntokens)) {

        uint64_t zerov = 0;
        return create_exprtree(DEC_TYPE, &zerov, NULL, NULL);
    }

    // If we've exceeded the number of tokens we should detect an error
    assert(parser->pos < parser->ntokens);

    exprtree expr;

    if (parser->tokens[parser->pos] == LPAR_SYMBOL) {
        // If the atomic expression starts with parenthesis

        parser->pos++; // Consume left parenthesis

        expr = parse_expr(parser);

        // If we've exceeded the number of tokens we should detect an error
        // This assertion is triggered if an expression without right parenthesis is the input
        // It should be handled as an error below
        /* assert(parser->pos < parser->ntokens); */

        if (parser->tokens[parser->pos] == RPAR_SYMBOL)
            parser->pos++; // Consume right parenthesis
        else {

            // For now, everything to the right of an unclosed left parenthesis will be equivalent to 0
            uint64_t zerov = 0;
            return create_exprtree(DEC_TYPE, &zerov, NULL, NULL);

            // TODO: Find a way to do error handling and displaying, possibly give one more type to exprtree type = ERR_TYPE and have in the union a char* for the error message
            /* fprintf(stderr, "Invalid expression!!!\n"); */
        }

    }
    else {
        // If it doesn't start with parenthesis then it's a normal number

        expr = parse_number(parser);
    }

    return expr;

}

/**
 * @brief Tries to parse next token as a number
 */
static exprtree parse_number(parser_t parser) {

    // Grammar rule: number: ( (0-9)+ | 0?x(0-9a-f)+ | 0?b(0-1)+ )

    // If we've exceeded the number of tokens we should detect an error
    assert(parser->pos < parser->ntokens);

    int numbertype = DEC_TYPE;
    if (parser->pos+1 < parser->ntokens) {
        switch (parser->tokens[parser->pos]) {
            case '0':
                // check second character
                switch(parser->tokens[parser->pos+1]) {
                    case 'b': // 0b0101
                        numbertype = BIN_TYPE;
                        parser->pos += 2;
                        break;
                    case 'x': // 0xffff
                        numbertype = HEX_TYPE;
                        parser->pos += 2;
                        break;
                    default:  // number is decimal
                        break;
                }
                break;
            case 'x': // xffff
                numbertype = HEX_TYPE;
                parser->pos += 1;
                break;
            case 'b': // b0101
                numbertype = BIN_TYPE;
                parser->pos += 1;
                break;
        }

    }

    char numberfound[MAX_CHARS + 1];
    int numberlen = 0;

    while ( parser->pos < parser->ntokens &&
            ((numbertype == DEC_TYPE && strchr(VALID_DEC_SYMBOLS, parser->tokens[parser->pos]))
            || (numbertype == HEX_TYPE && strchr(VALID_HEX_SYMBOLS, parser->tokens[parser->pos]))
            || (numbertype == BIN_TYPE && strchr(VALID_BIN_SYMBOLS, parser->tokens[parser->pos]))) ) {

        numberfound[numberlen++] = parser->tokens[parser->pos];

        parser->pos++; // Consume 1 digit (1 token)

    }
    numberfound[numberlen] = '\0';

    // If no number was found, return for now a zero value expression
    //
    // TODO: return the error expression instead
    // this happens when the input is valid tokens like abc, but then the number doesn't start with 0x,
    // and possibly in other situations
    if (numberlen == 0) {

        uint64_t zerov = 0;
        return create_exprtree(DEC_TYPE, &zerov, NULL, NULL);
    }

    // Else, create the expression from the found number
    int numberbase = 0;
    switch (numbertype) {
        case DEC_TYPE:
            numberbase = 10;
            break;
        case HEX_TYPE:
            numberbase = 16;
            break;
        case BIN_TYPE:
            numberbase = 2;
            break;
    }

    uint64_t value = strtoull(numberfound, NULL, numberbase);

    exprtree number_expr = create_exprtree(numbertype, &value, NULL, NULL);

    return number_expr;
}

/**
 * @brief Tries to parse next binary expression with any of chars in ops as operator
 *
 * First tries to parse left side of operation with parse_inner_expr.
 * This means that parse_inner_expr gets a higher priority in operator precedence.
 * Then looks for any of the chars in ops, if found, tries to parse right side of operation with parse_inner_expr.
 * If the operator was not found, this means that parsing of the current sub-string is done.
 */
static exprtree parse_stdop_expr(parser_t parser, char* ops, exprtree (*parse_inner_expr) (parser_t)) {

    // TODO: We don't want to do this - when the input is badly formatted an error should be displayed.
    // This is a temporary fix that returns the expression immediately as zero.
    if (!(parser->pos < parser->ntokens)) {

        uint64_t zerov = 0;
        return create_exprtree(DEC_TYPE, &zerov, NULL, NULL);
    }

    // When the position gets here it should be smaller than the ntokens, or maybe only inner_expr should worry about it?
    assert(parser->pos < parser->ntokens);

    exprtree expr = parse_inner_expr(parser);

    while (parser->pos < parser->ntokens && strchr(ops, parser->tokens[parser->pos])) {

        operation* op = getopcode(parser->tokens[parser->pos]);

        parser->pos++; // Consume token

        exprtree right_expr = parse_inner_expr(parser);

        expr = create_exprtree(OP_TYPE, op, expr, right_expr);
    }

    return expr;

}

/**
 * @brief Create a new expression tree node.
 */
static exprtree create_exprtree(int type, void* content, exprtree left, exprtree right) {

    // attention: allocate size for *struct exprtree*, because *exprtree* is type defined as a pointer to *struct exprtree*
    exprtree expr = xmalloc(sizeof(struct exprtree));

    expr->type = type;

    if (type == OP_TYPE)
        expr->op = getopcode(*((char*) content));

    else {

        void* allocated[] = { expr };
        expr->value = xmalloc_with_ressources(sizeof(*expr->value), allocated, 1);
        *(expr->value) = *((uint64_t*) content);
    }

    expr->left = left;
    expr->right = right;

    total_trees_created++;
    return expr;

}

