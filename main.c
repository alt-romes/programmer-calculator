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
    char * records[100];
};

struct searchHistory {
    int size,counter;
    char * inputs[100];
};

struct searchHistory searchHistory;
struct history history;





/*---- Function Prototypes ----------------------------------------*/


// Drawing
WINDOW* displaywin, * inputwin;
void init_gui();
void draw(numberstack*, operation*);
void printbinary(long long,int);
void printhistory(numberstack*,int);
void browsehistory(char*,int);
void sweepline(int,int);
// General
operation* getopcode(char);
void process_input(numberstack*, operation**, char*);
void clear_history();
void add_to_history();
long long pushnumber(char *, numberstack*);
void add_number_to_history(long long, int); // 0 = decimal, 1 = hex, 2 = binary
void get_input(char *);

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
const char * VALID_NUMBER_INPUT = "0123456789abcdefx";
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

    // Get command line options to hide parts of the display
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

    /* 
     * The numberstack is used to store numbers used in calculations
     * It's a normal stack data structure (LIFO) that holds long long integers
     * Check the stack.h file for its operations
     *
     * The operation structure holds information regarding the ASCII character the operation uses,
     * The number of operands the operation takes, and the function to execute when applied
     *
     * Numbers get pushed to the numberstack "numbers", and operations are set as the "current_op"
     *
     * After receiving user input, if there's a current operation, the program compares the 
     * current stack size to the number of operands needed for that operation
     *
     * Should the operation be executed, the needed operands are popped from the stack,
     * the operation is executed, and the result of the calculation is pushed to the stack
     */
    numberstack* numbers = create_numberstack(4);
    operation* current_op = &operations[0];

    // Start numberstack and history with 0
    push_numberstack(numbers, 0);
    add_to_history("0");
    // Display number on top of the stack (0)
    draw(numbers, current_op);
    add_to_searchhistory("");
    //Main Loop
    for (;;) {

        // Get input
        char in[MAX_IN+1];
        
        // Make sure that if enter is pressed, a len == 0 null terminated string is in "in"
        in[0] = '\0';
        
        searchHistory.counter = 0;
        get_input(in);
        
        process_input(numbers, &current_op, in);

        // Display number on top of the stack
        draw(numbers, current_op);
    }

    endwin();

    return 0;
}

