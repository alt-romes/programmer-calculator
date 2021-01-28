#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"
#include "global.h"
#include "history.h"

extern unsigned long long globalmask;
extern int globalmasksize;

extern WINDOW *displaywin;

struct history searchHistory;
struct history history;

void clear_history() {

    for (; history.size>0; history.size--)
        free(history.records[history.size-1]);

    // To make sure realloc behaves like malloc later
    history.records = NULL;

    sweepline(displaywin, 14, 11);
    sweepline(displaywin, 15, 0);
}

void add_to_history(struct history* h, char* in) {

    // Void pointer for temporarily storing h->record's realloc
    void *hrealloc;

    if ((hrealloc = realloc(h->records, (h->size + 1) * sizeof(char *)))) {
        // Contine if realloc succeded
        h->records = hrealloc;
    }
    else {
        // Exit
        endwin();
        fprintf(stderr, "OUT OF MEMORY");
        exit(-1);
    }

    if ((h->records[h->size++] = strdup(*in == '\0' && h == &history ? "0" : in)) == NULL) {
        // strdup failed with allocating memory
        endwin();
        fprintf(stderr, "OUT OF MEMORY");
        exit(-1);

    }

}

void add_number_to_history(long long n, int type) {

    char str[67];

    if (type == 0)
        sprintf(str,"%lld", n);
    else if (type == 1)
        sprintf(str,"0x%llX", n);
    else if (type == 2) {

        unsigned long long mask = rr(1, 1);

        int i = 0;
        for (; i<64; i++, mask>>=1)
            if (mask & n)
                break;

        int nbits = globalmasksize - i;

        sprintf(str, "0b");
        if (nbits == 0)
            sprintf(str+2, "0");
        else
            for (i=0; i<nbits; i++, mask>>=1)
                sprintf(str+i+2, "%c", mask & n ? '1' : '0');

    }

    add_to_history(&history, str);
    wrefresh(displaywin);
}
