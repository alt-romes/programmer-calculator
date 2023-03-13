#ifndef _HISTORY_H
#define _HISTORY_H

#include <stdint.h>

#define HISTORY_RECORDS_BEFORE_REALLOC 20

#define NTYPE_DEC 0
#define NTYPE_HEX 1
#define NTYPE_BIN 2

struct history {
    int size;
    char **records;
};

extern struct history searchHistory;
extern struct history history;

void clear_history();
void add_to_history(struct history* h,char* in);
void add_number_to_history(uint64_t n, int type);
void browsehistory(char*, int, int*);
void free_history(struct history *h);

#endif
