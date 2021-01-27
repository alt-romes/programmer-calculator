#ifndef _OPERATORS_H
#define _OPERATORS_H

#define DEFAULT_MASK -1
#define DEFAULT_MASK_SIZE 64


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
