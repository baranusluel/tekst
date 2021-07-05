#include <algorithm>
#include <curses.h>
#include <iostream>
#include <optional>
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
// Number of lines in text edit region (excludes header and footer)
#define LINES_TXT LINES - 2
#define COLS_TXT COLS

// Table of lines / strings that are currently within the editor's view.
// This is the viewer's text memory, not to be confused with Buffer's complete text memory.
// Lines out of range have an empty optional.
std::vector<std::optional<std::string>> linesInView;

// Displays desired text line from file in target row.
// Note that this method moves the cursor.
void displayLine(WINDOW* win, int fileLineNum, int displayRow, Buffer* b) {
    // Get line from buffer in memory
    std::optional<std::string> line = b->getLine(fileLineNum);    
    if (line.has_value()) {
        /// TODO: Limit number of characters according to COLS?
        // If this line is being displayed in last row of text edit region, strip
        // the newline when printing or else curses will shift lines up and add blank line.
        bool hasNewline = line->length() > 0 && line->back() == '\n';
        if (displayRow == LINES_TXT - 1 && hasNewline) {
            mvwaddstr(win, displayRow, 0,
                line->substr(0, line->length() - 1).c_str());
        } else {
            mvwaddstr(win, displayRow, 0, line->c_str());
        }
        
    }
    // Update memory with newly loaded line
    linesInView.insert(linesInView.begin() + displayRow, line);
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
        if (DEBUG)
            std::cout << debugLog.str();
        std::cout << msg << std::endl;
        return 0;
    }

    // Setup curses mode
    initscr();
    raw();
    noecho();

    // Create text edit region window
    WINDOW* txtW = newwin(LINES_TXT, COLS_TXT, 1, 0);
    keypad(txtW, TRUE);
    // Enable vertical scrolling
    scrollok(txtW, TRUE);
    // Scrolling region excludes header and footer rows
    /// TODO: Update on resize
    // setscrreg(1, LINES - 2);

    // Draw window chrome
    WINDOW* headW = newwin(1, COLS, 0, 0);
    wattron(headW, A_BOLD);
    wattron(headW, A_STANDOUT);
    waddstr(headW, "tekst by Baran Usluel\n");
    wnoutrefresh(headW);
    WINDOW* footW = newwin(1, COLS, LINES-1, 0);
    wattron(footW, A_BOLD);
    waddstr(footW, argv[1]);
    wnoutrefresh(footW);

    int scrollOffset = 0; // Amount text window was scrolled by (positive = downwards)
    int row = 0, col = 0; // Position of cursor in text window
    // Which column cursors wants to be on (for persistent
    // cursor position across varying line lengths)
    int colGoal = 0;

    // Display text from read file in visible rows
    for (int i = 0; i < LINES_TXT; ++i) {
        displayLine(txtW, i, i, b.get());
    }

    // Move cursor to start of file
    wmove(txtW, row, col);
    // wrefresh = wnoutrefresh + doupdate
    // When refreshing multiple windows, only doupdate once for efficiency
    wnoutrefresh(txtW);
    doupdate();

    // Whether program is terminated early due to an error
    bool err = false;

    // Input loop
    int ch = wgetch(txtW);
    while (ch != ctrl('c')) { // Exit code
        switch (ch) {
            /// TODO: Add page up/down key cases
            case KEY_BACKSPACE:
                // Special case if cursor at leftmost position
                if (col == 0) {
                    // If not first line of text, then move to delete EOL of previous line
                    if (row > 0) {
                        wmove(txtW, --row, colGoal = col = getCleanStrLen(linesInView[row - 1].value()));
                    } else {
                        break;
                    }
                } else {
                    // Otherwise move left, and flow into KEY_DC case below to delete char
                    wmove(txtW, row, colGoal = --col);
                }
                // No break
            case KEY_DC:
                b->delChar(row, col);
                wdelch(txtW);
                // If cursor is at end of line, deleting linebreak
                if (col == getCleanStrLen(linesInView[row].value())) {
                    curs_set(0); // Hide cursor during operations to avoid flickering
                    wdeleteln(txtW); // Deletes next row and shifts everything up
                    // Delete entry for current row from linesInView, it will be readded by displayLine
                    linesInView.erase(linesInView.begin() + row);
                    // Delete entry for the deleted row from linesInView
                    linesInView.erase(linesInView.begin() + row);
                    // Display updated (concatenated) line
                    displayLine(txtW, scrollOffset + row, row, b.get());
                    // Display line that scrolled into view from bottom
                    displayLine(txtW, LINES_TXT - 1 + scrollOffset, LINES_TXT - 1, b.get());
                    wmove(txtW, row, col);
                    curs_set(1);
                } else {
                    // Delete char from line in view memory
                    linesInView[row]->erase(col, 1);
                }
                break;
            case KEY_LEFT:
                wmove(txtW, row, colGoal = col = std::max(col - 1, 0));
                break;
            case KEY_RIGHT:
                wmove(txtW, row, colGoal = col = std::min(std::min(col + 1, COLS_TXT - 1), getCleanStrLen(linesInView[row].value())));
                break;
            case KEY_UP:
                if (row == 0) {
                    if (scrollOffset > 0) {
                        curs_set(0); // Hide cursor during operations to avoid flickering
                        wscrl(txtW, -1);
                        scrollOffset--;
                        // Delete entry from end of table because only tracking visible lines
                        linesInView.erase(linesInView.end() - 1);
                        // Load text to display in the new scrolled line.
                        displayLine(txtW, scrollOffset, row, b.get());
                    }
                } else {
                    row--;
                }
                wmove(txtW, row, col = std::min(colGoal, getCleanStrLen(linesInView[row].value())));
                curs_set(1);
                break;
            case KEY_DOWN:
                if (row == LINES_TXT - 1) {
                    // If there is no accessible next line, don't move cursor down.
                    // Can't check next line directly because not loaded into view memory yet,
                    // so instead check if there is newline at the end of this line.
                    if (linesInView[row]->back() != '\n')
                        break;
                    curs_set(0); // Hide cursor during operations to avoid flickering
                    wscrl(txtW, 1);
                    scrollOffset++;
                    // Delete entry from start of table because only tracking visible lines
                    linesInView.erase(linesInView.begin());
                    // Load text to display in the new scrolled line.
                    displayLine(txtW, LINES_TXT - 1 + scrollOffset, row, b.get());
                } else {
                    // If there is no accessible next line, don't move cursor down
                    if (!linesInView[row + 1].has_value())
                        break;
                    row++;
                }
                wmove(txtW, row, col = std::min(colGoal, getCleanStrLen(linesInView[row].value())));
                curs_set(1);
                break;
            case KEY_HOME:
                wmove(txtW, row, colGoal = col = 0);
                break;
            case KEY_END:
                wmove(txtW, row, colGoal = col = getCleanStrLen(linesInView[row].value()));
                break;
            case ctrl('s'):
                try {
                    b->save();
                } catch (std::string msg) {
                    err = true;
                    debugLog << msg << std::endl;
                    break;
                }
                break;
            default:
                winsch(txtW, ch); // Insert typed character on screen
                b->insertChar(ch, row, col); // Insert character in buffer
                if (ch == '\n') {
                    curs_set(0); // Hide cursor during operations to avoid flickering
                    // Current line is truncated to linebreak position in view memory
                    linesInView[row] = linesInView[row]->substr(0, col) + '\n';
                    // Move cursor to start of next row
                    wmove(txtW, ++row, colGoal = col = 0);
                    winsertln(txtW); // Add new empty line on screen above cursor, shifting rest down
                    displayLine(txtW, scrollOffset + row, row, b.get()); // Text to go in new line
                    wmove(txtW, row, col);
                    // Delete entry from end of table because only tracking visible lines
                    linesInView.erase(linesInView.end() - 1);
                    curs_set(1);
                } else {
                    // Insert new character into line in view memory
                    linesInView[row]->insert(linesInView[row]->begin() + col, ch);
                    // Move cursor right when character typed
                    wmove(txtW, row, colGoal = col = std::min(col + 1, COLS_TXT - 1));
                }
                
        }
        wrefresh(txtW);
        ch = wgetch(txtW);
    }

    delwin(headW);
    delwin(footW);
    delwin(txtW);

    endwin();
    if (DEBUG || err)
        std::cout << debugLog.str();

    return 0;
}