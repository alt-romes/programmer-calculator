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


// Operations Control
// Example: '+' takes two operands, therefore the noperands = 2
typedef struct operation {
    char character;
    unsigned char noperands;
    long long (*execute) (long long, long long);
} operation;

extern unsigned long long globalmask;
extern int globalmasksize;

extern operation operations[];

operation* getopcode(char c);

long long sr(long long, long long);
long long rr(long long, long long);


#endif
