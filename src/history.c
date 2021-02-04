#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"
#include "global.h"
#include "history.h"

struct history searchHistory;
struct history history;

void clear_history() {

    for (; history.size>0; history.size--)
        free(history.records[history.size-1]);

    free(history.records);
    // To make sure realloc behaves like malloc later
    history.records = NULL;

    sweepline(displaywin, 14, 11);
    sweepline(displaywin, 15, 0);
}

void add_to_history(struct history* h, char* in) {

    if (h->size % HISTORY_RECORDS_BEFORE_REALLOC == 0) {
        // Void pointer for temporarily storing h->record's realloc
        void *hrealloc;

        if ((hrealloc = realloc(h->records, (h->size + 1) * sizeof(char *) * HISTORY_RECORDS_BEFORE_REALLOC))) {
            // Contine if realloc succeded
            h->records = hrealloc;
        }
        else {
            // Exit
            exit_pcalc(0xb0);
        }
    }

    if ((h->records[h->size++] = strdup(*in == '\0' && h == &history ? "0" : in)) == NULL) {
        // strdup failed with allocating memory

        exit_pcalc(0xb1);

    }

}

void add_number_to_history(long long n, int type) {

    char str[67];

    if (type == NTYPE_DEC)
        sprintf(str,"%lld", n);
    else if (type == NTYPE_HEX)
        sprintf(str,"0x%llX", n);
    else if (type == NTYPE_BIN) {

        unsigned long long mask = ror(1, 1);

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

void browsehistory(char* in , int mode, int* counter) {

    if( (mode == 1 && *counter < searchHistory.size-1) || (mode == -1 && *counter > 0)) {

        *counter += mode;
        strcpy(in, searchHistory.records[*counter]);
    }
    else if (mode == 1 && *counter == searchHistory.size - 1) {

        // When the user is scrolling down and the limit is reached, the input becomes empty again, and the counter is set to the end

        *counter += 1; /* Set the counter == searchHistory.size.
                        * this is a non existent position, indicating that the counter
                        * is currently not being used.
                        *
                        * You can also think about it in this way:
                        * Next time the user presses key up, the counter == searchHistory.size
                        * will be decremented, and the last position of history will be accessed
                        * history[searchHistory.size - 1]
                        */
        strcpy(in, "");
    }

}

void free_history(struct history *h) {

    for (int i = 0; i < h->size; ++i)
        free(h->records[i]);

    free(h->records);

}
