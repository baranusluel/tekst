#include <algorithm>
#include <curses.h>
#include <iostream>
#include <string>
#include "Buffer.h"
#include "Utils.h"

// Flags
/// TODO: Add cmd argument for debug flag
#define DEBUG 1

// While in curses terminal mode can't print to std::cout so keeping
// a tmp stream and printing at termination.
// Variable is extern declared in Utils.h for global usage.
std::ostringstream debugLog;

// Macro for checking CTRL + KEY presses
#define ctrl(x) ((x) & 0x1f)

// Displays desired text line from file in target row
void displayLine(int fileLineNum, int displayRow, Buffer* b) {
    // Get line from buffer in memory
    std::string line = b->getLine(fileLineNum);
    /// TODO: Add second argument as COLS?
    mvaddstr(displayRow, 0, line.c_str());
}

// Cleanup for terminating application.
// Stops curses mode, prints given message and debug log.
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
    
    std::unique_ptr<Buffer> b; // Owned reference to text buffer
    /// TODO: Add cmd argument for buffer type
    BufferType bType = static_cast<BufferType>(0);
    debugLog << "Buffer type: " << Buffer::bufferTypeToString(bType) << std::endl;
    try {
        b = Buffer::createBuffer(bType, argv[1]);
    } catch (std::string msg) {
        dumpAndExit(msg);
        return 0;
    }

    // Setup curses mode
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    // Enable vertical scrolling
    scrollok(stdscr, TRUE);
    // Scrolling region excludes header and footer rows
    /// TODO: Update on resize
    setscrreg(1, LINES - 2);

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

    // Display text from read file in visible rows
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
                // If cursor at leftmost position, backspace has no effect
                if (col == 0) break;
                // Otherwise move left, and flow into KEY_DC case below to delete char
                move(row, --col);
                // No break
            case KEY_DC:
                /// TODO: Refactor magic number offsets
                b->delChar(row - 1, col);
                delch();
                break;
            case KEY_LEFT:
                move(row, col = std::max(col - 1, 0));
                break;
            case KEY_RIGHT:
                move(row, col = std::min(col + 1, COLS - 1));
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
                break;
            case KEY_HOME:
                move(row, col = 0);
                break;
            case KEY_END:
                /// TODO: This should only go to end of line (delimiter)
                move(row, col = COLS - 1);
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
                /// TODO: Refactor with KEY_RIGHT case
                move(row, col = std::min(col + 1, COLS - 1));
        }
        refresh();
        ch = getch();
    }

    dumpAndExit("");
    return 0;
}