#ifndef _PARSER_H
#define _PARSER_H

#include "operators.h"

#define MAX_TOKENS 80

#define VALID_TOKENS "+-*/%&|$^~<>()'rl0123456789abcdefx"
#define VALID_SYMBOLS "+-*/%&|$^~<>rl"
#define VALID_SIGN_SYMBOLS "+-"
#define VALID_DEC_SYMBOLS "0123456789"
#define VALID_HEX_SYMBOLS "0123456789abcdef"
#define VALID_BIN_SYMBOLS "01"


#define OR_SYMBOL '|'
#define NOR_SYMBOL '$'
#define MINUS_SYMBOL '-'

#define OP_TYPE 0
#define DEC_TYPE 1
#define HEX_TYPE 2
#define BIN_TYPE 3

typedef struct exprtree {
    int type;
    union {
        operation* op;
        long long* value;
    };
    struct exprtree* left;
    struct exprtree* right;
} * exprtree;

typedef struct parser_t {
    char* tokens;
    int pos;
} * parser_t;

exprtree parse(char*);
long long calculate(exprtree);

#endif
