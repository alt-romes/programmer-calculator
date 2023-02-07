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





#define VERSION "v3.0"





/*---- Function Prototypes ----------------------------------------*/


static void process_input(operation**, char*);
static void get_input(char*);
static void apply_operations(numberstack*, operation**);
static void exit_pcalc_success();





/*---- Main Logic -------------------------------------------------*/


int main(int argc, char* argv[])
{
    // Set all long arguments that can be used
    struct option long_options[] = {

        {"help",             no_argument, NULL, 'h'},
        {"version",          no_argument, NULL, 'v'},
        {"history",          no_argument, NULL, 'i'},
        {"binary",           no_argument, NULL, 'b'},
        {"hex",              no_argument, NULL, 'x'},
        {"decimal",          no_argument, NULL, 'd'},
        {"operation",        no_argument, NULL, 'o'},
        {"symbol",           no_argument, NULL, 's'},
        {"colors",           no_argument, NULL, 'c'},
        {"alternate-colors", no_argument, NULL, 'a'},
        {"no-interface",     no_argument, NULL, 'n'},
        {NULL,               0,           NULL,  0}

     };

    // Get command line options to hide parts of the display
    int opt;
    while ((opt = getopt_long(argc, argv, "hvibxdoscan", long_options, NULL)) != -1) {
        switch (opt) {

            case 'h':
                puts("Currently --help only displays the following information about the program options.");
                puts("If you think something else should be here let us know @ github.com/alt-romes/programmer-calculator");
                puts("The following options customize the interface: -ibxdos");
                puts("--history = -i");
                puts("--binary = -b");
                puts("--hex = -x");
                puts("--decimal = -d");
                puts("--operation = -o");
                puts("--symbol = -s");
                puts("Other options:");
                puts("--colors = -c");
                puts("--alternate-colors = -a (alternate colors for 1s and 0s in binary)");
                puts("--no-interface = -n");
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

            case 'n':
                use_interface = 0;
                break;

            case 'c':
                colors_enabled = 1;
                break;

            case 'a':
                colors_enabled = 1;
                alt_colors_enabled = 1;
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
    //operation* current_op = NULL;

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
        char in[MAX_IN + 1];

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

static void process_input(operation** current_op, char* in) {

    // Process input

    // Try to find a known command and handle it
    if (!strcmp(in, "quit") || !strcmp(in, "q") || !strcmp(in, "exit"))
        exit_pcalc(0);

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

    else if (strstr(in, "bit") != NULL) {

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

    else {

        // It's not a known command - handle input as expression
        
        
        // To handle the expression, first tokenize the input

        // Tokenize the input
        char* tokens = tokenize(in);

        // We need to check if the last token is an operation before it gets freed,
        // And save it, to set it as the current op after the input is processed
        operation* suffix_op = NULL;


        // Search for an operation symbol as the first token

        /* There are four valid situations when an operation symbol
         * is found in the tokens
         *
         * 1 - just the op i.e. "+"
         * 2 - an expression ending with an op i.e. "2+"
         * 3 - an op then an expression i.e. "+2"
         * 4 - an expression i.e. "1+2*3" (this case is handled as a number)
         */

        int ntokens = strlen(tokens);

        if (tokens[0] != '\0' && strchr(ALL_OPS, tokens[0]) && 
                (ntokens == 1 || (tokens[0] != NOT_SYMBOL && tokens[0] != TWOSCOMPLEMENT_SYMBOL && tokens[0] != SWAPENDIANNESS_SYMBOL))) {

            // The input is either just an op, or an expression that starts with an op that isn't a prefix | case 1 or case 3

            // Set the current operation as the operation structure for that symbol
            *current_op = getopcode(tokens[0]);

            // Add the operation to history
            char opchar[2] = {tokens[0], '\0'};
            add_to_history(&history, opchar);

            // Duplicate the *tokens* string starting from the immediate next position, and free previous tokens afterwards
            char* tokens_wout_op = strdup(tokens+1);

            free(tokens);

            tokens = tokens_wout_op;

            // The length of the tokens is now 1 character smaller
            ntokens--;

        }

        if (ntokens > 0 && strchr(ALL_OPS, tokens[ntokens-1])) {
            
            // Last token is an op | case 2

            // Set a new operation from the last symbol
            suffix_op = getopcode(tokens[ntokens-1]);

            // Remove the last token from the string
            tokens[ntokens-1] = '\0';
            ntokens--;
        }

        if (*current_op == NULL ||
                (in[0] == '\0' && !(*current_op = NULL))) {

            // There's no current operation and we're going to process a new number
            // -> clear the stack and history before processing it
            // Or the input was empty. When the input is empty set the operation to NULL

            clear_numberstack(numbers);
            clear_history();

        }

        if (ntokens > 0) {

            // Add the tokens to history as a whole, for now...
            add_to_history(&history, tokens);

            // Parse the tokens into an expression
            // This function will free *tokens*
            exprtree expression = parse(tokens);

            // Calculate the result of the expression
            // The globalmask is applied inside calculate
            long long result = calculate(expression);

            // The expression is no longer needed since we have its value
            free_exprtree(expression);

            // Push result to the numberstack
            push_numberstack(numbers, result);

            if (suffix_op != NULL) {
                
                // Last token is an op | case 2
                
                // Apply pending operation right away, to then set a new one
                apply_operations(numbers, current_op);

                // Set a new operation from the symbol
                *current_op = suffix_op;
                
                char opchar[2] = {suffix_op->character, '\0'};
                add_to_history(&history, opchar);
            }

        }
        else {
            // The input expression generated an empty token string.
            // Because parse() isn't called, we must free *tokens* manually
            free(tokens);
            total_tokens_freed++;
        }

        if (ntokens == 0 && *current_op == NULL) {

            // The op is null (means we cleared the stack before reading a number)
            // But we didn't read a number - so the stack is empty

            // Add needed 0 to history and to stack
            push_numberstack(numbers, 0);
            add_to_history(&history, "0");

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
    for (int pos = 0, len = 0; (inp = getchar()) != 13 && inp != '\n';) {

        // Get max possible input length
        int max = getmaxx(inputwin) - INPUT_START;

        int searched = 0;

        /* Check for forbidden keys
         * -1 is a key that indicates the terminal got resized
         *  5 is a key that indicates mouse wheel down 
         *  25 is a key that indicates mouse wheel up
         *  27 is a key that indicates an arrow key was pressed
         *  127 is a key that indicates the brackspace key was pressed
         */
        switch(inp) {
            
            case -1:
                update_win_borders(numbers);
            case 5:
            case 25:
                continue;
                break;

            case 27:
                getchar();
                inp = getchar();
                switch (inp) {

                    case 'A':
                        // Up arrow
                        browsehistory(in, -1, &history_counter);
                        len = strlen(in);
                        searched = 1;
                        browsing = 0;

                        break;

                    case 'B':
                        // Down arrow
                        browsehistory(in, 1, &history_counter);
                        len = strlen(in);
                        searched = 1;
                        browsing = 0;

                        break;

                    case 'C':
                        // Right arrow
                        if (browsing) {    // The right arrow should only work while in the middle of the input
                            pos++;
    
                            // Exit browsing mode if the cursor is at the end of the input
                            if (pos == len) {
                                browsing = 0;
                            }
                        }
                        searched = 1;

                        break;

                    case 'D':
                        // Left arrow
                        if (pos != 0) {
                            pos--;
                            browsing = 1; // The left arrow will always be in browsing mode, so no need for a check
                        }
                        searched = 1;

                        break;
                }
                break;

            case 127:
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
                break;

        }

        // Prevent user to input more than MAX_IN
        if(!searched && len <= MAX_IN && (len <= max || !use_interface)) {
            if (!browsing) {
                // If the cursor is at the end of the text
                
                // Append char to in array
                in[pos] = inp;
                in[++pos] = '\0';
                len++; // Make sure that len is still equal to pos

                if (inp == '\0') {
                    // Clear screen from previous input
                    sweepline(inputwin, 1, 22 + --len);
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

                    sweepline(inputwin, 1, 22 + len);
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

        // Clear input (only necessary because of the history feature)
        sweepline(inputwin, 1, 22);

        // Finaly print input
        if (use_interface)
            mvwprintw_colors(inputwin, 1, 22, COLOR_PAIR_DEFAULT, "%s", in);
        else
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
