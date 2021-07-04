#include <algorithm>
#include <curses.h>
#include <iostream>
#include <string>
#include "Buffer.h"
#include "Utils.h"

// Flags
#define DEBUG               1 // TODO: Add cmd argument for debug flag

// While in curses terminal mode can't print to std::cout so keeping
// a tmp stream and printing at termination.
// Variable is extern declared in Utils.h for global usage.
std::ostringstream debugLog;

#define ctrl(x)             ((x) & 0x1f)

void displayLine(int fileLineNum, int displayRow, Buffer* b) {
    // Get line from buffer in memory
    std::string line = b->getLine(fileLineNum);
    // Deleting carriage returns because curses makes it overwrite text
    size_t CR = line.find('\r');
    if (CR != std::string::npos)
        line.erase(CR, 1);
    mvaddstr(displayRow, 0, line.c_str()); // TODO: Add second argument as COLS?
}

void dumpAndExit(std::string msg) {
    endwin();
    if (DEBUG)
        std::cout << debugLog.str();
    std::cout << msg << std::endl;
}

int main (int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "tekst <filename>" << std::endl;
        return 0;
    }
    debugLog << "Opening file: " << argv[1] << std::endl;
    
    std::unique_ptr<Buffer> b;
    // TODO: Add cmd argument for buffer type
    BufferType bType = static_cast<BufferType>(0);
    debugLog << "Buffer type: " << Buffer::bufferTypeToString(bType) << std::endl;
    try {
        b = Buffer::createBuffer(bType, argv[1]);
    } catch (std::string msg) {
        dumpAndExit(msg);
        return 0;
    }

    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    scrollok(stdscr, TRUE);
    setscrreg(1, LINES - 2); // TODO: Update on resize

    // Draw window chrome
    attron(A_BOLD);
    addstr("tekst by Baran Usluel\n");
    mvaddstr(LINES-1, 0, argv[1]);
    attroff(A_BOLD);
    // Default row is after header
    int row = 1;
    int col = 0;
    int scrollOffset = 0;
    move(row, col);

    // Display text from read file
    for (int i = 0; i < LINES - 2; ++i) {
        displayLine(i, row + i, b.get());
    }
    move(row, col);
    refresh();

    // Input loop
    int ch = getch();
    while (ch != ctrl('c')) { // Exit code
        switch (ch) {
            case KEY_BACKSPACE:
            case KEY_LEFT:
                move(row, col = std::max(col - 1, 0));
                if (ch == KEY_BACKSPACE) {
                    b->delChar(row - 1, col); // TODO: Refactor magic number offsets
                    delch();
                }
                // Refreshing in particular cases instead of after entire switch statement because
                // in default case echochar already includes a refresh (faster than addch + refresh).
                refresh();
                break;
            case KEY_DC:
                b->delChar(row - 1, col);
                delch();
                refresh();
                break;
            case KEY_RIGHT:
                move(row, col = std::min(col + 1, COLS - 1));
                refresh();
                break;
            case KEY_UP:
                if (row == 1) {
                    if (scrollOffset > 0) {
                        scrl(-1);
                        scrollOffset--;
                        displayLine(scrollOffset, row, b.get());
                    }
                } else {
                    row--;
                }
                move(row, col);
                refresh();
                break;
            case KEY_DOWN:
                if (row == LINES - 2) {
                    scrl(1);
                    scrollOffset++;
                    displayLine(LINES - 3 + scrollOffset, row, b.get());
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
            case ctrl('s'):
                try {
                    b->save();
                } catch (std::string msg) {
                    dumpAndExit(msg);
                    return 0;
                }
                break;
            default:
                insch(ch);
                b->insertChar(ch, row - 1, col);
                move(row, col = std::min(col + 1, COLS - 1)); // TODO: Refactor with KEY_RIGHT case
                refresh();
                // getyx(curscr, row, col);
        }
        ch = getch();
    }

    dumpAndExit("");
    return 0;
}