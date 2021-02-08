#ifndef _DRAW_H
#define _DRAW_H

#include <ncurses.h>

#include "numberstack.h"
#include "operators.h"


extern WINDOW* displaywin, *inputwin;

extern int wMaxX, wMaxY;
extern int operation_enabled, decimal_enabled, hex_enabled, symbols_enabled, binary_enabled, history_enabled;

extern int use_interface;

void init_gui();
void draw(numberstack*, operation*);
void sweepline(WINDOW*, int, int);

#endif
