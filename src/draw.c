#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "draw.h"
#include "history.h"
#include "numberstack.h"
#include "operators.h"
#include "parser.h"

WINDOW* displaywin, * inputwin;

// ASCII control characters
const char* ctrl_chars[] = { "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", 
	"BEL", "BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI", "DLE", "DC1", "DC2",
	"DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC", "FS", "GS", 
	"RS", "US", "SPACE"};

int wMaxX;
int wMaxY;

int operation_enabled = 1;
int decimal_enabled = 1;
int hex_enabled = 1;
int ascii_enabled = 1;
int symbols_enabled = 1;
int binary_enabled = 1;
int history_enabled = 1;
int colors_enabled = 0;
int alt_colors_enabled = 0;

int use_interface = 1;

static void printbinary(uint64_t, int);
static void printhistory(numberstack*, int);

void init_gui() {

    if (use_interface) {

        initscr();
        /* Only use colors if set so and if available */
        if (colors_enabled && has_colors() == true) {
            start_color();
            /* Every color pair needs to be initalized before use */
            init_pair(COLOR_PAIR_OPERATION, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_PAIR_DECIMAL, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_PAIR_HEX, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_PAIR_BINARY, COLOR_CYAN, COLOR_BLACK);
            init_pair(COLOR_PAIR_BINARY_ALT, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_PAIR_SYMBOLS, COLOR_YELLOW, COLOR_BLACK);
            init_pair(COLOR_PAIR_HISTORY, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(COLOR_PAIR_INPUT, COLOR_YELLOW, COLOR_BLACK);
        } else {
            /* Disable colors if terminal does not support colors */
            colors_enabled = 0;
        }
        cbreak();

        getmaxyx(stdscr, wMaxY, wMaxX);

        displaywin = newwin(wMaxY-3, wMaxX, 0, 0);
        refresh();

        box(displaywin, ' ', 0);
        if (symbols_enabled) {

            mvwprintw_colors(displaywin, wMaxY-8, 2, COLOR_PAIR_SYMBOLS, "ADD  +    SUB  -    MUL  *    DIV  /\n");
            wprintw_colors(displaywin, COLOR_PAIR_SYMBOLS, "  MOD  %%    AND  &    OR   |    NOR  $\n");
            wprintw_colors(displaywin, COLOR_PAIR_SYMBOLS, "  XOR  ^    NOT  ~    SL   <    SR   >\n");
            wprintw_colors(displaywin, COLOR_PAIR_SYMBOLS, "  RL   :    RR   ;    2's  _    SE   @");
        }
        wrefresh(displaywin);
        inputwin = newwin(3, wMaxX, wMaxY-3, 0);
        refresh();
        box(inputwin, ' ', 0);
        wrefresh(inputwin);

    }

}

static void printbinary(uint64_t value, int priority) {

    uint64_t mask = ((uint64_t) 1) << (globalmasksize - 1); // Mask starts at the last bit to display, and is >> until the end

    int i=DEFAULT_MASK_SIZE-globalmasksize;

    mvwprintw_colors(displaywin, 8-priority, 2, COLOR_PAIR_BINARY, "Binary:    \n         %02d  ", globalmasksize); // %s must be a 2 digit number

    for (; i<64; i++, mask>>=1) {

        uint64_t bitval = value & mask;
        wprintw_colors(displaywin,
            (alt_colors_enabled && bitval) ? COLOR_PAIR_BINARY_ALT : COLOR_PAIR_BINARY,
            "%c", bitval ? '1' : '0');

        if (i%16 == 15 && 64 - ((i/16+1)*16))
            // TODO: Explain these numbers better (and decide if to keep them)
            wprintw_colors(displaywin, COLOR_PAIR_BINARY,"\n         %d  ", 64-((i/16)+1)*16);
        else if (i%8 == 7)
            wprintw_colors(displaywin, COLOR_PAIR_BINARY, "   ");
        else if (i%4 == 3)
            wprintw_colors(displaywin, COLOR_PAIR_BINARY, "  ");
        else
            waddch(displaywin, ' ');

    }
}

static void printhistory(numberstack* numbers, int priority) {
    int currY,currX;
    mvwprintw_colors(displaywin, 14-priority, 2, COLOR_PAIR_HISTORY, "History:   ");
    for (int i=0; i<history.size; i++) {
        getyx(displaywin,currY,currX);
        if(currX >= wMaxX-3 || currY > 14) {
            clear_history();
            uint64_t aux = *top_numberstack(numbers);
            add_number_to_history(aux, 0);
        }
        wprintw_colors(displaywin, COLOR_PAIR_HISTORY, "%s ", history.records[i]);
    }
}

static void display_ascii_hex(uint64_t value, int priority) {
	// ASCII not enabled, just display HEX
	// Or ASCII out of rante
	if ((hex_enabled && !ascii_enabled) || value > 127) {
		mvwprintw_colors(displaywin, priority, 2, COLOR_PAIR_HEX, "Hex:       0x%llX", value);
		return;
	}
	
	// HEX not enabled, just display ASCII 
	if (!hex_enabled && ascii_enabled) {
		// Display control characters
		if (value < 33) {
			mvwprintw_colors(displaywin, priority, 2, COLOR_PAIR_HEX, "ASCII:      %s", ctrl_chars[value]);
		}
		// Display DEL (Dec: 127)
		else if (value == 127) {
			mvwprintw_colors(displaywin, priority, 2, COLOR_PAIR_HEX, "ASCII:      DEL");
		}
		// Display printable characters
		else {
			mvwprintw_colors(displaywin, priority, 2, COLOR_PAIR_HEX, "ASCII:      %c", (uint8_t)value);
		}
		return;
	}
	
	// Both ASCII and HEX enabled
	// Display control characters 
	if (value < 33) {
		mvwprintw_colors(displaywin, priority, 2, COLOR_PAIR_HEX, "Hex:       0x%llX		ASCII:      %s", value, ctrl_chars[value]);
	}
	// Display DEL (Dec: 127)
	else if (value == 127) {
		mvwprintw_colors(displaywin, priority, 2, COLOR_PAIR_HEX, "Hex:       0x%llX		ASCII:      DEL");
	}
	else {
		// Display printable characters
		mvwprintw_colors(displaywin, priority, 2, COLOR_PAIR_HEX, "Hex:       0x%llX		ASCII:      %c", value, (uint8_t)value);
	}
}


void draw(numberstack* numbers, operation* current_op) {
    
    uint64_t* np = top_numberstack(numbers);
    uint64_t n;

    if (np == NULL) n = 0;
    else n = *np;

    if (use_interface) {

        int prio = 0; // Priority

        // Clear lines
        for(int i = 2 ; i < 16 ; i++) {
            sweepline(displaywin, i, 0);
        }

        if(!operation_enabled) prio += 2;
        else mvwprintw_colors(displaywin, 2, 2, COLOR_PAIR_OPERATION, "Operation: %c\n", current_op ? current_op->character : ' ');

        if(!decimal_enabled) prio += 2;
        else mvwprintw_colors(displaywin, 4-prio, 2, COLOR_PAIR_DECIMAL, "Decimal:   %lld", (long long)n);

        if(!hex_enabled && !ascii_enabled) prio += 2;
		else display_ascii_hex(n, 6-prio);

        if(!binary_enabled) prio +=6;
        else printbinary(n,prio);

        if(!history_enabled) prio += 2;
        else printhistory(numbers,prio);

        wrefresh(displaywin);

        // Clear input
        sweepline(inputwin, 1, 19);

        // Prompt input
        mvwprintw_colors(inputwin, 1, 2, COLOR_PAIR_INPUT, "Number or operator: ");
        wrefresh(inputwin);

    }
    else {

        printf("Decimal: %lld, Hex: 0x%llx, Operation: %c\n", (long long)n, (unsigned long long)n, current_op ? current_op->character : ' ');
        /* printf("created|freed -> tokens: %d|%d, parsers: %d|%d, trees: %d|%d\n", total_tokens_created, total_tokens_freed, total_parsers_created, total_parsers_freed, total_trees_created, total_trees_freed); */
    }
}

void mvwprintw_colors(WINDOW* w, int y, int x, enum colors color_pair, const char* format, ...) {
    /* Prints colors if available otherwise not */
    va_list ap;
    va_start(ap, format);
    wmove(w, y, x);
    wattron(w, COLOR_PAIR(color_pair));
    vw_printw(w, format, ap);
    wattroff(w, COLOR_PAIR(color_pair));
    va_end(ap);

}

void wprintw_colors(WINDOW* w, enum colors color_pair, const char* format, ...) {
    /* Prints colors if available otherwise not */
    va_list ap;
    va_start(ap, format);
    wattron(w, COLOR_PAIR(color_pair));
    vw_printw(w, format, ap);
    wattroff(w, COLOR_PAIR(color_pair));
    va_end(ap);
}

void update_win_borders(numberstack* numbers) {

    doupdate();
    init_gui();
    draw(numbers, current_op);
}


void sweepline(WINDOW* w, int y, int x) {
    wmove(w, y, x);
    wclrtoeol(w);
}
