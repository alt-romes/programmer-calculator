#ifndef _XMALLOC_H
#define _XMALLOC_H

#include <stdlib.h>

void *xmalloc(size_t bytes);
void *xcalloc(size_t nelem, size_t bytes);
void *xrealloc(void *pntr, size_t bytes);
void xfree(void *pntr);

#endif
