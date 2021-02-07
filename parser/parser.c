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
static void free_exprtree(exprtree);


// For a clearer explanation of this parser check github.com/alt-romes/calculator-c-parser ? (TODO)

// Main
int main(int argc, char* argv[]) {

    argc = 1;
    argv = NULL;

    char in[MAX_TOKENS];

    printf("\nEnter input: ");
    scanf("%[^\n]", in);

    // Parse the expression
    char* tokens = tokenize(in);

    exprtree expression = parse(tokens);

    long long value = calculate(expression);

    printf("The result of the calculation is %lld\n", value);


    free_exprtree(expression);

    return 0;
}

long long calculate(exprtree expr) {

    if (expr == NULL) {

        // If a NULL expr is being calculated then the operation calling calculate is not going to use it the value return from here.
        // This is because when parsing the input we've made sure all operations have the correct amount of operands.
        // This being said, it doesn't matter what value we return

        return 0;
    }

    if (expr->type == OP_TYPE) {

        long long left_value = calculate(expr->left);

        long long right_value = calculate(expr->right);

        // TODO: Change order of operands in operators.c and then change the order here
        long long value = expr->op->execute(right_value, left_value);

        return value;

    }
    else {

        // Expression is a leaf (is a number) - so return the number directly

        return *(expr->value);
    }
        
}

exprtree parse(char* tokens) {

    // TODO: How to stop with errors?

    parser_t parser = malloc(sizeof(parser_t));
    parser->tokens = tokens;
    parser->ntokens = strlen(tokens);
    parser->pos = 0;

    exprtree expression = parse_expr(parser);

    free(parser->tokens);
    free(parser);
    
    return expression;
}

static exprtree parse_expr(parser_t parser) {

    // Grammar rule: expression := or_exp

    return parse_or_expr(parser);
}

static exprtree parse_or_expr(parser_t parser) {

    // Grammar rule: or_exp := xor_exp ( (| | $) xor_exp )*

    char ops[3];
    ops[0] = OR_SYMBOL;
    ops[1] = NOR_SYMBOL;
    ops[2] = '\0';

    return parse_stdop_expr(parser, ops, parse_xor_expr);

}

static exprtree parse_xor_expr(parser_t parser) {

    // Grammar rule: xor_exp := and_exp (^ and_exp)*

    char ops[2];
    ops[0] = XOR_SYMBOL;
    ops[1] = '\0';

    return parse_stdop_expr(parser, ops, parse_and_expr);

}

static exprtree parse_and_expr(parser_t parser) {

    // Grammar rule: and_exp := shift_exp (& shift_exp)*

    char ops[2];
    ops[0] = AND_SYMBOL;
    ops[1] = '\0';

    return parse_stdop_expr(parser, ops, parse_shift_expr);

}

static exprtree parse_shift_expr(parser_t parser) {

    // Grammar rule: shift_exp := add_exp ((<< | >> | ror | rol) add_exp)*

    char ops[5];
    ops[0] = SHR_SYMBOL;
    ops[1] = SHL_SYMBOL;
    ops[2] = ROR_SYMBOL;
    ops[3] = ROL_SYMBOL;
    ops[4] = '\0';

    return parse_stdop_expr(parser, ops, parse_add_expr);

}

static exprtree parse_add_expr(parser_t parser) {

    // Grammar rule: add_exp := mult_exp ((+ | -) mult_exp)*

    // TODO: This example should go away in favour of a repository containing this simple code explained
    // Example of how all other common parse_expressions work
    // The others use *parse_stdop_expr* which basically factors out the following code:

    char ops[3];
    ops[0] = ADD_SYMBOL;
    ops[1] = SUB_SYMBOL;
    ops[2] = '\0';
    
    return parse_stdop_expr(parser, ops, parse_mult_expr);

}

static exprtree parse_mult_expr(parser_t parser) {

    // Grammar rule: mult_exp := not_exp ((* | / | %) not_exp)*

    char ops[4];
    ops[0] = MUL_SYMBOL;
    ops[1] = DIV_SYMBOL;
    ops[2] = MOD_SYMBOL;
    ops[3] = '\0';

    return parse_stdop_expr(parser, ops, parse_prefix_expr);

}

