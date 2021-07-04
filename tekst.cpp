#include <algorithm>
#include <curses.h>
#include <iostream>
#include <string>
#include <vector>
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

// Lookup table for lengths of lines displayed on screen
std::vector<size_t> lineLengths;

// Displays desired text line from file in target row.
// Note that this method moves the cursor.
void displayLine(int fileLineNum, int displayRow, Buffer* b) {
    /// TODO: Make cursor temporarily invisible when scrolling, etc. to prevent flickering
    // Get line from buffer in memory
    std::string line = b->getLine(fileLineNum);
    /// TODO: Limit number of characters according to COLS?
    mvaddstr(displayRow, 0, line.c_str());
    // Update line lengths table with newly loaded line 
    lineLengths.insert(lineLengths.begin() + displayRow - 1, line.length());
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
    move(row, col);

    // Which column cursors wants to be on (for persistent
    // cursor position across varying line lengths)
    int colGoal = 0;

    int scrollOffset = 0;

    // Display text from read file in visible rows
    for (int i = 0; i < LINES - 2; ++i) {
        displayLine(i, i + 1, b.get());
    }
    move(row, col);
    refresh();

    // Input loop
    int ch = getch();
    while (ch != ctrl('c')) { // Exit code
        switch (ch) {
            /// TODO: Add page up/down key cases
            case KEY_BACKSPACE:
                // Special case if cursor at leftmost position
                if (col == 0) {
                    // If not first line of text, then delete EOL of previous line
                    if (row > 1) {
                        move(--row, colGoal = col = lineLengths[row - 2] - 1);
                    } else {
                        break;
                    }
                } else {
                    // Otherwise move left, and flow into KEY_DC case below to delete char
                    move(row, colGoal = --col);
                }
                // No break
            case KEY_DC:
                /// TODO: Refactor magic number offsets
                b->delChar(row - 1, col);
                delch();
                // If cursor is at end of line, deleting linebreak
                if (col == lineLengths[row - 1] - 1) {
                    deleteln(); // Deletes next row and shifts everything up
                    // Delete entry for current row from lineLengths, it will be readded by displayLine
                    lineLengths.erase(lineLengths.begin() + row - 1);
                    // Delete entry for the deleted row from lineLengths
                    lineLengths.erase(lineLengths.begin() + row - 1);
                    // Display updated (concatenated) line
                    displayLine(scrollOffset + row - 1, row, b.get());
                    // Display line that scrolled into view from bottom
                    displayLine(LINES - 3 + scrollOffset, LINES - 3, b.get());
                    move(row, col);
                } else {
                    lineLengths[row - 1]--;
                }
                break;
            case KEY_LEFT:
                move(row, colGoal = col = std::max(col - 1, 0));
                break;
            case KEY_RIGHT:
                move(row, colGoal = col = std::min(std::min(col + 1, COLS - 1), std::max((int)lineLengths[row - 1] - 1, 0)));
                break;
            case KEY_UP:
                if (row == 1) {
                    if (scrollOffset > 0) {
                        scrl(-1);
                        scrollOffset--;
                        // Delete entry from end of table because only tracking visible lines
                        lineLengths.erase(lineLengths.end() - 1);
                        // Load text to display in the new scrolled line.
                        // This also adds an entry to the lineLengths table
                        displayLine(scrollOffset, row, b.get());
                    }
                } else {
                    row--;
                }
                move(row, col = std::min(colGoal, std::max((int)lineLengths[row - 1] - 1, 0)));
                break;
            case KEY_DOWN:
                if (row == LINES - 2) {
                    scrl(1);
                    scrollOffset++;
                    // Delete entry from start of table because only tracking visible lines
                    lineLengths.erase(lineLengths.begin());
                    // Load text to display in the new scrolled line.
                    // This also adds an entry to the lineLengths table
                    displayLine(LINES - 3 + scrollOffset, row, b.get());
                } else {
                    row++;
                }
                /// TODO: Shouldn't be able to go down into empty space
                move(row, col = std::min(colGoal, std::max((int)lineLengths[row - 1] - 1, 0)));
                break;
            case KEY_HOME:
                move(row, colGoal = col = 0);
                break;
            case KEY_END:
                move(row, colGoal = col = lineLengths[row - 1] - 1);
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
                insch(ch); // Insert typed character on screen
                b->insertChar(ch, row - 1, col); // Insert character in buffer
                if (ch == '\n') {
                    // New length of current line is wherever linebreak was added
                    lineLengths[row - 1] = col + 1;
                    // Move cursor to start of next row
                    move(++row, colGoal = col = 0);
                    /// TODO: #17 Put text edit region in a curses window
                    insertln(); // Add new empty line on screen above cursor, shifting rest down
                    displayLine(scrollOffset + row - 1, row, b.get()); // Text to go in new line
                    move(row, col);
                    // Delete entry from end of table because only tracking visible lines
                    lineLengths.erase(lineLengths.end() - 1);
                } else {
                    // Move cursor right when character typed
                    move(row, colGoal = col = std::min(col + 1, COLS - 1));
                    lineLengths[row - 1]++;
                }
                
        }
        refresh();
        ch = getch();
    }

    dumpAndExit("");
    return 0;
}