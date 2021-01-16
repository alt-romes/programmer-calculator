typedef struct stack {
    int max_size;
    int size;
    long long * elements;
} stack;

stack * create_stack(int max_size) {

    stack* s;
    s = malloc(sizeof (stack));
    s->elements = malloc((sizeof(long long)));
    s->size = 0;
    s->max_size = max_size;
    return s;
}

stack * resize_stack(stack* s) {

    s->max_size *= 2;
    return realloc(s, s->max_size);
}

long long * pop(stack* s) {

    if (s->size == 0)
        return NULL;

    return &s->elements[--s->size];
}

void push(stack* s, long long value) {

   if (s->size == s->max_size) 
       resize_stack(s);

   s->elements[s->size++] = value;
}
