#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ncurses.h>

#include "draw.h"
#include "global.h"
#include "history.h"
#include "numberstack.h"
#include "operators.h"






/*---- Structures -------------------------------------------------*/


extern struct history history;
extern struct history searchHistory;


/*---- Function Prototypes ----------------------------------------*/


// Drawing
extern WINDOW* displaywin, * inputwin;
// General
void process_input(numberstack*, operation**, char*);
void get_input(char *);
void apply_operations(numberstack*, operation**);



/*---- Define Operations and Global Vars --------------------------*/


// Variables
extern int wMaxX;
extern int wMaxY;

extern int operation_enabled;
extern int decimal_enabled;
extern int hex_enabled;
extern int symbols_enabled;
extern int binary_enabled;
extern int history_enabled;

#define ALL_OPS "+-*/&|$^<>()%~'"
#define VALID_NUMBER_INPUT "0123456789abcdefx"

extern unsigned long long globalmask;
extern int globalmasksize;

extern operation operations[];






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
    char * op = strpbrk(in, ALL_OPS);

    if (op != NULL) {

        char * in_saved;
        int niterations = 0;

        while (op != NULL) {

            // An operation symbol was found

            /* Before the strtok replaces the operator with \0 when searching for tokens
             * Save the string starting from right after the operator so we can search it later for more ops
             */
            in_saved = strdup(op) + 1;

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
                    add_to_history(&history, opchar);
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
                // History size is always > 0 because when "nothing" is in it, there should actually be a 0 there
                if(strcmp(opchar, history.records[history.size-1]))
                    add_to_history(&history, opchar);

                // There's definitely an operation already in the stack
                // This will be important when we add two different symbols in a row (i.e. +*)
                // Because only the first will be added to history
                operationInStack = 1;
            }


            // Read all the 2 next numbers (split by the operation) (case 2, 3 and 4) (only runs twice if is case 4)
            while (token != NULL && numbers->size < 2) {

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
                    add_to_history(&history, opchar);
                    operationInStack = 1;
                }

                // Following iterations should only treat case 1 and 3
                if (niterations == 0) {

                    // If this is the second number we've processed
                    // We won't read a third one, and we'll exit with the token intact
                    // So we can check ahead for the case a++b
                    if (numbers->size == 2)
                        break;

                    // Get the second token (next number)
                    token = strtok(NULL, opchar);
                } else {
                    // When we're not dealing wit the first iteration, we'll just allow this while to read one number
                    token = NULL;
                }

            }

            // The next iterations will only be able to find case 1 and 3
            
            // Read comment a few lines below about distance_to_op
            int distance_to_op, distance_to_token;
            distance_to_op = op - in;

            // Try to find an operation in the new string
            op = strpbrk(in_saved, ALL_OPS);

            // Make sure the new op we're reading comes after the last token read, to avoid a++b
        
            // Because op and token are two different strings, we need to measure the distance to the beginning first to compare them later 
            distance_to_op += op - in_saved;
            distance_to_token = token - in;

            // If it comes before, we need to search for the next one
            if (numbers->size == 2 && token != NULL && op != NULL && distance_to_op < distance_to_token)
                op = strpbrk(op+1, ALL_OPS);

            // In the next iteration will have everything at the right of the new operation as input 
            in = op;

            // If we do find one, then we'll try to execute the operation
            apply_operations(numbers, current_op);

            niterations++;
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
    apply_operations(numbers, current_op);    
}


void apply_operations(numberstack* numbers, operation** current_op) {

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


void get_input(char *in) {

    char inp;
    int history_counter = searchHistory.size;

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
                // Up arrow
                browsehistory(in, -1, &history_counter);
                i = strlen(in);
                searched = 1;
            }
            else if (inp == 'B')
            {
                // Down arrow
                browsehistory(in, 1, &history_counter);
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
        sweepline(inputwin, 1, 22);
            
        mvwprintw(inputwin, 1, 22, in);
        wrefresh(inputwin);
    }
    
    if (in[0] != '\0' && (searchHistory.size == 0 || strcmp(in, searchHistory.records[searchHistory.size - 1])))
        add_to_history(&searchHistory, in);

}

