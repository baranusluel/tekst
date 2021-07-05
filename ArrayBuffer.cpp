#include "ArrayBuffer.h"

#include <fstream>
#include "Utils.h"

ArrayBuffer::ArrayBuffer(char* filename) {
    this->filename = filename;

    // Open file for reading
    std::ifstream fileStream(filename);
    // If such file exists, read it in. If not, is a new file
    if (fileStream.is_open()) {
        // Seek to EOF to get length
        fileStream.seekg(0, std::ios::end);
        int length = fileStream.tellg();
        fileStream.seekg(0, std::ios::beg);
        debugLog << "ArrayBuffer | Length is " << length << " bytes, allocating 2x capacity" << std::endl;

        // Contiguous array capacity initially twice file length so that
        // array doesn't immediately have to be copied after some insertions.
        // This will be unmanageably large for huge files.
        fileMemory.reserve(length * 2);

        // Read file line by line and append to array in memory.
        std::string line;
        while (getline(fileStream, line)) {
            fileMemory.insert(fileMemory.end(), line.begin(), line.end());
            // Remove carriage returns. tekst uses LF, not CRLF, for simplicity.
            if (fileMemory.back() == '\r')
                fileMemory.pop_back();
            fileMemory.push_back('\n'); // getline() removes original \n
        }

        fileStream.close();
    }
}

// Gets the start and end indices of a given line in the contiguous string,
// by counting delimiters.
void ArrayBuffer::getLineBounds(uint lineNum, size_t* beginP, size_t* endP) {
    uint lineCount = 0;
    size_t begin = 0;
    size_t end = 0;

    do {
        // If not first iteration, move `begin` pos to char after delimiter
        begin = end > 0 ? end + 1 : 0;
        // Search for the next delimiter starting from `begin`
        end = fileMemory.find('\n', begin);
        // No more delimiters
        if (end == std::string::npos) {
            // If desired line was after current line (from `begin` to npos),
            // no such line. Set begin to npos as a flag.
            if (lineCount < lineNum) {
                begin = std::string::npos;
            }
            break;
        }
    } while (lineCount++ < lineNum);

    *beginP = begin;
    *endP = end;
}

std::optional<std::string> ArrayBuffer::getLine(uint lineNum) {
    // Get n-th line from text file's representation in memory.
    // Since stored as a contiguous array, have to count delimiters
    // which is inefficient but is an intrinsic weakness of this naive
    // buffer implementation. Natural next step is array of arrays implementation.

    size_t begin, end;
    getLineBounds(lineNum, &begin, &end);
    if (begin == std::string::npos)
        return {};
    else
        return fileMemory.substr(begin, end - begin + 1);
}

// Writes to file from string in memory
void ArrayBuffer::save() {
    std::ofstream fileStream(filename, std::ofstream::trunc);
    if (!fileStream.is_open()) {
        throw std::string("Unable to write to file: ") + filename;
    }
    fileStream << fileMemory;
    fileStream.close();
}

void ArrayBuffer::delChar(int line, int col) {
    // Get bound indices of line to edit
    size_t begin, end;
    getLineBounds(line, &begin, &end);
    // No effect if out of range
    if (col > end - begin || begin + col >= fileMemory.length())
        return;
    size_t charPos = begin + col;
    // Delete single character at given position
    fileMemory.erase(charPos, 1);
}

void ArrayBuffer::insertChar(char c, int line, int col) {
    // Get bound indices of line to edit
    size_t begin, end;
    getLineBounds(line, &begin, &end);
    // No effect if out of range
    if (col > end - begin || begin + col > fileMemory.length())
        return;
    // Insert character, shifting everything afterwards
    fileMemory.insert(fileMemory.begin() + begin + col, c);
}