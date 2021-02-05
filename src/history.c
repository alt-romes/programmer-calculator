#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"
#include "global.h"
#include "history.h"
#include "xmalloc.h"

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

    if (h->size % HISTORY_RECORDS_BEFORE_REALLOC == 0)
        h->records = xrealloc(h->records, (h->size + HISTORY_RECORDS_BEFORE_REALLOC) * sizeof(char *));

    if ((h->records[h->size++] = strdup(*in == '\0' && h == &history ? "0" : in)) == NULL)
        exit_pcalc(MEM_FAIL);

}

void add_number_to_history(long long n, int type) {

    char *str = str_with_base_of_number(n, type);
    add_to_history(&history, str);
    xfree(str);
}

void browsehistory(char* in , int mode, int* counter) {

    /* @mode is -1 when scrolling up
     * @mode is 1 when scrolling down
     * this is due to the fact that when scrolling up we're browsing the history backwards,
     * starting at the most recent command, until we hit the oldest command added to history
     */

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
        xfree(h->records[i]);

    xfree(h->records);

}


char *str_with_base_of_number(long long n, int type) {

    char *str = xmalloc(67);

    if (type == 0)
        sprintf(str,"%lld", n);
    else if (type == 1)
        sprintf(str,"0x%llX", n);
    else if (type == 2) {

        unsigned long long mask = ror(1, 1);

        int i = 0;
        for (; i<64; i++, mask>>=1)
            if (mask & n)
                break;

        int nbits = globalmasksize - i;

        sprintf(str, "0b");
        if (nbits == 0)
            str[2] = '0';
        else
            for (i=0; i<nbits; i++, mask>>=1)
                str[i+2] = mask & n ? '1' : '0';

        str[i+2] = '\0';
    }

    return str;

}
