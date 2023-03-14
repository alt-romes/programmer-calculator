#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <stdint.h>

#define MAX_IN 80
#define INPUT_START 24

#define MEM_FAIL -1

void exit_pcalc(int);

char *str_with_base_of_number(uint64_t, int type);

#endif