void process_input(numberstack* numbers, operation** current_op, char* in) {

    // Process input

    // There's an operation if one of the operation symbols is found in the input string
    char * op = strpbrk(in, all_ops);

    if (op != NULL) {

        // An operation symbol was found
        
        // A new string holds the operation symbol
        char opchar[2];
        opchar[0] = *op;
        opchar[1] = '\0';

        // Set the current operation as the operation structure for that symbol
        *current_op = getopcode(*op);

        /* There are four valid situations when an operation symbol
         * is found in the input string
         *
         * 1 - just the symbol i.e. "+"
         * 2 - a number then a symbol i.e. "2+"
         * 3 - a symbol then a number i.e. "+2"
         * 4 - a number, a symbol and a number i.e. "2+2"
         *
         * The following code handles the input to generate one of those cases
         * Adding the numbers and operation accordingly to the respective structures
         */

        // Find the first number in the string, by splitting with the symbol found
        char * token = strtok(in, opchar);

        // Bool to avoid duplicate operations in stack
        int operationInStack = 0;

        // If no number is found (case 1)
        if(token == NULL) {
            // Make sure the operation isn't already in history to avoid duplicate symbols
            if(strcmp(opchar, history.records[history.size-1]))
                add_to_history(opchar);
        }
        // When the first number comes before the operation symbol (case 2 and 4)
        else if (token != NULL && token < op) {
            // History and numbers will be overridden by the input number and operation
            // So we clean the stack and the history
            clear_numberstack(numbers);
            clear_history();
        }

        // When the first number comes after the operation symbol (case 3)
        else if (token != NULL && op < token) {

            // Make sure the operation isn't already in history to avoid duplicate symbols
            if(strcmp(opchar, history.records[history.size-1]))
                add_to_history(opchar);

            // There's definitely an operation already in the stack
            // This will be important when we add two different symbols in a row (i.e. +*)
            // Because only the first will be added to history
            operationInStack = 1;
        }

        // Read all the next numbers (split by the operation) until we're done
        while (token != NULL) {

            // We have a number, so we'll push it to the number stack
            long long aux = pushnumber(token, numbers);
            
            // History will display the number in the format inserted
            // So we must separate 0b from 0x from a normal decimal
            // add_number_to_history takes a second parameter to display accordingly
            if(strstr(token, "0b") != NULL)
                add_number_to_history(aux, 2);
            else if (strstr(token, "0x") != NULL)
                add_number_to_history(aux, 1);
            else
                add_number_to_history(aux, 0);

            // After inserting the number in the history, if an operation isn't
            // detected in the history stack yet, add the operation to the history
            if(!operationInStack && strcmp(opchar, history.records[history.size-1])) {
                add_to_history(opchar);
                operationInStack = 1;
            }
            
            // Get the next token (next number)
        	token = strtok(NULL, opchar);
        }


    }

    // Handle other commands when an operation wasn't in the input string
    else if (!strcmp(in, "binary"))
        binary_enabled = !binary_enabled;

    else if (!strcmp(in, "hex"))
        hex_enabled = !hex_enabled;

    else if (!strcmp(in, "decimal"))
        decimal_enabled = !decimal_enabled;

    else if (!strcmp(in, "history"))
        history_enabled = !history_enabled;

    else if (!strcmp(in, "operation"))
        operation_enabled = !operation_enabled;

    else if (strstr(in, "cb") != NULL) {

        // Command to change the number of bits

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

    // If there's no operation, and it's not a known command, handle input as a number
    else {

        if (strpbrk(in, VALID_NUMBER_INPUT) || in[0] == '\0') {

            // If is the invalid operation (first in array of operations)
            if (*current_op == operations || (in[0] == '\0' && (*current_op = operations)) ) {

                clear_numberstack(numbers);
                clear_history();
            }

            long long aux = pushnumber(in, numbers);

            if(strstr(in, "0b") != NULL)
                add_number_to_history(aux, 2);
            else if (strstr(in, "0x") != NULL)
                add_number_to_history(aux, 1);
            else
                add_number_to_history(aux, 0);
        
        }

    }

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

void add_to_searchhistory(char* in) {
    searchHistory.inputs[searchHistory.size] = malloc(MAX_IN);
    strcpy(searchHistory.inputs[searchHistory.size++], in);
}

void get_input(char *in) {
        char inp;
        // Collect input until enter is pressed
        for (int i = 0; (inp = getchar()) != 13;)
        {
            int searched = 0;
            // Handles all arrow keys
            if (inp == 27) {
                getchar();
                inp = getchar();
                if (inp == 'A')
                {   
                    browsehistory(in,1);
                    i = strlen(in);
                    searched = 1;
                }
                else if (inp == 'B')
                {   
                    browsehistory(in,-1);
                    i = strlen(in);
                    searched = 1;
                }
            }

            if (inp == 127)
            {
                //Backspace
                i == 0 ? i = 0 : --i;
                inp = '\0';
            }

            if(!searched) {
                // Prevent user to input more than MAX_IN
                if (i <= MAX_IN) {
                    // Append char to in array
                    in[i++] = inp;
                    in[i] = '\0';
                    if (inp == '\0')
                    {
                        // Clear screen from previous input
                        mvwprintw(inputwin, 1, 22 + --i ," ");
                    }
                }
            }
            // Finaly print input
            wmove(inputwin, 1, 22);
            wclrtoeol(inputwin);
            mvwprintw(inputwin, 1, 22, in);
            wrefresh(inputwin);
        }
    add_to_searchhistory(in);
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
            add_number_to_history(aux, 0);
        }
        wprintw(displaywin, "%s ", history.records[i]);
    }
}

void browsehistory(char* in , int mode) {
    if(searchHistory.counter < searchHistory.size-1 || searchHistory.counter > 0) {
        searchHistory.counter+=mode;
        strcpy(in,searchHistory.inputs[searchHistory.counter]);
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

    add_to_history(str);
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
