#ifndef _HISTORY_H
#define _HISTORY_H


#define HISTORY_RECORDS_BEFORE_REALLOC 20

struct history {
    int size;
    char **records;
};

void clear_history();
void add_to_history(struct history* h,char* in);
void add_number_to_history(long long n, int type);
void browsehistory(char*, int, int*);
void free_history(struct history *h);

#endif
