#include <stddef.h>

#include "operators.h"

unsigned long long globalmask = DEFAULT_MASK;
int globalmasksize = DEFAULT_MASK_SIZE;

operation operations[16] = {
    {0, 0, NULL},
    {'+', 2, add},
    {'-', 2, subtract},
    {'*', 2, multiply},
    {'/', 2, divide},
    {'&', 2, and},
    {'|', 2, or},
    {'$', 2, nor},
    {'^', 2, xor},
    {'<', 2, sl},
    {'>', 2, sr},
    {'(', 2, rl},
    {')', 2, rr},
    {'%', 2, modulus},
    {'~', 1, not},
    {'\'', 1, twos_complement}
};

operation* getopcode(char c)  {

    operation* r;
    switch (c) {

        case '+':
            r = &operations[1];
            break;
        case '-':
            r = &operations[2];
            break;
        case '*':
            r = &operations[3];
            break;
        case '/':
            r = &operations[4];
            break;
        case '&':
            r = &operations[5];
            break;
        case '|':
            r = &operations[6];
            break;
        case '$':
            r = &operations[7];
            break;
        case '^':
            r = &operations[8];
            break;
        case '<':
            r = &operations[9];
            break;
        case '>':
            r = &operations[10];
            break;
        case '(':
            r = &operations[11];
            break;
        case ')':
            r = &operations[12];
            break;
        case '%':
            r = &operations[13];
            break;
        case '~':
            r = &operations[14];
            break;
        case '\'':
            r = &operations[15];
            break;

    }

    return r;
}


long long add(long long a, long long b) {

    return (a + b) & globalmask;
}

// remember op1 = first popped ( right operand ), op2 = second popped ( left operand )
long long subtract(long long a, long long b) {

    return (b - a) & globalmask;
}
long long multiply(long long a, long long b) {

    return (a * b) & globalmask;
}

long long divide(long long a, long long b) {

    //TODO not divisible by 0
    if(!a)
        return 0;
    return (b / a) & globalmask;
}

long long and(long long a, long long b) {

    return (a & b) & globalmask;
}

long long or(long long a, long long b) {

    return (a | b) & globalmask;
}

long long nor(long long a, long long b) {

    return (~or(a,b)) & globalmask;
}

long long xor(long long a, long long b) {

    return (a ^ b) & globalmask;
}
long long sl(long long a, long long b) {

    return (b << a) & globalmask;
}

long long sr(long long a, long long b) {

    return ( (b >> a) & ~((long long) -1 << (64-a)) ) & globalmask;
}

long long rl(long long a, long long b) {

    return ( b << a | sr(globalmasksize-a, b) ) & globalmask;
}

long long rr(long long a, long long b) {

    return ( sr(a, b) | ( b << (globalmasksize- a) ) ) & globalmask;
}

long long modulus(long long a, long long b) {

    //TODO not divisible by 0
    if(!a)
        return 0;

    return (b % a) & globalmask;
}

long long not(long long a, long long UNUSED(b)) {

    return ~a & globalmask;
}

long long twos_complement(long long a, long long UNUSED(b)) {

    return (~a + 1) & globalmask;
}
