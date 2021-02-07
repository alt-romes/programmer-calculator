#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

// Static functions
static char* tokenize(char*);

static exprtree parse_expr(parser_t);
static exprtree parse_add_exp(parser_t);
static exprtree parse_mult_exp(parser_t);
static exprtree parse_number(parser_t);
static exprtree create_exprtree(int, void*, exprtree, exprtree);
static void free_exprtree(exprtree);


// Main
int main(int argc, char* argv[]) {

    argc = 1;
    argv = NULL;

    char in[MAX_TOKENS];

    printf("\nEnter input: ");
    scanf("%[^\n]", in);

    // Parse the expression
    exprtree expression = parse(in);

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

exprtree parse(char* in) {

    // TODO: How to stop with errors?

    parser_t parser = malloc(sizeof(parser_t));
    parser->tokens = tokenize(in);

    exprtree expression = parse_expr(parser);

    free(parser->tokens);
    free(parser);
    
    return expression;
}

static exprtree parse_expr(parser_t parser) {

    return parse_add_exp(parser);
}

static exprtree parse_add_exp(parser_t parser) {

    exprtree expr = parse_mult_exp(parser);
    
    while (parser->tokens[parser->pos] == '+' || parser->tokens[parser->pos] == '-') {

        operation* op = getopcode(parser->tokens[parser->pos]);

        parser->pos++; // Consume token

        exprtree right_expr = parse_mult_exp(parser);

        expr = create_exprtree(OP_TYPE, op, expr, right_expr);
    }
    
    return expr;

}

static exprtree parse_mult_exp(parser_t parser) {

    exprtree expr = parse_number(parser);
    
    while (parser->tokens[parser->pos] == '*' || parser->tokens[parser->pos] == '/' || parser->tokens[parser->pos] == '%') {

        operation* op = getopcode(parser->tokens[parser->pos]);

        parser->pos++; // Consume token

        exprtree right_expr = parse_number(parser);

        expr = create_exprtree(OP_TYPE, op, expr, right_expr);
    }
    
    return expr;
}


static exprtree parse_number(parser_t parser) {
    
    int sign_prefix = 1;
    if (strchr(VALID_SIGN_SYMBOLS, parser->tokens[parser->pos])) {

        // Find + or - before the number (to make numbers positive or negative)

        // When the symbol found is +, there's no need to do anything

        if (parser->tokens[parser->pos] == MINUS_SYMBOL)
            sign_prefix = -1;

        parser->pos++; // Consume token

    }

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

    long long value = sign_prefix * strtoll(numberfound, NULL, numberbase);

    exprtree number_expr = create_exprtree(numbertype, &value, NULL, NULL);

    return number_expr;
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

    if (expr->left)
        free_exprtree(expr->left);
    if (expr->right)
        free_exprtree(expr->right);

    free(expr);
}

static char* tokenize(char* in) {

    char* tokens = malloc(sizeof(char) * MAX_TOKENS);

    int in_len = strlen(in);
    for (int i = 0, token_pos = 0; i < in_len; i++)
        if (strchr(VALID_TOKENS, in[i]))
            tokens[token_pos++] = in[i];

    return tokens;
}
