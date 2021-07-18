#include <algorithm>
#include <curses.h>
#include <iostream>
#include <optional>
#include <signal.h>
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
#define COLS_TXT COLS - 4

// Table of lines / strings that are currently within the editor's view.
// This is the viewer's text memory, not to be confused with Buffer's complete text memory.
// Lines out of range have an empty optional.
std::vector<std::optional<std::string>> linesInView;

// Curses windows for specific regions of the UI.
WINDOW* txtW; // Where textfile contents are displayed and edited
WINDOW* headW; // Top margin of UI
WINDOW* footW; // Bottom margin of UI
WINDOW* lineNumW; // Left margin of UI

// Displays desired text line from file in target row.
// Note that this method moves the cursor.
void displayLineFromBuffer(int fileLineNum, int displayRow, Buffer* b) {
    // Get line from buffer in memory
    std::optional<std::string> line = b->getLine(fileLineNum);    
    if (line.has_value()) {
        /// TODO: Limit number of characters according to COLS?
        // If this line is being displayed in last row of text edit region, strip
        // the newline when printing or else curses will shift lines up and add blank line.
        bool hasNewline = line->length() > 0 && line->back() == '\n';
        if (displayRow == LINES_TXT - 1 && hasNewline) {
            mvwaddstr(txtW, displayRow, 0,
                line->substr(0, line->length() - 1).c_str());
        } else {
            mvwaddstr(txtW, displayRow, 0, line->c_str());
        }
        
    }
    // Update memory with newly loaded line
    linesInView.insert(linesInView.begin() + displayRow, line);
}

/// TODO: Handle inserting & deleting EOL cases with this method
/// instead of above for efficiency
void displayLineFromCache(int row) {
    // Get line from view memory
    if (linesInView[row].has_value()) {
        mvwaddstr(txtW, row, 0, linesInView[row]->c_str());
    }
}

void initDraw(Buffer* b, int scrollOffset) {
    waddstr(headW, "tekst by Baran Usluel\n");
    wnoutrefresh(headW);

    waddstr(footW, b->filename.c_str());
    wnoutrefresh(footW);

    for (int i = 0; i < LINES_TXT; ++i) {
        wmove(lineNumW, i, 0);
        wprintw(lineNumW, "%u", i + 1 + scrollOffset);
    }
    wnoutrefresh(lineNumW);

    // Display text from read file in visible rows
    for (int i = 0; i < LINES_TXT; ++i) {
        displayLineFromBuffer(i + scrollOffset, i, b);
    }
    // wrefresh = wnoutrefresh + doupdate
    // When refreshing multiple windows, only doupdate once for efficiency
    wnoutrefresh(txtW);
    doupdate();
}

/// TODO: Optimize this by not fully clearing and redrawing
void handleResize(Buffer* b, int scrollOffset) {
    // Clear all windows
    wclear(txtW);
    wclear(headW);
    wclear(footW);
    wclear(lineNumW);
    // Resize all windows in memory
    wresize(txtW, LINES_TXT, COLS_TXT);
    wresize(headW, 1, COLS);
    wresize(footW, 1, COLS);
    wresize(lineNumW, LINES_TXT, 4);
    // Move footer
    mvwin(footW, LINES - 1, 0);
    // Redraw all contents
    initDraw(b, scrollOffset);
}

void scrollLineNumsUp(const int& scrollOffset) {
    wscrl(lineNumW, 1);
    wmove(lineNumW, LINES_TXT - 1, 0);
    wprintw(lineNumW, "%u", LINES_TXT + scrollOffset);
    wnoutrefresh(lineNumW);
}

void scrollLineNumsDown(const int& scrollOffset) {
    wscrl(lineNumW, -1);
    wmove(lineNumW, 0, 0);
    wprintw(lineNumW, "%u", scrollOffset + 1);
    wnoutrefresh(lineNumW);
}

// Moves lines in text view up due to user scrolling downwards
void scrollTextViewUp(int& scrollOffset, Buffer* b, bool loadBottomLine) {
    curs_set(0); // Hide cursor during operations to avoid flickering
    wscrl(txtW, 1);
    scrollOffset++;
    // Delete entry from start of table because only tracking visible lines
    linesInView.erase(linesInView.begin());
    // Load text to display in the new scrolled line.
    if (loadBottomLine)
        displayLineFromBuffer(LINES_TXT - 1 + scrollOffset, LINES_TXT - 1, b);
    scrollLineNumsUp(scrollOffset);
    curs_set(1);
}

