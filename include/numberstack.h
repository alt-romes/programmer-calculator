#ifndef _NUMBERSTACK_H
#define _NUMBERSTACK_H

#include <stdint.h>

typedef struct numberstack {
    int max_size;
    int size;
    uint64_t * elements;
} numberstack;

extern numberstack* numbers;

numberstack * create_numberstack(int max_size);
uint64_t * pop_numberstack(numberstack* s);
uint64_t * top_numberstack(numberstack* s);
void push_numberstack(numberstack* s, uint64_t value);
void clear_numberstack(numberstack* s);
void free_numberstack(numberstack* s);

#endif
