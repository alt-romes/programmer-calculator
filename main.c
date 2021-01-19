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
void printbinary(long long,int);
void printhistory(numberstack*,int);
void sweepline(int,int);
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

int operation_enabled = 1;
int decimal_enabled = 1;
int hex_enabled = 1;
int symbols_enabled = 1;
int binary_enabled = 1;
int history_enabled = 1;

const char *  all_ops = "+-*/&|$^<>()%~'";
const unsigned long long DEFAULT_MASK = -1;
const int DEFAULT_MASK_SIZE = 64;
unsigned long long globalmask = DEFAULT_MASK;
int globalmasksize = DEFAULT_MASK_SIZE;

operation operations[16] = {
    {0, 0, NULL},
    {'+', 2, add},
    {'-', 2, subtract},
    {'*', 2, multiply},
    {'/', 2, divide},
    {'&', 2, and},
    {'|', 2, or},
    {'$', 2, nor},
    {'^', 2, xor},
    {'<', 2, sl},
    {'>', 2, sr},
    {'(', 2, rl},
    {')', 2, rr},
    {'%', 2, modulus},
    {'~', 1, not},
    {'\'', 1, twos_complement}
};







/*---- Main Logic -------------------------------------------------*/


int main(int argc, char *argv[])
{
    if (argc >= 2 && argv[1][0] == '-') {

        for (int i=1; argv[1][i] != '\0'; i++) {
            
            switch (argv[1][i]) {

                case 'h':
                    history_enabled = 0;
                    break;
                
                case 'b':
                    binary_enabled = 0;
                    break;

                case 'x':
                    hex_enabled = 0;
                    break;

                case 'd':
                    decimal_enabled = 0;
                    break;

                case 'o':
                    operation_enabled = 0;
                    break;

                case 's':
                    symbols_enabled = 0;
                    break;

            }

        }    
    }

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
        char * token = strtok(in, opchar);
        if(token != NULL && token < op) {
            clear_numberstack(numbers);
            clear_history();
        }
        while (token != NULL) {
        	pushnumber(token, numbers);
        	token = strtok(NULL, opchar);
        }
    }
    else if (!strcmp(in, "binary")) {
        binary_enabled = !binary_enabled;
        //help();
    } else if (!strcmp(in, "hex")) {
        hex_enabled = !hex_enabled;
    } else if (!strcmp(in, "decimal")) {
        decimal_enabled = !decimal_enabled;
    } else if (!strcmp(in, "history")) {
        history_enabled = !history_enabled;
    } else if (!strcmp(in, "operation")) {
        operation_enabled = !operation_enabled;
    }
    //TODO: isto não pode estar aqui, pq se não não dá para se escrever 0b011
    else if (strstr(in, "0b") == NULL && strrchr(in, 'b') != NULL) {

        int requestedmasksize = atoi(in);
        globalmasksize = requestedmasksize > DEFAULT_MASK_SIZE || requestedmasksize <= 0 ? DEFAULT_MASK_SIZE : requestedmasksize;

        //globalmask cant be 0x16f's
    	globalmask = DEFAULT_MASK >> (DEFAULT_MASK_SIZE-globalmasksize);

        // apply mask to all numbers in stack
        numberstack* aux = create_numberstack(numbers->size);
        for (int i=0; i<numbers->size; i++)
            push_numberstack(aux, *pop_numberstack(numbers) & globalmask);

        for (int i=0; i<aux->size; i++)
            push_numberstack(numbers, *pop_numberstack(aux) & globalmask);

    }
    else {
        // If is the invalid operation (first in array of operations)
        if (*current_op == operations || (in[0] == '\0' && (*current_op = operations)) ) {

            clear_numberstack(numbers);
            clear_history();
        }

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
        case '$':
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
        case '\'':
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
            char str[22];
            sprintf(str,"%lld",aux);
            add_to_history(str);
        }
        wprintw(displaywin, "%s ", history.records[i]);
    }
    if(priority) {
    wmove(displaywin, 14, 2);
    wclrtoeol(displaywin);
    }
}

void draw(numberstack* numbers, operation* current_op) {

    long long* np = top_numberstack(numbers);
    long long n;
    int prio = 0;
    if (np == NULL) n = 0;
    else n = *np;

    // Clear lines
    for(int i = 2 ; i < 16 ; i++) {
        wmove(displaywin,i,0);
        wclrtoeol(displaywin);
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
    wmove(inputwin, 1, 19);
    wclrtoeol(inputwin);

    // Prompt input
    mvwprintw(inputwin, 1, 2, "Number or operator: ");
    wrefresh(inputwin);
}

void sweepline(int priority,int prompt) {
    wmove(displaywin,priority,prompt);
    wclrtoeol(displaywin);
}

void pushnumber(char * in, numberstack* numbers) {

    char* hbstr;
    if ((hbstr = strstr(in, "0x")) != NULL)
        push_numberstack(numbers, strtoll(hbstr+2, NULL, 16) & globalmask);
    else if ((hbstr = strstr(in, "0b")) != NULL)
        push_numberstack(numbers, strtoll(hbstr+2, NULL, 2) & globalmask);
    else
        push_numberstack(numbers, atoll(in) & globalmask);
}





/*---- Operations -------------------------------------------------*/


long long add(long long a, long long b) {

    return (a + b) & globalmask;
}

// remember op1 = first popped ( right operand ), op2 = second popped ( left operand )
long long subtract(long long a, long long b) {

    return (b - a) & globalmask;
}
long long multiply(long long a, long long b) {

    return (a * b) & globalmask;
}

long long divide(long long a, long long b) {

    //TODO not divisible by 0
    if(!a)
        return 0;
    return (b / a) & globalmask;
}

long long and(long long a, long long b) {

    return (a & b) & globalmask;
}

long long or(long long a, long long b) {

    return (a | b) & globalmask;
}

long long nor(long long a, long long b) {

    return (~or(a,b)) & globalmask;
}

long long xor(long long a, long long b) {

    return (a ^ b) & globalmask;
}
long long sl(long long a, long long b) {

    return (b << a) & globalmask;
}

long long sr(long long a, long long b) {
    return ( (b >> a) & ~((long long) -1 << (64-a)) ) & globalmask;
}

long long rl(long long a, long long b) {

    return ( b << a | sr(globalmasksize-a, b) ) & globalmask;
}

long long rr(long long a, long long b) {

    return ( sr(a, b) | ( b << (globalmasksize- a) ) ) & globalmask;
}

long long modulus(long long a, long long b) {

    return (b % a) & globalmask;
}

long long not(long long a, long long b) {

    return ~a & globalmask;
}

long long twos_complement(long long a, long long b) {

    return (~a + 1) & globalmask;
}
