#ifndef _DRAW_H
#define _DRAW_H

#include <ncurses.h>

#include "numberstack.h"
#include "operators.h"

enum colors {

    COLOR_PAIR_DEFAULT,
    COLOR_PAIR_OPERATION,
    COLOR_PAIR_DECIMAL,
    COLOR_PAIR_HEX,
    COLOR_PAIR_BINARY,
    COLOR_PAIR_BINARY_ALT,
    COLOR_PAIR_HISTORY,
    COLOR_PAIR_SYMBOLS,
    COLOR_PAIR_INPUT,

};

extern WINDOW* displaywin, *inputwin;

extern int wMaxX, wMaxY;
extern int operation_enabled, decimal_enabled, hex_enabled, ascii_enabled, symbols_enabled, binary_enabled, history_enabled, colors_enabled, alt_colors_enabled;

extern int use_interface;

void init_gui();
void draw(numberstack*, operation*);
void update_win_borders(numberstack* numbers);
void sweepline(WINDOW*, int, int);
void mvwprintw_colors(WINDOW* w, int y, int x, enum colors color_pair, const char* format, ...);
void wprintw_colors(WINDOW* w, enum colors color_pair, const char* format, ...);

#endif
