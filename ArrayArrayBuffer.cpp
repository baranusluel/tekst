#include "ArrayArrayBuffer.h"

#include <fstream>
#include "Utils.h"

ArrayArrayBuffer::ArrayArrayBuffer(char* filename) {
    this->filename = filename;

    // Open file for reading
    std::ifstream fileStream(filename);
    // If such file exists, read it in. If not, is a new file
    if (fileStream.is_open()) {
        // Read file line by line and insert into array in memory.
        std::string line;
        while (getline(fileStream, line)) {
            // Remove carriage returns. tekst uses LF, not CRLF, for simplicity.
            if (line.back() == '\r')
                line.pop_back();
            line.push_back('\n'); // getline() removes original \n
            fileMemory.push_back(line);
        }
        fileMemory.push_back(""); // Should always be an empty editable line, even if no newlines

        fileStream.close();
    }
}

std::optional<std::string> ArrayArrayBuffer::getLine(uint lineNum) {
    if (lineNum < fileMemory.size())
        return fileMemory[lineNum];
    else
        return {};
}

// Writes to file from string in memory
void ArrayArrayBuffer::save() {
    std::ofstream fileStream(filename, std::ofstream::trunc);
    if (!fileStream.is_open()) {
        throw std::string("Unable to write to file: ") + filename;
    }
    for (int i = 0; i < fileMemory.size(); i++) {
        fileStream << fileMemory[i];
    }
    fileStream.close();
}

void ArrayArrayBuffer::delChar(int line, int col) {
    // No effect if out of range
    if (line >= fileMemory.size() || col >= fileMemory[line].length())
        return;
    // Check whether char to delete is a newline
    if (fileMemory[line][col] == '\n') {
        // Get next line, if it exists
        std::string nextLine = "";
        if (line + 1 < fileMemory.size())
            nextLine = fileMemory[line + 1];
        // Delete the newline char
        fileMemory[line].erase(col, 1);
        // Append next line into current line
        fileMemory[line].append(nextLine);
        // Delete next line
        fileMemory.erase(fileMemory.begin() + line + 1);
    } else {
        // Delete single character at given position
        fileMemory[line].erase(col, 1);
    }
}

void ArrayArrayBuffer::insertChar(char c, int line, int col) {
    // No effect if out of range
    if (line >= fileMemory.size() || col > fileMemory[line].length())
        return;
    // Check whether char to insert is a newline
    if (c == '\n') {
        // Get rest of line after newline character positoin
        std::string restOfLine = fileMemory[line].substr(col, std::string::npos);
        // Delete rest of line and add newline char
        fileMemory[line].erase(col, std::string::npos);
        fileMemory[line].push_back('\n');
        // Insert new line
        fileMemory.insert(fileMemory.begin() + line + 1, restOfLine);
    } else {
        // Insert character, shifting everything afterwards
        fileMemory[line].insert(fileMemory[line].begin() + col, c);
    }
}