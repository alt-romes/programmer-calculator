#ifndef _XMALLOC_H
#define _XMALLOC_H

#include <stdlib.h>

#include "global.h"

void* xmalloc(size_t bytes);
void* xmalloc_with_ressources(size_t bytes, void** ressources, size_t nres);
void* xcalloc(size_t nelem, size_t bytes);
void* xcalloc_with_ressources(size_t nelem, size_t bytes, void** ressources, size_t nres);
void* xrealloc(void* pntr, size_t bytes);
void* xrealloc_with_ressources(void* pntr, size_t bytes, void** ressources, size_t nres);
void xfreen(void** pntrs, size_t npntrs);
void xfree(void* pntr);

#endif
