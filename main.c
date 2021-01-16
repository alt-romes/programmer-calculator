#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ncurses.h>

#include "stack.h"

#define MAX_IN 80

// Operations Control
// Example: '+' takes two operands, therefore the noperands = 2
typedef struct operation {
    char character;
    unsigned char noperands;
    long long (*execute) (long long, long long);
} operation;

struct history {
    int size;
    char * records[500];
};

struct history history;

// ncurses
WINDOW* displaywin, * inputwin;
void init_gui();
void draw(numberstack*, operation*);
void printbinary(long long);
void printhistory();

// General
operation* getopcode(char);
void process_input(numberstack*, operation**, char*);
void clear_history();
void add_to_history();

// Operations
long long add(long long, long long);
long long subtract(long long, long long);
long long multiply(long long, long long);

operation operations[4] = {
    {0, 0, NULL},
    {'+', 2, add},
    {'-', 2, subtract},
    {'*', 2, multiply}
};

int main(int argc, char *argv[])
{
    
    init_gui(&displaywin, &inputwin);

    numberstack* numbers = create_numberstack(4);
    operation* current_op = &operations[0];

    // Start with 0
    push_numberstack(numbers, 0);
    add_to_history("0");
    draw(numbers, current_op);

    for (;;) {

        // Get input
        char in[MAX_IN+1];
        wgetnstr(inputwin, in, MAX_IN);

        process_input(numbers, &current_op, in);

        // Display number on top of the stack
        draw(numbers, current_op);
    }

    endwin();

    return 0;
}

void process_input(numberstack* numbers, operation** current_op, char* in) {

    // Process input
    switch (in[0]) {

        case '+':
        case '-':
            if (in[1] != '\0') // differ subtraction from minus sign
                goto default_push_label;
        case '*':
            *current_op = getopcode(in[0]);
            break;

        case '0':

            if (in[1] == 'x')
                push_numberstack(numbers, strtoll(in+2, NULL, 16));

            else if (in[1] == 'b')
                push_numberstack(numbers, strtoll(in+2, NULL, 2));

            else
                goto default_push_label;

            break;

        case '\0':
            // Pressed empty enter aka clear

            // clear history
            clear_history();
        default:
            default_push_label:

            if (*current_op == operations) { // If is the invalid operation (first in array of operations)

                clear_numberstack(numbers);
                clear_history();
            }
            push_numberstack(numbers, atoll(in));
    }

    // Add to history
    add_to_history(in);

    // Apply operations
    if (*current_op != operations) {

        unsigned char noperands = (*current_op)->noperands;

        if (numbers->size >= noperands) {

            long long operands[2] = {0};

            for (unsigned char i=0; i < noperands; i++)
                operands[i] = *pop_numberstack(numbers);

            long long result = (*current_op)->execute(operands[0], operands[1]);

            push_numberstack(numbers, result);

            *current_op = &operations[0]; // Set to invalid operation
        }
    }
}

operation* getopcode(char c)  {

    operation* r;
    switch (c) {

        case '+':
            r = &operations[1];
            break;
        case '-':
            r = &operations[2];
            break;
        case '*':
            r = &operations[3];
            break;
    } 

    return r;
}

void clear_history() {

    for (; history.size>0; history.size--)
        free(history.records[history.size-1]);

    wmove(displaywin, 14, 11);
    wclrtoeol(displaywin);
}

void add_to_history(char* in) {

    history.records[history.size] = malloc(MAX_IN);
    strcpy(history.records[history.size++], *in == '\0' ? "0" : in);
}

void init_gui() {

    initscr();
    cbreak(); // exit on ctrl+c

    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);

    displaywin = newwin(ymax/2, xmax, 0, 0);
    refresh();

    box(displaywin, ' ', 0);
    wrefresh(displaywin);

    inputwin = newwin(3, xmax, ymax/2, 0);
    refresh();

    box(inputwin, ' ', 0);
    wrefresh(inputwin);
}

void printbinary(long long value) {

    unsigned long long mask = 0x8000000000000000;

    mvwprintw(displaywin, 8, 2, "Binary:    \n         64  ");

    for (int i=0; i<64; i++, mask>>=1) {

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

void printhistory() {

    mvwprintw(displaywin, 14, 2, "History:   ");

    for (int i=0; i<history.size; i++) {
        
        wprintw(displaywin, "%s ", history.records[i]);
    }
}

void draw(numberstack* numbers, operation* current_op) {

    long long* np = top_numberstack(numbers);
    long long n;
    if (np == NULL) n = 0;
    else n = *np;

    // Clear lines
    wmove(displaywin, 2, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 4, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 6, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 8, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 9, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 10, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 11, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 12, 11);
    wclrtoeol(displaywin);

    // Write values
    mvwprintw(displaywin, 2, 2, "Operation: %c\n", current_op->character ? current_op->character : ' ');
    mvwprintw(displaywin, 4, 2, "Decimal:   %d", n);
    mvwprintw(displaywin, 6, 2, "Hex:       0x%X", n);
    printbinary(n);
    printhistory();
    wrefresh(displaywin);

    // Clear input
    wmove(inputwin, 1, 19);
    wclrtoeol(inputwin);

    // Prompt input
    mvwprintw(inputwin, 1, 2, "Number or operator: ");
    wrefresh(inputwin);
}



// Operations

long long add(long long a, long long b) {

    return a + b;
}

// remember op1 = first popped ( right operand ), op2 = second popped ( left operand )
long long subtract(long long a, long long b) {
    
    return b - a;
}
long long multiply(long long a, long long b) {

    return a * b;
}

