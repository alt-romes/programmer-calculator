#include <stddef.h>

#include "operators.h"

unsigned long long globalmask = DEFAULT_MASK;
int globalmasksize = DEFAULT_MASK_SIZE;

operation *current_op = NULL;

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
static long long swap_endianness(long long, long long);

static operation operations[16] = {
    {ADD_SYMBOL, 2, add},
    {SUB_SYMBOL, 2, subtract},
    {MUL_SYMBOL, 2, multiply},
    {DIV_SYMBOL, 2, divide},
    {AND_SYMBOL, 2, and},
    {OR_SYMBOL, 2, or},
    {NOR_SYMBOL, 2, nor},
    {XOR_SYMBOL, 2, xor},
    {SHL_SYMBOL, 2, shl},
    {SHR_SYMBOL, 2, shr},
    {ROL_SYMBOL, 2, rol},
    {ROR_SYMBOL, 2, ror},
    {MOD_SYMBOL, 2, modulus},
    {NOT_SYMBOL, 1, not},
    {TWOSCOMPLEMENT_SYMBOL, 1, twos_complement},
    {SWAPENDIANNESS_SYMBOL, 1, swap_endianness}
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

    // Shift longer than type length is undefined behaviour
    return b << a;
}

long long shr(long long a, long long b) {

    // Shift longer than 64 bits is undefined behaviour
    // don't include shift in tests or //TODO: define behaviour for this calculator
    return ((unsigned long long) b >> a);
}

static long long rol(long long a, long long b) {

    // prevent shift by 64 bits because a shift longer than type length is undefined behaviour
    return b << a | ( globalmasksize - a < 64 ? shr(globalmasksize - a, b) : 0 );
}

long long ror(long long a, long long b) {

    // prevent shift by 64 bits because a shift longer than type length is undefined behaviour
    return shr(a, b) | (globalmasksize - a < 64 ? b << (globalmasksize - a) : 0);
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

static long long swap_endianness(long long a, long long UNUSED(b)) {

    long long out = 0;
    // shift the leftmost bits to the right
    for (int i = 0; i < globalmasksize / 16; i++) {
        //        create a bitmask and apply it to a        and shift the selected byte to its new position
        out |= (a & (0xffull << (globalmasksize-8 - i*8)) ) >> (globalmasksize-8 - i*16);
    }
    // shift the rightmost bits left (and leave the miidle bit in place in the case of an odd number of bytes)
    for (int i = 0; i < globalmasksize / 16 + 1; i++) {
        out |= (a & (0xffull << (((globalmasksize/2 - 1) & -8) - i*8)) ) << (((globalmasksize/8 & 1) ? 0 : 8) + i*16);
    }
    return out;
}
