#ifndef _DRAW_H
#define _DRAW_H

#include <ncurses.h>

#include "numberstack.h"
#include "operators.h"

void init_gui();
void draw(numberstack*, operation*);
void printbinary(long long, int);
void printhistory(numberstack*, int);
void browsehistory(char*, int, int*);
void sweepline(WINDOW*, int, int);
long long pushnumber(char * in, numberstack* numbers);


#endif
