#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

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


// For a simpler version of this parser check github.com/alt-romes/calculator-c-parser 

char* tokenize(char* in) {

    char* tokens = xmalloc(sizeof(char) * MAX_TOKENS);

    int in_len = strlen(in);
    int token_pos = 0;
    for (int i = 0; i < in_len; i++)
        if (strchr(VALID_TOKENS, in[i]))
            tokens[token_pos++] = in[i];

    tokens[token_pos] = '\0';

    return tokens;
}


exprtree parse(char* tokens) {

    // TODO: How to stop with errors?

    // attention: allocate size for *struct parser_t*, because *parser_t* is type defined as a pointer to *struct parser_t*
    parser_t parser = xmalloc(sizeof(struct parser_t));

    assert(tokens != NULL);
    parser->tokens = tokens;

    int ntokens = strlen(tokens);
    assert(ntokens > 0);
    parser->ntokens = ntokens;

    parser->pos = 0;

    exprtree expression = parse_expr(parser);

    free(parser->tokens);
    free(parser);
    
    return expression;
}

long long calculate(exprtree expr) {

    // expr shouldn't be null if being calculated.
    assert(expr != NULL);
    // all expressions have 2 operands because 1 operand operators are taken care of immediately

    if (expr->type == OP_TYPE) {

        long long left_value = calculate(expr->left);

        long long right_value = calculate(expr->right);

        // Execute takes the operands switched because the stack inverts the order of the numbers
        long long value = expr->op->execute(right_value, left_value);

        return value & globalmask;

    }
    else {

        // Expression is a leaf (is a number) - so return the number directly

        return *(expr->value) & globalmask;
    }
        
}

void free_exprtree(exprtree expr) {

    if (expr) {

        if (expr->left)
            free_exprtree(expr->left);
        if (expr->right)
            free_exprtree(expr->right);

        free(expr);

    }

}

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

    // Grammar rule: prefix_exp := (~ | + | -)? atom_exp

    char prefixes[] = {ADD_SYMBOL, SUB_SYMBOL, NOT_SYMBOL, TWOSCOMPLEMENT_SYMBOL, '\0'};

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

        // Prefix is either SUB_SYMBOL, NOT_SYMBOL or TWOSCOMPLEMENT_SYMBOL

        // We get the operation to use in the expression
        operation* op = getopcode(prefix);

        // SUB sets the symmetric of number with the expression (0 - expression), so we create a
        // subtraction expression with 0 as the left tree

        // Bitwise NOT of a number - only one parameter is used:
        // And because the order is changed in execute(), put a number in the right expr instead of the left one 
        // apply the NOT operation to the number on the right branch. The other branch doesn't matter so we set it as the same as SUB

        // Two's complement serves the same logic - since it only uses one parameter we can set the other as anything

        // So we create an expression with 0 on the left, and the correct op, and it works
        long long zero_val = 0;
        exprtree zero_val_expr = create_exprtree(DEC_TYPE, &zero_val, NULL, NULL);
        return create_exprtree(OP_TYPE, op, zero_val_expr, expr);
    }

}

static exprtree parse_atom_expr(parser_t parser) {

    // Grammar rule: atom_expr := number | left_parenthesis expression right_parenthesis
    
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
            long long zerov = 0;
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

static exprtree parse_number(parser_t parser) {

    // If we've exceeded the number of tokens we should detect an error
    assert(parser->pos < parser->ntokens);
    
    int numbertype = DEC_TYPE;
    if (parser->tokens[parser->pos] == '0' && parser->pos+1 < parser->ntokens) {
        if (parser->tokens[parser->pos+1] == 'x') {
            numbertype = HEX_TYPE;
            parser->pos += 2;
        }
        else if (parser->tokens[parser->pos+1] == 'b') {
            numbertype = BIN_TYPE;
            parser->pos += 2;
        }
    }

    char numberfound[MAX_TOKENS + 1];
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

        long long zerov = 0;
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

    long long value = strtoll(numberfound, NULL, numberbase);

    exprtree number_expr = create_exprtree(numbertype, &value, NULL, NULL);

    return number_expr;
}

static exprtree parse_stdop_expr(parser_t parser, char* ops, exprtree (*parse_inner_expr) (parser_t)) {

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

static exprtree create_exprtree(int type, void* content, exprtree left, exprtree right) {

    // attention: allocate size for *struct exprtree*, because *exprtree* is type defined as a pointer to *struct exprtree*
    exprtree expr = xmalloc(sizeof(struct exprtree));

    expr->type = type;

    if (type == OP_TYPE)
        expr->op = getopcode(*((char*) content));
    else {

        expr->value = xmalloc(sizeof(long long));
        *(expr->value) = *((long long*) content);
    }

    expr->left = left;
    expr->right = right;

    return expr;

}

