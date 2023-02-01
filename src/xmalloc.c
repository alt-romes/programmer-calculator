#include "xmalloc.h"

#include "global.h"


/**
 * behaves the same as malloc but kills the program if malloc fails
 * @param bytes size of allocation
 */
void *xmalloc(size_t bytes) {
    return xmalloc_with_ressources(bytes, NULL, 0);
}

/**
 * behaves the same as malloc but kills the program if malloc fails
 * @param bytes size of allocation
 * @param pntrs is the list of pointers to free
 * @param npntrs is the amount of pointers to free = size of array
 */
void* xmalloc_with_ressources(size_t bytes, void** ressources, size_t nres) {
    void* temp = malloc(bytes);
    if (temp == NULL) {
        xfreen(ressources, nres);
        exit_pcalc(MEM_FAIL);
    }
    return temp;
}

/**
 * behaves the same as calloc but kills the program if malloc fails
 * @param nelem number of elements
 * @param bytes byte that is set
 */
void* xcalloc(size_t nelem, size_t bytes) {
    return xcalloc_with_ressources(nelem, bytes, NULL, 0);
}


/**
 * behaves the same as calloc but kills the program if malloc fails
 * @param nelem number of elements
 * @param bytes byte that is set
 * @param pntrs is the list of pointers to free
 * @param npntrs is the amount of pointers to free = size of array
 */
void* xcalloc_with_ressources(size_t nelem, size_t bytes, void** ressources, size_t nres) {
    void *temp = calloc(nelem, bytes);
    if (temp == NULL) {
        xfreen(ressources, nres);
        exit_pcalc(MEM_FAIL);
    }
    return temp;
}

/**
 * behaves the same as xrealloc but kills the program if malloc fails
 * @param pntr pointer to reallocate
 * @param bytes new size
 */
void* xrealloc(void *pntr, size_t bytes) {
    return xrealloc_with_ressources(pntr, bytes, NULL, 0);
}


/**
 * behaves the same as realloc but kills the program if malloc fails
 * @param pntr pointer to reallocate
 * @param bytes new size
 * @param pntrs is the list of pointers to free
 * @param npntrs is the amount of pointers to free = size of array
 */
void* xrealloc_with_ressources(void* pntr, size_t bytes, void** ressources, size_t nres) {
    void* temp = pntr ? realloc(pntr, bytes) : xmalloc(bytes);
    if (temp == NULL) {
        xfreen(ressources, nres);
        exit_pcalc(MEM_FAIL);
    }
    return (temp);
}

/**
 * Frees npntrs elements
 * @param pntrs is the list of pointers to free
 * @param npntrs is the amount of pointers to free = size of array
 */
void xfreen(void** pntrs, size_t npntrs) {
    for (size_t i = 0; i < npntrs; ++i) {
        xfree(pntrs[i]);
    }
}

/**
 * behaves the same as free
 * @param pntr the pointer to be freed
 */
void xfree(void* pntr) {
    free(pntr);
}
