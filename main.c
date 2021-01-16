#include <ncurses.h>
#include <stdlib.h>
/* #include "stack.h" */

#define MAX_IN 50

void init_gui();
void draw();

int main(int argc, char *argv[])
{
    
    WINDOW* displaywin, * inputwin;
    init_gui(&displaywin, &inputwin);

    /* char* str = malloc(MAX_IN); */
    /* fgets(str, MAX_IN, stdin); */

    /* stack* numbers = create_stack(42); */

    mvwprintw(displaywin, 1, 2, "0");
    mvwprintw(displaywin, 2, 2, "0x0");
    mvwprintw(displaywin, 3, 2, "0 0 0 0");

    mvwprintw(inputwin, 1, 2, "Number or operator: ");

    wrefresh(displaywin);
    wrefresh(inputwin);

    do {

        char str[MAX_IN];
        wgetnstr(inputwin, str, MAX_IN);

        mvwprintw(displaywin, 1, 2, "%s\n", str);

        mvwprintw(inputwin, 1, 2, "Number or operator: ");

        wrefresh(displaywin);
        wrefresh(inputwin);

    } while (getch() != '\n');

    endwin();
    return 0;
}

void init_gui(WINDOW** displaywin, WINDOW** inputwin) {

    initscr();
    cbreak(); // exit on ctrl+c

    int ymax, xmax;
    getmaxyx(stdscr, ymax, xmax);

    *displaywin = newwin(ymax/2, xmax, 0, 0);
    refresh();

    /* box(*displaywin, ' ', 0); */
    wrefresh(*displaywin);

    *inputwin = newwin(3, xmax, ymax/2, 0);
    refresh();

    /* box(*inputwin, ' ', 0); */
    wrefresh(*inputwin);
}
