#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ncurses.h>

#include "stack.h"

#define MAX_IN 80





/*---- Structures -------------------------------------------------*/


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





/*---- Function Prototypes ----------------------------------------*/


// Drawing
WINDOW* displaywin, * inputwin;
void init_gui();
void draw(numberstack*, operation*);
void printbinary(long long);
void printhistory(numberstack*);

// General
operation* getopcode(char);
void process_input(numberstack*, operation**, char*);
void clear_history();
void add_to_history();
void pushnumber(char *, numberstack*);

// Operations
long long add(long long, long long);
long long subtract(long long, long long);
long long multiply(long long, long long);
long long divide(long long, long long);
long long and(long long, long long);
long long or(long long, long long);
long long nor(long long, long long);
long long xor(long long, long long);
long long sl(long long, long long);
long long sr(long long, long long);
long long rl(long long, long long);
long long rr(long long, long long);
long long modulus(long long, long long);
long long not(long long, long long);
long long twos_complement(long long, long long);





/*---- Define Operations and Global Vars --------------------------*/


// Variables
int wMaxX;
int wMaxY;

int binary_enabled = 1;

const char *  all_ops = "+-*/&|n^<>()%~t";

operation operations[16] = {
    {0, 0, NULL},
    {'+', 2, add},
    {'-', 2, subtract},
    {'*', 2, multiply},
    {'/', 2, divide},
    {'&', 2, and},
    {'|', 2, or},
    {'n', 2, nor},
    {'^', 2, xor},
    {'<', 2, sl},
    {'>', 2, sr},
    {'(', 2, rl},
    {')', 2, rr},
    {'%', 2, modulus},
    {'~', 1, not},
    {'t', 1, twos_complement}
};







/*---- Main Logic -------------------------------------------------*/


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
    // -> operand (command) operand
    // non functioned commands (1ºhelp,clean,2ºhistory)
    // -> (command) operand [-10 != history - 10]
    // Process input

    char * op = strpbrk(in, all_ops);
    if (op != NULL) {
        char opchar[2];
        opchar[0] = *op;
        opchar[1] = '\0';
        *current_op = getopcode(*op);
        char *  token = strtok(in, opchar);
        while (token != NULL) {
        	pushnumber(token, numbers);
        	token = strtok(NULL, opchar);
        }
    }
    else if (!strcmp(in, "binary")) {
        binary_enabled = !binary_enabled;
        //help();
    }
    else {
        if (*current_op == operations) { // If is the invalid operation (first in array of operations)

            clear_numberstack(numbers);
            clear_history();
        }
        if (in[0] == '\0')
            clear_history();
        else
            pushnumber(in, numbers);
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
        case '/':
            r = &operations[4];
            break;
        case '&':
            r = &operations[5];
            break;
        case '|':
            r = &operations[6];
            break;
        case 'n':
            r = &operations[7];
            break;
        case '^':
            r = &operations[8];
            break;
        case '<':
            r = &operations[9];
            break;
        case '>':
            r = &operations[10];
            break;
        case '(':
            r = &operations[11];
            break;
        case ')':
            r = &operations[12];
            break;
        case '%':
            r = &operations[13];
            break;
        case '~':
            r = &operations[14];
            break;
        case 't':
            r = &operations[15];
            break;

    }

    return r;
}

void clear_history() {

    for (; history.size>0; history.size--)
        free(history.records[history.size-1]);

    wmove(displaywin, 14, 11);
    wclrtoeol(displaywin);
    wmove(displaywin, 15, 0);
    wclrtoeol(displaywin);
}

void add_to_history(char* in) {

    history.records[history.size] = malloc(MAX_IN);
    strcpy(history.records[history.size++], *in == '\0' ? "0" : in);
}







/*---- Graphics Logic ---------------------------------------------*/


void init_gui() {

    initscr();
    cbreak(); // exit on ctrl+c

    getmaxyx(stdscr, wMaxY, wMaxX);

    displaywin = newwin(wMaxY-3, wMaxX, 0, 0);
    refresh();

    box(displaywin, ' ', 0);
    mvwprintw(displaywin, wMaxY-7, 2, "ADD  +    SUB  -    MUL  *    DIV  /\n  AND  &    OR   |    NOR  n    XOR  x\n  SL   <    SR   >    RL   ?    RR   ?");
    wrefresh(displaywin);

    inputwin = newwin(3, wMaxX, wMaxY-3, 0);
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

void printhistory(numberstack* numbers) {
    int currY,currX;
    mvwprintw(displaywin, 14, 2, "History:   ");
    for (int i=0; i<history.size; i++) {
        getyx(displaywin,currY,currX);
        if(currX >= wMaxX-3 || currY > 14) {
            clear_history();
            long long aux = *top_numberstack(numbers);
            char str[21];
            sprintf(str,"%lld",aux);
            add_to_history(str);
        }
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
    if (binary_enabled)
        printbinary(n);
    printhistory(numbers);
    wrefresh(displaywin);

    // Clear input
    wmove(inputwin, 1, 19);
    wclrtoeol(inputwin);

    // Prompt input
    mvwprintw(inputwin, 1, 2, "Number or operator: ");
    wrefresh(inputwin);
}

void pushnumber(char * in, numberstack* numbers) {
    if (in[0] == '0') {
        if (in[1] == 'x')
            push_numberstack(numbers, strtoll(in+2, NULL, 16));

        else if (in[1] == 'b')
            push_numberstack(numbers, strtoll(in+2, NULL, 2));
    }
    else
        push_numberstack(numbers, atoll(in));
}





/*---- Operations -------------------------------------------------*/


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

long long divide(long long a, long long b) {
    //TODO not divisible by 0
    if(!a)
        return 0;
    return b / a;
}

long long and(long long a, long long b) {

    return a & b;
}

long long or(long long a, long long b) {

    return a | b;
}

long long nor(long long a, long long b) {

    return ~or(a,b);
}

long long xor(long long a, long long b) {

    return a ^ b;
}
long long sl(long long a, long long b) {

    return b << a;
}

long long sr(long long a, long long b) {
    return b >> a;
}

long long rl(long long a, long long b) {
    long long res = b;
    for(int i = 0; i<a;i++){
        long long left_most_bit = 0x8000000000000000 & res;
        res = res<<1;
        if(left_most_bit)
            res = res | 0x1;
        else
            res = res & 0xFFFFFFFFFFFFFFFE;
    }
    return res;
    // return ( b << a | ( (~(-1 >> a) & b ) >> 64 - a ));
}

long long rr(long long a, long long b) {
    long long res = b;
    for(int i = 0; i<a;i++){
        long long right_most_bit = 0x1 & res;
        res = res>>1;
        if(right_most_bit)
            res = res | 0x8000000000000000;
        else
            res = res & 0x7FFFFFFFFFFFFFFF;
    }
    return res;
    // return ( b >> a | ( (~(-1 << a) & b ) << 64 - a ));
}

long long modulus(long long a, long long b) {

    return b % a;
}

long long not(long long a, long long b) {

    return ~a;
}

long long twos_complement(long long a, long long b) {

    return ~a + 1;
}
