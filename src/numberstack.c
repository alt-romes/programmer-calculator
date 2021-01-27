
#include <stdlib.h>

// Numbers Stack
#include "numberstack.h"

// Allocate and set up numberstack
numberstack * create_numberstack(int max_size) {

    numberstack* s;
    s = malloc(sizeof (numberstack));
    s->elements = malloc(max_size * sizeof(long long));
    s->size = 0;
    s->max_size = max_size;
    return s;
}

numberstack * resize_numberstack(numberstack* s) {

    s->max_size *= 2;
    return realloc(s->elements, s->max_size * sizeof(long long));
}

// Pop element from the top of the stack (return and remove element)
long long * pop_numberstack(numberstack* s) {

    if (s->size == 0)
        return NULL;

    return &s->elements[--s->size];
}

// Return the element at the top of the stack without removing it
long long * top_numberstack(numberstack* s) {

    if (s->size == 0)
        return NULL;

    return &s->elements[s->size-1];
}

// Push number to the top of the stack
void push_numberstack(numberstack* s, long long value) {

    if (s->size == s->max_size) 
        resize_numberstack(s);

    s->elements[s->size++] = value;
}

// Clear the stack
void clear_numberstack(numberstack* s) {

    s->size = 0;
}
