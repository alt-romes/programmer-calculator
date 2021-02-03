#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"
#include "history.h"
#include "numberstack.h"
#include "operators.h"

extern struct history history;
extern struct history searchHistory;

WINDOW* displaywin, * inputwin;

int wMaxX;
int wMaxY;

int operation_enabled = 1;
int decimal_enabled = 1;
int hex_enabled = 1;
int symbols_enabled = 1;
int binary_enabled = 1;
int history_enabled = 1;

extern unsigned long long globalmask;
extern int globalmasksize;

void init_gui() {

    initscr();
    cbreak(); 

    getmaxyx(stdscr, wMaxY, wMaxX);

    displaywin = newwin(wMaxY-3, wMaxX, 0, 0);
    refresh();

    box(displaywin, ' ', 0);
    if (symbols_enabled)
        mvwprintw(displaywin, wMaxY-7, 2, "ADD  +    SUB  -    MUL  *    DIV  /    MOD  %%\n  AND  &    OR   |    NOR  $    XOR  ^    NOT  ~\n  SL   <    SR   >    RL   (    RR   )    2's  '");
    wrefresh(displaywin);

    inputwin = newwin(3, wMaxX, wMaxY-3, 0);
    refresh();

    box(inputwin, ' ', 0);
    wrefresh(inputwin);

}

void printbinary(long long value, int priority) {

    unsigned long long mask = ((long long) 1) << (globalmasksize - 1); // Mask starts at the last bit to display, and is >> until the end

    int i=DEFAULT_MASK_SIZE-globalmasksize;

    mvwprintw(displaywin, 8-priority, 2, "Binary:    \n         %02d  ", globalmasksize); // %s must be a 2 digit number

    for (; i<64; i++, mask>>=1) {

        unsigned long long bitval = value & mask;
        waddch(displaywin, bitval ? '1':'0');

        if (i%16 == 15 && 64 - ((i/16+1)*16))
            // TODO: Explain these numbers better (and decide if to keep them)
            wprintw(displaywin, "\n         %d  ", 64-((i/16)+1)*16);
        else if (i%8 == 7)
            wprintw(displaywin, "   ");
        else if (i%4 == 3)
            wprintw(displaywin, "  ");
        else
            waddch(displaywin, ' ');

    }
}

void printhistory(numberstack* numbers, int priority) {
    int currY,currX;
    mvwprintw(displaywin, 14-priority, 2, "History:   ");
    for (int i=0; i<history.size; i++) {
        getyx(displaywin,currY,currX);
        if(currX >= wMaxX-3 || currY > 14) {
            clear_history();
            long long aux = *top_numberstack(numbers);
            add_number_to_history(aux, 0);
        }
        wprintw(displaywin, "%s ", history.records[i]);
    }
}
//-------------------------------------------------------------------------------------------------------------------------


void draw(numberstack* numbers, operation* current_op) {

    long long* np = top_numberstack(numbers);
    long long n;
    int prio = 0;
    if (np == NULL) n = 0;
    else n = *np;

    // Clear lines
    for(int i = 2 ; i < 16 ; i++) {
        sweepline(displaywin, i, 0);
    }

    if(!operation_enabled) prio += 2;
    else mvwprintw(displaywin, 2, 2, "Operation: %c\n", current_op->character ? current_op->character : ' ');

    if(!decimal_enabled) prio += 2;
    else mvwprintw(displaywin, 4-prio, 2, "Decimal:   %lld", n);

    if(!hex_enabled) prio += 2;
    else mvwprintw(displaywin, 6-prio, 2, "Hex:       0x%llX", n);

    if(!binary_enabled) prio +=6;
    else printbinary(n,prio);

    if(!history_enabled) prio += 2;
    else printhistory(numbers,prio);

    wrefresh(displaywin);

    // Clear input
    sweepline(inputwin, 1, 19);

    // Prompt input
    mvwprintw(inputwin, 1, 2, "Number or operator: ");
    wrefresh(inputwin);
}

void sweepline(WINDOW* w, int y, int x) {
    wmove(w, y, x);
    wclrtoeol(w);
}

long long pushnumber(char * in, numberstack* numbers) {

    long long n;

    char* hbstr;
    if ((hbstr = strstr(in, "0x")) != NULL)
        n = strtoll(hbstr+2, NULL, 16) & globalmask;
    else if ((hbstr = strstr(in, "0b")) != NULL)
        n = strtoll(hbstr+2, NULL, 2) & globalmask;
    else
        n = atoll(in) & globalmask;

    push_numberstack(numbers, n);
    return n;

}
