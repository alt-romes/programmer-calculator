#include <stdlib.h>
#include <limits.h>
#include <ncurses.h>

#include "stack.h"

#define MAX_IN 80

// ncurses
WINDOW* displaywin, * inputwin;
void init_gui();
void draw();

// General
unsigned char getopcode(char);
void process_input(numberstack*, unsigned char*, char*);

// Operations
long long add(long long, long long);
long long subtract(long long, long long);
long long multiply(long long, long long);

// Number of operands per operation (by opcode)
// Example: the opcode for '+' is 1, '+' takes two operands, therefore the array entry[1] = 2
unsigned char opcodes_noperands[4] = {
    0, 2, 2, 2
};

long long (*execute_ops[4]) (long long, long long) = {
    NULL, &add, &subtract, &multiply
};

int main(int argc, char *argv[])
{
    
    init_gui(&displaywin, &inputwin);

    numberstack* numbers = create_numberstack(4);
    unsigned char current_op = 0;

    // Start with 0
    push_numberstack(numbers, 0);
    draw(numbers);

    for (;;) {

        // Get input
        char in[MAX_IN+1];
        wgetnstr(inputwin, in, MAX_IN);

        process_input(numbers, &current_op, in);

        // Display number on top of the stack
        draw(numbers);
    }

    endwin();

    return 0;
}

void process_input(numberstack* numbers, unsigned char * current_op, char* in) {

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

        default:
            default_push_label:

            if (*current_op == 0)
                clear_numberstack(numbers);
            push_numberstack(numbers, atoi(in));
    }

    // Apply operations
    if (*current_op > 0) {

        unsigned char noperands = opcodes_noperands[*current_op];

        if (numbers->size >= noperands) {

            long long operands[2] = {0};

            for (unsigned char i=0; i < noperands; i++)
                operands[i] = *pop_numberstack(numbers);

            long long result = (*execute_ops[*current_op])(operands[0], operands[1]);

            push_numberstack(numbers, result);

            *current_op = 0;
        }
    }
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

void draw(numberstack* numbers) {

    long long* np = top_numberstack(numbers);
    long long n;
    if (np == NULL) n = 0;
    else n = *np;

    mvwprintw(displaywin, 1, 2, "Decimal:   %d\n", n);
    mvwprintw(displaywin, 2, 2, "Hex:       0x%X\n", n);
    wrefresh(displaywin);

    werase(inputwin);
    box(inputwin, ' ', 0); // Is there a way so that I don't have to call box everytime?
    mvwprintw(inputwin, 1, 2, "Number or operator: ");
    wrefresh(inputwin);
}

unsigned char getopcode(char c)  {

    switch (c) {

        case '+':
            c = 1;
            break;
        case '-':
            c = 2;
            break;
        case '*':
            c = 3;
            break;
    } 

    return c;
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