static exprtree parse_prefix_expr(parser_t parser) {

    // Grammar rule: prefix_exp: (~ | + | -)? atom_exp

    char prefixes[4];
    prefixes[0] = ADD_SYMBOL;
    prefixes[1] = SUB_SYMBOL;
    prefixes[2] = NOT_SYMBOL;
    prefixes[3] = '\0';

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

        // Prefix is either SUB_SYMBOL or NOT_SYMBOL

        // Since it's one of these two, we need to apply an operation to the expression
        operation* op = NULL;

        // Sub sets the symmetric of number, the expression (0 - expression) does that
        if (prefix == SUB_SYMBOL) 
            op = getopcode(SUB_SYMBOL);

        // Bitwise NOT of a number - only one parameter is used: because the order is changed in execute(), set the number to
        // apply the NOT operation to on the right branch. The other branch doesn't matter so we set it as the same as SUB
        else if (prefix == NOT_SYMBOL) 
            op = getopcode(NOT_SYMBOL);

        long long zero_val = 0;
        exprtree zero_val_expr = create_exprtree(DEC_TYPE, &zero_val, NULL, NULL);
        return create_exprtree(OP_TYPE, op, zero_val_expr, expr);
    }

}

static exprtree parse_atom_expr(parser_t parser) {

    // Grammar rule: atom_expr := number | left_parenthesis expression right_parenthesis
    
    exprtree expr;

    if (parser->tokens[parser->pos] == LPAR_SYMBOL) {
        // If the atomic expression starts with parenthesis

        parser->pos++; // Consume left parenthesis

        expr = parse_expr(parser);

        if (parser->tokens[parser->pos] == RPAR_SYMBOL)
            parser->pos++; // Consume right parenthesis
        else
            fprintf(stderr, "Invalid expression!!!\n"); // TODO: Find a way to do error handling and displaying, possibly give one more type to exprtree type = ERR_TYPE and have in the union a char* for the error message

    }
    else {
        // If it doesn't start with parenthesis then it's a normal number

        expr = parse_number(parser);
    }

    return expr;

}

static exprtree parse_number(parser_t parser) {
    
    int numbertype = DEC_TYPE;
    if (parser->tokens[parser->pos] == '0') {
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

    while ( (numbertype == DEC_TYPE && strchr(VALID_DEC_SYMBOLS, parser->tokens[parser->pos]))
            || (numbertype == HEX_TYPE && strchr(VALID_HEX_SYMBOLS, parser->tokens[parser->pos]))
            || (numbertype == BIN_TYPE && strchr(VALID_BIN_SYMBOLS, parser->tokens[parser->pos])) ) {

        numberfound[numberlen++] = parser->tokens[parser->pos];

        parser->pos++; // Consume 1 digit (1 token)

    }
    numberfound[numberlen] = '\0';

    // If no number was found, return the null expression
    // TODO: return the error expression
    if (numberlen == 0)
        return NULL;

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
    exprtree expr = malloc(sizeof(struct exprtree));

    expr->type = type;

    if (type == OP_TYPE)
        expr->op = getopcode(*((char*) content));
    else {

        expr->value = malloc(sizeof(long long));
        *(expr->value) = *((long long*) content);
    }

    expr->left = left;
    expr->right = right;

    return expr;

}

static void free_exprtree(exprtree expr) {

    if (expr) {

        if (expr->left)
            free_exprtree(expr->left);
        if (expr->right)
            free_exprtree(expr->right);

        free(expr);

    }

}

char* tokenize(char* in) {

    char* tokens = malloc(sizeof(char) * MAX_TOKENS);

    int in_len = strlen(in);
    int token_pos = 0;
    for (int i = 0; i < in_len; i++)
        if (strchr(VALID_TOKENS, in[i]))
            tokens[token_pos++] = in[i];

    tokens[token_pos] = '\0';

    return tokens;
}