// Moves lines in text view down due to user scrolling upwards
void scrollTextViewDown(int& scrollOffset, Buffer* b, bool loadTopLine) {
    curs_set(0); // Hide cursor during operations to avoid flickering
    wscrl(txtW, -1);
    scrollOffset--;
    // Delete entry from end of table because only tracking visible lines
    linesInView.erase(linesInView.end() - 1);
    // Load text to display in the new scrolled line.
    if (loadTopLine)
        displayLineFromBuffer(scrollOffset, 0, b);
    scrollLineNumsDown(scrollOffset);
    curs_set(1);
}

bool moveCursorUp(int& scrollOffset, Buffer* b, int& row, int& col, int& colGoal) {
    if (row == 0) {
        // If no more lines above, stop
        if (scrollOffset <= 0)
            return false;
        scrollTextViewDown(scrollOffset, b, true);
    } else {
        row--;
    }
    wmove(txtW, row, col = std::min(colGoal, getCleanStrLen(linesInView[row].value())));
    return true;
}

bool moveCursorDown(int& scrollOffset, Buffer* b, int& row, int& col, int& colGoal) {
    if (row == LINES_TXT - 1) {
        // If there is no accessible next line, don't move cursor down.
        // Can't check next line directly because not loaded into view memory yet,
        // so instead check if there is newline at the end of this line.
        if (linesInView[row]->back() != '\n')
            return false;
        scrollTextViewUp(scrollOffset, b, true);
    } else {
        // If there is no accessible next line, don't move cursor down
        if (!linesInView[row + 1].has_value())
            return false;
        row++;
    }
    wmove(txtW, row, col = std::min(colGoal, getCleanStrLen(linesInView[row].value())));
    return true;
}

bool moveCursorLeft(int& scrollOffset, Buffer* b, int& row, int& col, int& colGoal) {
    // If at start of line and moving left, try to go to end of previous line
    if (col == 0) {
        // If move up success, then go to end
        if (moveCursorUp(scrollOffset, b, row, col, colGoal))
            wmove(txtW, row, colGoal = col = getCleanStrLen(linesInView[row].value()));
        else
            return false;
    } else
        wmove(txtW, row, colGoal = --col);
    return true;
}

bool moveCursorRight(int& scrollOffset, Buffer* b, int& row, int& col, int& colGoal) {
    // If at end of line and moving right, try to go to start of next line
    if (col == getCleanStrLen(linesInView[row].value())) {
        // If move down success, then go to start
        if (moveCursorDown(scrollOffset, b, row, col, colGoal))
            wmove(txtW, row, colGoal = col = 0);
        else
            return false;
    } else
        wmove(txtW, row, colGoal = col = std::min(col + 1, COLS_TXT - 1));
    return true;
}

void delCharAtCursor(int& scrollOffset, Buffer* b, int& row, int& col) {
    b->delChar(row + scrollOffset, col);
    wdelch(txtW);
    // If cursor is at end of line, deleting linebreak
    if (col == getCleanStrLen(linesInView[row].value())) {
        curs_set(0); // Hide cursor during operations to avoid flickering
        wdeleteln(txtW); // Deletes next row and shifts everything up
        // Delete entry for current row from linesInView, it will be readded by displayLineFromBuffer
        linesInView.erase(linesInView.begin() + row);
        // Delete entry for the deleted row from linesInView
        linesInView.erase(linesInView.begin() + row);
        // Display updated (concatenated) line
        displayLineFromBuffer(scrollOffset + row, row, b);
        // Display line that scrolled into view from bottom
        displayLineFromBuffer(LINES_TXT - 1 + scrollOffset, LINES_TXT - 1, b);
        wmove(txtW, row, col);
        curs_set(1);
    } else {
        // Delete char from line in view memory
        linesInView[row]->erase(col, 1);
    }
}

