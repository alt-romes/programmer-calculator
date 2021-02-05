#include "xmalloc.h"

#include "global.h"

void *xmalloc(size_t bytes) {

    void *temp = malloc(bytes);
    if (temp == NULL)
        exit_pcalc(MEM_FAIL);
    return (temp);
}

void *xcalloc(size_t nelem, size_t bytes) {

    void *temp = calloc(nelem, bytes);
    if (temp == NULL)
        exit_pcalc(MEM_FAIL);
    return (temp);
}

void *xrealloc(void *pntr, size_t bytes) {

    void *temp = pntr ? realloc(pntr, bytes) : malloc(bytes);
    if (temp == NULL)
        exit_pcalc(MEM_FAIL);
    return (temp);
}

void xfree(void *pntr) {
    free(pntr);
}
