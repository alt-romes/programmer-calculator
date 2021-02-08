#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ncurses.h>
#include <unistd.h>
#include <signal.h>

#include "global.h"
#include "draw.h"
#include "history.h"
#include "numberstack.h"
#include "operators.h"
#include "parser.h"





#define VERSION "v1.8"





/*---- Function Prototypes ----------------------------------------*/


// General
static void process_input(operation**, char*);
static void get_input(char*);
static void apply_operations(numberstack*, operation**);
static void exit_pcalc_success();





/*---- Main Logic -------------------------------------------------*/


int main(int argc, char* argv[])
{
    // Set all long arguments that can be used
    struct option long_options[] = {

        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"history", no_argument, NULL, 'i'},
        {"binary", no_argument, NULL, 'b'},
        {"hex", no_argument, NULL, 'x'},
        {"decimal", no_argument, NULL, 'd'},
        {"operation", no_argument, NULL, 'o'},
        {"symbol", no_argument, NULL, 's'}

     };

    // Get command line options to hide parts of the display
    int opt;
    while ((opt = getopt_long(argc, argv, "hvibxdos", long_options, NULL)) != -1) {
        switch (opt) {

            case 'h':
                puts("\nCurrently --help only displays the following information about the program options.");
                puts("If you think something else should be here let us know @ github.com/alt-romes/programmer-calculator");
                puts("The following options customize the interface: -ibxdos");
                puts("--history = -i");
                puts("--binary = -b");
                puts("--hex = -x");
                puts("--decimal = -d");
                puts("--operation = -o");
                puts("--symbol = -s\n");
                exit(0);
                break;

            case 'v':
                printf("Programmer calculator %s\n", VERSION);
                exit(0);
                break;

            case 'i':
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

            default:
                exit(EXIT_FAILURE);
        }
    }


    init_gui(&displaywin, &inputwin);

    // Set handler for CTRL+C to clean exit
    signal(SIGINT, exit_pcalc_success);

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
    numbers = create_numberstack(4);
    operation* current_op = NULL;

    // Initalize history pointers with NULL (realloc will bahave like malloc)
    history.records = NULL;
    searchHistory.records = NULL;

    // Start numberstack and history with 0
    push_numberstack(numbers, 0);
    add_to_history(&history, "0");
    // Display number on top of the stack (0)
    draw(numbers, current_op);

    // No longer add empty string to history bottom, because the scroll was reversed
    /* add_to_history(&searchHistory, ""); */

    //Main Loop
    for (;;) {

        // Get input
        char in[MAX_IN+1];

        // Make sure that if enter is pressed, a len == 0 null terminated string is in "in"
        in[0] = '\0';

        get_input(in);

        process_input(&current_op, in);

        // Display number on top of the stack
        draw(numbers, current_op);
    }

    endwin();

    return 0;
}

static long long pushnumber_from_string(char* in, numberstack* numbers) {

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

static void process_input(operation** current_op, char* in) {

    // Process input

    // There's an operation if one of the operation symbols is found in the input string
    char* op = strpbrk(in, ALL_OPS);

    if (op != NULL) {

        // An operation symbol was found

        /* There are four valid situations when an operation symbol
         * is found in the input string
         *
         * 1 - just the op i.e. "+"
         * 2 - an expression ending with an op i.e. "2+"
         * 3 - an op then an expression i.e. "+2"
         * 4 - an expression i.e. "1+2*3"
         */

        // Tokenize the input
        char* tokens = tokenize(in);

        // TODO: add the tokens and groups of tokens found to the history

        if (strchr(ALL_OPS, tokens[0]) && tokens[0] != NOT_SYMBOL) {

            // The first token is an op that's not a prefix | case 1 or case 3

            // Set the current operation as the operation structure for that symbol
            *current_op = getopcode(tokens[0]);

            // Add the operation to history
            char opchar[2] = {tokens[0], '\0'};
            add_to_history(&history, opchar);

            // Duplicate tokens string starting from the next position and free previous tokens
            char* tokens_wout_op = strdup(tokens+1);

            free(tokens);

            tokens = tokens_wout_op;

        }
        else {

            // The first token is a number or number with prefix -> clear the previous stack and history

            clear_numberstack(numbers);
            clear_history();

        }

        int ntokens = strlen(tokens);
        if (ntokens > 0) {

            // Add the tokens to history as a whole, for now...
            add_to_history(&history, tokens);
            /* fprintf(stderr, "Added to history\n"); */

            // Parse the tokens into an expression
            // This function will free *tokens*
            exprtree expression = parse(tokens);
            /* fprintf(stderr, "Parsed expression\n"); */

            // Calculate the result of the expression
            // The globalmask is applied inside calculate
            long long result = calculate(expression);
            /* fprintf(stderr, "Calculated expression\n"); */

            // The expression is no longer needed since we have its value
            free_exprtree(expression);
            /* fprintf(stderr, "Freed expression\n"); */

            // Push result to the numberstack
            push_numberstack(numbers, result);
            /* fprintf(stderr, "Pushed result\n"); */


            /* if (strchr(ALL_OPS, tokens[ntokens-1])) { */
                
            /*     // Last token is an op | case 2 */
                
            /*     // Apply pending operations, then set a new one */
            /*     apply_operations(numbers, current_op); */

            /*     // Set a new operation from the symbol */
            /*     *current_op = getopcode(tokens[ntokens-1]); */
                
            /* } */

        }

    }

    else if (!strcmp(in, "quit") || !strcmp(in, "q") || !strcmp(in, "exit"))
        exit_pcalc(0);

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
        globalmask = shr((DEFAULT_MASK_SIZE-globalmasksize), DEFAULT_MASK);

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
            // Or if is an empty string (and if it is, set the operation as NULL)
            if (*current_op == NULL || (in[0] == '\0' && !(*current_op = NULL)) ) {

                clear_numberstack(numbers);
                clear_history();
            }

            long long aux = pushnumber_from_string(in, numbers);

            if(strstr(in, "0b") != NULL)
                add_number_to_history(aux, NTYPE_BIN);
            else if (strstr(in, "0x") != NULL)
                add_number_to_history(aux, NTYPE_HEX);
            else
                add_number_to_history(aux, NTYPE_DEC);

        }

    }

    // Apply operations
    apply_operations(numbers, current_op);
}