void insertCharAtCursor(int& scrollOffset, Buffer* b, int& row, int& col, int& colGoal, int ch) {
    /// CONSIDER: Splitting up this function for \n and other chars
    winsch(txtW, ch); // Insert typed character on screen
    b->insertChar(ch, row + scrollOffset, col); // Insert character in buffer
    if (ch == '\n') {
        curs_set(0); // Hide cursor during operations to avoid flickering
        // Current line is truncated to linebreak position in view memory
        linesInView[row] = linesInView[row]->substr(0, col) + '\n';
        // If view has to scroll due to new line
        if (row == LINES_TXT - 1) {
            // Not directly scrolling text view here because it happens automatically
            // when \n char printed by winsch().
            scrollOffset++;
            // Delete entry from start of table because only tracking visible lines
            linesInView.erase(linesInView.begin());
            // Explicitly scroll line numbers
            scrollLineNumsUp(scrollOffset);
        } else {
            wmove(txtW, ++row, col); // Move to next row before inserting new line
            winsertln(txtW); // Add new empty line on screen above cursor, shifting rest down
            // Delete entry from end of table because only tracking visible lines
            linesInView.erase(linesInView.end() - 1);
        }
        displayLineFromBuffer(scrollOffset + row, row, b); // Text to go in new line
        // Move cursor to start of new line
        wmove(txtW, row, colGoal = col = 0);
        curs_set(1);
    } else {
        // Insert new character into line in view memory
        linesInView[row]->insert(linesInView[row]->begin() + col, ch);
        // Move cursor right when character typed
        wmove(txtW, row, colGoal = col = std::min(col + 1, COLS_TXT - 1));
    }
}

int main(int argc, char* argv[]) {
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

    // Initialize text edit region window
    txtW = newwin(LINES_TXT, COLS_TXT, 1, 4);
    keypad(txtW, TRUE);
    // Enable vertical scrolling
    scrollok(txtW, TRUE);

    // Initialize header & footer windows
    headW = newwin(1, COLS, 0, 0);
    wattron(headW, A_BOLD);
    wattron(headW, A_STANDOUT);
    footW = newwin(1, COLS, LINES-1, 0);
    wattron(footW, A_BOLD);

    // Initialize line number window
    lineNumW = newwin(LINES_TXT, 4, 1, 0);
    scrollok(lineNumW, TRUE);
    wattron(lineNumW, A_DIM);

    int scrollOffset = 0; // Amount text window was scrolled by (positive = downwards)
    int row = 0, col = 0; // Position of cursor in text window
    // Which column cursors wants to be on (for persistent
    // cursor position across varying line lengths)
    int colGoal = 0;

    // Draw contents of all windows
    initDraw(b.get(), scrollOffset);

    // Move cursor to start of file
    wmove(txtW, row, col);

    // Whether program is terminated early due to an error
    bool err = false;

    // Input loop
    int ch = wgetch(txtW);
    while (ch != ctrl('c') && !err) { // Exit code
        switch (ch) {
            /// TODO: Add page up/down key cases
            case KEY_BACKSPACE:
                // If able to move left (or up), do it and delete char
                if (moveCursorLeft(scrollOffset, b.get(), row, col, colGoal))
                    delCharAtCursor(scrollOffset, b.get(), row, col);
                break;
            case KEY_DC:
                delCharAtCursor(scrollOffset, b.get(), row, col);
                break;
            case KEY_LEFT:
                moveCursorLeft(scrollOffset, b.get(), row, col, colGoal);
                break;
            case KEY_RIGHT:
                moveCursorRight(scrollOffset, b.get(), row, col, colGoal);
                break;
            case KEY_UP:
                moveCursorUp(scrollOffset, b.get(), row, col, colGoal);
                break;
            case KEY_DOWN:
                moveCursorDown(scrollOffset, b.get(), row, col, colGoal);
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
                }
                break;
            case KEY_RESIZE:
                handleResize(b.get(), scrollOffset);
                wmove(txtW, row, col);
                break;
            default:
                insertCharAtCursor(scrollOffset, b.get(), row, col, colGoal, ch);
        }
        wrefresh(txtW);
        ch = wgetch(txtW);
    }

    delwin(headW);
    delwin(footW);
    delwin(txtW);
    delwin(lineNumW);

    endwin();
    if (DEBUG || err)
        std::cout << debugLog.str();

    return 0;
}