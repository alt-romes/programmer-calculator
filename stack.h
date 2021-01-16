
// Numbers Stack 

typedef struct numberstack {
    int max_size;
    int size;
    long long * elements;
} numberstack;

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

long long * pop_numberstack(numberstack* s) {

    if (s->size == 0)
        return NULL;

    return &s->elements[--s->size];
}

long long * top_numberstack(numberstack* s) {

    if (s->size == 0)
        return NULL;

    return &s->elements[s->size-1];
}

void push_numberstack(numberstack* s, long long value) {

    if (s->size == s->max_size) 
        resize_numberstack(s);

    s->elements[s->size++] = value;
}

void clear_numberstack(numberstack* s) {

    s->size = 0;
}
