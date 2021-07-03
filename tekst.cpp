#include <algorithm>
#include <curses.h>

#include "Buffer.h"

#define ctrl(x)             ((x) & 0x1f)

int main (int argc, char* argv[]) {
    std::unique_ptr<Buffer> b = Buffer::createBuffer(static_cast<BufferType>(0));

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    scrollok(stdscr, TRUE);
    setscrreg(1, LINES - 2); // TODO: Update on resize

    // Draw window chrome
    printw("tekst by Baran Usluel\n");
    mvaddstr(LINES-1, 0, "end");

    int row = 1; // Default row is after header
    int col = 0;
    move(row, col);
    refresh();

    int ch = getch();
    while (ch != ctrl('c')) { // Exit code
        switch (ch) {
            case KEY_BACKSPACE:
            case KEY_LEFT:
                move(row, col = std::max(col - 1, 0));
                if (ch == KEY_BACKSPACE) delch();
                // Refreshing in particular cases instead of after entire switch statement because
                // in default case echochar already includes a refresh (faster than addch + refresh).
                refresh();
                break;
            case KEY_RIGHT:
                move(row, col = std::min(col + 1, COLS - 1));
                refresh();
                break;
            case KEY_UP:
                if (row == 1) {
                    scrl(-1);
                } else {
                    row--;
                }
                move(row, col);
                refresh();
                break;
            case KEY_DOWN:
                if (row == LINES - 2) {
                    scrl(1);
                } else {
                    row++;
                }
                move(row, col);
                refresh();
                break;
            case KEY_HOME:
                move(row, col = 0);
                refresh();
                break;
            case KEY_END:
                move(row, col = COLS - 1);
                refresh();
                break;
            default:
                echochar(ch);
                getyx(curscr, row, col);
        }
        ch = getch();
    }

    endwin();
}