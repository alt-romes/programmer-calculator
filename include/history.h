#ifndef _HISTORY_H
#define _HISTORY_H

struct history {
    int size;
    char * records[100];
};

void clear_history();
void add_to_history(struct history* h,char* in);
void add_number_to_history(long long n, int type);

#endif
