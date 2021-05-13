#ifndef _DRAW_H
#define _DRAW_H

#include <ncurses.h>

#include "numberstack.h"
#include "operators.h"

enum colors {

    COLOR0,
    COLOR1,
    COLOR2,
    COLOR3,
    COLOR4,
    COLOR5,
    COLOR6,
    COLOR7,
    COLOR8

};

extern WINDOW* displaywin, *inputwin;

extern int wMaxX, wMaxY;
extern int operation_enabled, decimal_enabled, hex_enabled, symbols_enabled, binary_enabled, history_enabled, colors_enabled;

extern int use_interface;

void init_gui();
void draw(numberstack*, operation*);
void update_win_borders(numberstack* numbers);
void sweepline(WINDOW*, int, int);
void mvwprintw_colors(WINDOW* w, int y, int x, enum colors color_pair, const char* format, ...);
void wprintw_colors(WINDOW* w, enum colors color_pair, const char* format, ...);

#endif
