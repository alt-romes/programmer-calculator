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

operation* getopcode(char c);

long long add(long long, long long);
long long subtract(long long, long long);
long long multiply(long long, long long);
long long divide(long long, long long);
long long and(long long, long long);
long long or(long long, long long);
long long nor(long long, long long);
long long xor(long long, long long);
long long sl(long long, long long);
long long sr(long long, long long);
long long rl(long long, long long);
long long rr(long long, long long);
long long modulus(long long, long long);
long long not(long long, long long);
long long twos_complement(long long, long long);

#endif
