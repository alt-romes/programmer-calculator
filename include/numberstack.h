#ifndef _NUMBERSTACK_H
#define _NUMBERSTACK_H

typedef struct numberstack {
    int max_size;
    int size;
    long long * elements;
} numberstack;

numberstack * create_numberstack(int max_size);
numberstack * resize_numberstack(numberstack* s);
long long * pop_numberstack(numberstack* s);
long long * top_numberstack(numberstack* s);
void push_numberstack(numberstack* s, long long value);
void clear_numberstack(numberstack* s);
void free_numberstack(numberstack* s);

#endif
