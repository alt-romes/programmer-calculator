#ifndef _PARSER_H
#define _PARSER_H

#include "operators.h"

#define MAX_TOKENS 80

#define VALID_TOKENS "+-*/%&|$^~<>():;_@0123456789abcdefABCDEFx"
#define VALID_NUMBER_INPUT "0123456789abcdefx()"
#define VALID_DEC_SYMBOLS "0123456789"
#define VALID_HEX_SYMBOLS "0123456789abcdefABCDEF"
#define VALID_BIN_SYMBOLS "01"

#define LPAR_SYMBOL '('
#define RPAR_SYMBOL ')'

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
    int ntokens;
    int pos;
} * parser_t;

char* tokenize(char*);
exprtree parse(char*);
long long calculate(exprtree);
void free_exprtree(exprtree);

extern int total_trees_created;
extern int total_trees_freed;
extern int total_parsers_created;
extern int total_parsers_freed;
extern int total_tokens_created;
extern int total_tokens_freed;

#endif
