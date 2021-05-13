#ifndef _DRAW_H
#define _DRAW_H

#include <ncurses.h>

#include "numberstack.h"
#include "operators.h"

enum colors {

    COLOR_PAIR0,
    COLOR_PAIR1,
    COLOR_PAIR2,
    COLOR_PAIR3,
    COLOR_PAIR4,
    COLOR_PAIR5,
    COLOR_PAIR6,
    COLOR_PAIR7,
    COLOR_PAIR8

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