static void apply_operations(numberstack* numbers, operation** current_op) {

    if (*current_op != NULL) {

        unsigned char noperands = (*current_op)->noperands;

        if (numbers->size >= noperands) {

            long long operands[2] = {0};

            for (unsigned char i=0; i < noperands; i++)
                operands[i] = *pop_numberstack(numbers);

            long long result = (*current_op)->execute(operands[0], operands[1]) & globalmask;

            push_numberstack(numbers, result);

            *current_op = NULL; // Set to invalid operation
        }
    }

}


static void get_input(char* in) {

    char inp;
    int history_counter = searchHistory.size;
    
    // Is the cursor at the end of the line or somewhere in the middle
    int browsing = 0;

    // Collect input until enter is pressed
    for (int pos = 0, len = 0; (inp = getchar()) != 13;) {
        int searched = 0;
        
        // Handles all arrow keys
        if (inp == 27) {
            getchar();
            inp = getchar();
            if (inp == 'A') {
                // Up arrow
                browsehistory(in, -1, &history_counter);
                len = strlen(in);
                searched = 1;
                browsing = 0;
            }
            else if (inp == 'B') {
                // Down arrow
                browsehistory(in, 1, &history_counter);
                len = strlen(in);
                searched = 1;
                browsing = 0;
            }
            else if (inp == 'D') {
                // Left arrow
                if (pos != 0) {
                    pos--;
                    browsing = 1; // The left arrow will always be in browsing mode, so no need for a check
                }
                searched = 1;
            }
            else if (inp == 'C') {
                // Right arrow
                if (browsing) {    // The right arrow should only work while in the middle of the input
                    pos++;
    
                    // Exit browsing mode if the cursor is at the end of the input
                    if (pos == len) {
                        browsing = 0;
                    }
                }
                searched = 1;
            }
        }

        if (inp == 127) {
            // Backspace

            if (pos != 0) {
                pos--;
                len--;
                inp = '\0';
            }
            else {
                // Skip printing if backspace was pressed but nothing was done, otherwise a strange undefined character is printed
                searched = 1;
            }
        }

        // Prevent user to input more than MAX_IN
        if(!searched && len <= MAX_IN) {
            if (!browsing) {
                // If the cursor is at the end of the text
                
                // Append char to in array
                in[pos] = inp;
                in[++pos] = '\0';
                len++; // Make sure that len is still equal to pos

                if (inp == '\0') {
                    // Clear screen from previous input
                    mvwprintw(inputwin, 1, 22 + --len, " ");
                }
            }
            else {
                // If the cursor is in the text, not at the end

                if (inp == '\0') {
                    // Backspace
                    
                    // Move all of in after pos one space back
                    for (int i = pos; i <= len; i++) {
                        in[i] = in[i + 1];
                    }
                    // Clear screen from previous input
                    mvwprintw(inputwin, 1, 22 + len, " ");
                }
                else {
                    // Everything except backspace

                    // Move all of in after pos one space forward to make room for the new input
                    len++;
                    for (int i = len; i > pos; i--) {
                        in[i] = in[i - 1];
                    }
                    // Append char to in array
                    in[pos++] = inp;
                
                }
            }
        }
        // This saves having to increment pos everytime len is incremented when youre not browsing
        if (!browsing) { pos = len; }
        
        // Finaly print input
        sweepline(inputwin, 1, 22);

        mvwprintw(inputwin, 1, 22, "%s", in);

        wmove(inputwin, 1, 22 + pos); // Move the cursor
        
        wrefresh(inputwin);
    }

    if (in[0] != '\0' && (searchHistory.size == 0 || strcmp(in, searchHistory.records[searchHistory.size - 1]))) {
        add_to_history(&searchHistory, in);
    }

}


void exit_pcalc(int code) {

    free_history(&history);
    free_history(&searchHistory);
    free_numberstack(numbers);

    endwin();

    switch (code) {

        case 0: break;
        case MEM_FAIL: fprintf(stderr, "OUT OF MEMORY\n"); break;


    }

    exit(code);
}

static void exit_pcalc_success() {

    exit_pcalc(0);
}
