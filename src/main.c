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






/*---- Function Prototypes ----------------------------------------*/


// General
static void process_input(operation**, char*);
static void get_input(char*, int);
static void apply_operations(numberstack*, operation**);
static void exit_pcalc_success();


/*---- Define Operations and Global Vars --------------------------*/


#define ALL_OPS "+-*/&|$^<>()%~'"
#define VALID_NUMBER_INPUT "0123456789abcdefx"


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
                puts("Programmer calculator v1.8");
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

    // Get the maximum input length given the width of the terminal
    // This could be moved into the main loop so that if the window is resized max_in is also adjusted (after pressing enter), but it doesn't account for entries from the history being longer
    // So when the code for resizing the windows is implemented this might also need changing a bit 
    int x;
    getmaxyx(inputwin, x, x);    // Get the width of the window, slightly abusing how getmaxyx works to not have to use a second variable for the y value (which we don't need)

    int max_in = MAX_IN;
    if (x < (MAX_IN + 24)) {                      // 24 is the width that is used regardless of input, so MAX_IN + 24 is the minimum width of the terminal for MAX_IN characters of input 
        max_in = MAX_IN - ((MAX_IN + 24) - x);    // So this gives the maximum input size for a given terminal width (MAX_IN minus the difference between the minimum width for MAX_IN and the current width)
    }

    //Main Loop
    for (;;) {
        // Get input
        char in[max_in+1];

        // Make sure that if enter is pressed, a len == 0 null terminated string is in "in"
        in[0] = '\0';

        get_input(in, max_in);

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
         * 1 - just the symbol i.e. "+"
         * 2 - a number then a symbol i.e. "2+"
         * 3 - a symbol then a number i.e. "+2"
         * 4 - a number, a symbol and a number i.e. "2+2"
         *
         * The following code, first, handles a number that comes before the operation
         * And then, in the while loop, handles the rest of the input that starts with an operation
         */

        /* Before the strtok replaces the operator with \0 when searching for tokens
         * Save the string starting at the operator so we can reuse after handling the first number
         */
        char* in_saved = strdup(op);

        // Find the first number in the string, by removing the operation symbol and everything after from the string
        char* token = strtok(in, ALL_OPS);

        // When the first number comes before the operation symbol (case 2 and 4)
        if (token != NULL && token < op) {

            // History and numbers will be overridden by the input number and operation
            // So we clean the stack and the history
            clear_numberstack(numbers);
            clear_history();

            // We have the number before the token, and we'll push it to the number stack right away
            long long aux = pushnumber_from_string(token, numbers);

            // History will display the number in the format inserted
            // So we must separate 0b from 0x from a normal decimal
            // add_number_to_history takes a second parameter to display accordingly
            if(strstr(token, "0b") != NULL)
                add_number_to_history(aux, NTYPE_BIN);
            else if (strstr(token, "0x") != NULL)
                add_number_to_history(aux, NTYPE_HEX);
            else
                add_number_to_history(aux, NTYPE_DEC);

        }

        // We'll set the input string for the next while - it'll handle the rest of the user input
        // The worked input should now be of type "[op](expression)"
        // this means, it'll start right where the operation is - after having handled the first number
        in = in_saved;

        // The op string was deleted because of the strtok, we're now setting it to the beginning of the *in* string
        // Because of the way we handled it, the operation will be right at the start of *in*
        op = in;

        // Because we're manipulating the string, we need a variable to keep track of one of the addresses we need to free
        char* last_allocated_addr = in_saved;

        // The next while will only handle case 1 and 3 - we already have the operation ready

        int niterations = 0;

        while (op != NULL) {

            // Set the current operation as the operation structure for that symbol
            *current_op = getopcode(*op);

            /* Before the strtok replaces the operator with \0 when searching for tokens
             * Save the string starting right from the operator so we can search it later for more ops
             */
            in_saved = strdup(op+1);

            // Find the next number in the string
            char* token = strtok(in, ALL_OPS);

            // Logic to add the op in the history

            // String to hold just the operator symbol character
            char opchar[2];
            opchar[0] = *op;
            opchar[1] = '\0';

            // Make sure the operation isn't already in history to avoid duplicate symbols
            // History size is always > 0 because when "nothing" is in it, there should actually be a 0 there
            if (strcmp(opchar, history.records[history.size-1]))
                add_to_history(&history, opchar);

            // Logic to add the number to the stack and to the history

            if (token != NULL) {

                // We have a number, so we'll push it to the number stack
                long long aux = pushnumber_from_string(token, numbers);

                // History will display the number in the format inserted
                // So we must separate 0b from 0x from a normal decimal
                // add_number_to_history takes a second parameter to display accordingly
                if(strstr(token, "0b") != NULL)
                    add_number_to_history(aux, 2);
                else if (strstr(token, "0x") != NULL)
                    add_number_to_history(aux, 1);
                else
                    add_number_to_history(aux, 0);

            }

            // Try to apply the operation, which will only actually be applied if enough numbers were added to the numberstack
            apply_operations(numbers, current_op);


            // We need to make sure the new op we're going to read comes after the last token read, to avoid a++b

            // Define helper distances to check if the new operand comes before the last read token and avoid a++b
            int distance_to_previous_op, distance_to_new_op, distance_to_last_token;
            // This has to come before we reassign *op* (distance from last op to the start of input)
            distance_to_previous_op = op - in;
            distance_to_last_token = token - in; // (token - in) is the distance from the last token to the beginning of the input


            // We no longer need the memory saved in the last allocated address here, and we set it to the last allocated address so it is cleared again
            free(last_allocated_addr);
            last_allocated_addr = in_saved;

            // Try to find a next operation in the string starting right after the last op
            op = strpbrk(in_saved, ALL_OPS);

            // Because op and token are two different strings, we need to measure the distance to the beginning first to compare them later
            // (op - in_saved) is the distance from the new op to the saved string that starts directly after the previous op
            distance_to_new_op = distance_to_previous_op + (op - in_saved);

            // If the new op comes before the last token, then we have a++b. We'll search for a next possible op
            if (token != NULL && op != NULL && distance_to_new_op < distance_to_last_token)
                op = strpbrk(op+1, ALL_OPS);

            // In the next iteration will start right with at the new operation, and have the rest of the input in front of it
            in = op;

            niterations++;

        }

        // When we exit the loop, free the only allocated memory left (that's in *in_saved*)
        free(in_saved);

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


static void get_input(char* in, int max_in) {

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

        // Prevent user to input more than max_in
        if(!searched && len <= max_in) {
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
