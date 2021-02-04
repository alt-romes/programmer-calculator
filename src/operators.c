#include <stddef.h>

#include "operators.h"

unsigned long long globalmask = DEFAULT_MASK;
int globalmasksize = DEFAULT_MASK_SIZE;

static long long add(long long, long long);
static long long subtract(long long, long long);
static long long multiply(long long, long long);
static long long divide(long long, long long);
static long long and(long long, long long);
static long long or(long long, long long);
static long long nor(long long, long long);
static long long xor(long long, long long);
static long long shl(long long, long long);
static long long rol(long long, long long);
static long long modulus(long long, long long);
static long long not(long long, long long);
static long long twos_complement(long long, long long);

static operation operations[15] = {
    {'+', 2, add},
    {'-', 2, subtract},
    {'*', 2, multiply},
    {'/', 2, divide},
    {'&', 2, and},
    {'|', 2, or},
    {'$', 2, nor},
    {'^', 2, xor},
    {'<', 2, shl},
    {'>', 2, shr},
    {'(', 2, rol},
    {')', 2, ror},
    {'%', 2, modulus},
    {'~', 1, not},
    {'\'', 1, twos_complement}
};

operation* getopcode(char c)  {

    for (unsigned long i=0; i < sizeof(operations); i++)
        if (operations[i].character == c)
            return &operations[i];

    return NULL;
}


static long long add(long long a, long long b) {

    return a + b;
}

// remember op1 = first popped ( right operand ), op2 = second popped ( left operand )
static long long subtract(long long a, long long b) {

    return b - a;
}
static long long multiply(long long a, long long b) {

    return a * b;
}

static long long divide(long long a, long long b) {

    //TODO not divisible by 0
    if(!a)
        return 0;
    return b / a;
}

static long long and(long long a, long long b) {

    return a & b;
}

static long long or(long long a, long long b) {

    return a | b;
}

static long long nor(long long a, long long b) {

    return ~(a | b);
}

static long long xor(long long a, long long b) {

    return a ^ b;
}
static long long shl(long long a, long long b) {

    return b << a;
}

long long shr(long long a, long long b) {

    return (b >> a) & ~((long long) -1 << (64-a));
}

static long long rol(long long a, long long b) {

    return b << a | shr(globalmasksize-a, b);
}

long long ror(long long a, long long b) {

    return shr(a, b) | ( b << (globalmasksize- a) );
}

static long long modulus(long long a, long long b) {

    //TODO not divisible by 0
    if(!a)
        return 0;

    return b % a;
}

static long long not(long long a, long long UNUSED(b)) {

    return ~a;
}

static long long twos_complement(long long a, long long UNUSED(b)) {

    return -a;
}
