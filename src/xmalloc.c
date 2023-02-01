#include "xmalloc.h"

#include "global.h"

void *xmalloc(size_t bytes) {

    return xmalloc_with_ressources(bytes, NULL, 0);
}

void *xmalloc_with_ressources(size_t bytes, void** ressources, size_t nres) {
    void* temp = malloc(bytes);
    if (temp == NULL) {
        xfreen(ressources, nres);
        exit_pcalc(MEM_FAIL);
    }
    return temp;
}

void *xcalloc(size_t nelem, size_t bytes) {

    return xcalloc_with_ressources(nelem, bytes, NULL, 0);
}

void *xcalloc_with_ressources(size_t nelem, size_t bytes, void** ressources, size_t nres) {
    void *temp = calloc(nelem, bytes);
    if (temp == NULL) {
        xfreen(ressources, nres);
        exit_pcalc(MEM_FAIL);
    }
    return temp;
}

void *xrealloc(void *pntr, size_t bytes) {

    return xrealloc_with_ressources(pntr, bytes, NULL, 0);
}

void *xrealloc_with_ressources(void *pntr, size_t bytes, void** ressources, size_t nres) {

    void *temp = pntr ? realloc(pntr, bytes) : xmalloc(bytes);
    if (temp == NULL) {
        xfreen(ressources, nres);
        exit_pcalc(MEM_FAIL);
    }
    return (temp);
}

void xfreen(void** pntrs, size_t npntrs) {
    for (size_t i = 0; i < npntrs; ++i) {
        xfree(pntrs[i]);
    }
}

void xfree(void *pntr) {
    free(pntr);
}
