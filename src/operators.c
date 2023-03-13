#include <stddef.h>
#include <stdint.h>

#include "operators.h"

uint64_t globalmask = DEFAULT_MASK;
int globalmasksize = DEFAULT_MASK_SIZE;

operation *current_op = NULL;

static uint64_t add(uint64_t, uint64_t);
static uint64_t subtract(uint64_t, uint64_t);
static uint64_t multiply(uint64_t, uint64_t);
static uint64_t divide(uint64_t, uint64_t);
static uint64_t and(uint64_t, uint64_t);
static uint64_t or(uint64_t, uint64_t);
static uint64_t nor(uint64_t, uint64_t);
static uint64_t xor(uint64_t, uint64_t);
static uint64_t shl(uint64_t, uint64_t);
static uint64_t rol(uint64_t, uint64_t);
static uint64_t modulus(uint64_t, uint64_t);
static uint64_t not(uint64_t, uint64_t);
static uint64_t twos_complement(uint64_t, uint64_t);
static uint64_t swap_endianness(uint64_t, uint64_t);

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


static uint64_t add(uint64_t a, uint64_t b) {

    return a + b;
}

// remember op1 = first popped ( right operand ), op2 = second popped ( left operand )
static uint64_t subtract(uint64_t a, uint64_t b) {

    return b - a;
}
static uint64_t multiply(uint64_t a, uint64_t b) {

    return a * b;
}

static uint64_t divide(uint64_t a, uint64_t b) {

    //TODO not divisible by 0
    if(!a)
        return 0;

    return b / a;
}

static uint64_t and(uint64_t a, uint64_t b) {

    return a & b;
}

static uint64_t or(uint64_t a, uint64_t b) {

    return a | b;
}

static uint64_t nor(uint64_t a, uint64_t b) {

    return ~(a | b);
}

static uint64_t xor(uint64_t a, uint64_t b) {

    return a ^ b;
}
static uint64_t shl(uint64_t a, uint64_t b) {

    // Shift longer than type length is undefined behaviour
    return b << a;
}

uint64_t shr(uint64_t a, uint64_t b) {

    // Shift longer than 64 bits is undefined behaviour
    // don't include shift in tests or //TODO: define behaviour for this calculator
    return ((uint64_t) b >> a);
}

static uint64_t rol(uint64_t a, uint64_t b) {

    // prevent shift by 64 bits because a shift longer than type length is undefined behaviour
    return b << a | ( globalmasksize - a < 64 ? shr(globalmasksize - a, b) : 0 );
}

uint64_t ror(uint64_t a, uint64_t b) {

    // prevent shift by 64 bits because a shift longer than type length is undefined behaviour
    return shr(a, b) | (globalmasksize - a < 64 ? b << (globalmasksize - a) : 0);
}

static uint64_t modulus(uint64_t a, uint64_t b) {

    //TODO not divisible by 0
    if(!a)
        return 0;

    return b % a;
}

static uint64_t not(uint64_t a, uint64_t UNUSED(b)) {

    return ~a;
}

static uint64_t twos_complement(uint64_t a, uint64_t UNUSED(b)) {

    return -a;
}

static uint64_t swap_endianness(uint64_t a, uint64_t UNUSED(b)) {

    uint64_t out = 0;
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
