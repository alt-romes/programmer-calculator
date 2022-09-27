#ifndef _OPERATORS_H
#define _OPERATORS_H

#define DEFAULT_MASK -1
#define DEFAULT_MASK_SIZE 64

/* Supress unused parameter warnings */
#ifdef __GNUC__
#   define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#   define UNUSED(x) UNUSED_##x
#endif

#define ALL_OPS "+-*/&|$^<>:;%~_@"

#define OR_SYMBOL '|'
#define NOR_SYMBOL '$'
#define XOR_SYMBOL '^'
#define AND_SYMBOL '&'
#define SHR_SYMBOL '>'
#define SHL_SYMBOL '<'
#define ROR_SYMBOL ';'
#define ROL_SYMBOL ':'
#define MUL_SYMBOL '*'
#define DIV_SYMBOL '/'
#define MOD_SYMBOL '%'
#define ADD_SYMBOL '+'
#define SUB_SYMBOL '-'
#define NOT_SYMBOL '~'
#define TWOSCOMPLEMENT_SYMBOL '_'
#define SWAPENDIANNESS_SYMBOL '@'

// Operations Control
// Example: '+' takes two operands, therefore the noperands = 2
typedef struct operation {
    char character;
    unsigned char noperands;
    long long (*execute) (long long, long long);
} operation;

extern unsigned long long globalmask;
extern int globalmasksize;
extern operation *current_op;

operation* getopcode(char c);

long long shr(long long, long long);
long long ror(long long, long long);


#endif
