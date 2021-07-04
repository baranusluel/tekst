#include "ArrayBuffer.h"

#include <fstream>
#include <sstream>
#include "Utils.h"

ArrayBuffer::ArrayBuffer(char* filename) {
    this->filename = filename;

    // Open file for reading
    std::ifstream fileStream(filename);
    if (!fileStream.is_open()) {
        throw std::string("Unable to open file: ") + filename;
    }

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
        fileMemory.push_back('\n'); // TODO: Preserve original EOL
    }

    fileStream.close();
}

void ArrayBuffer::getLineBounds(uint lineNum, size_t* beginP, size_t* endP) {
    uint lineCount = 0;
    size_t begin = 0;
    size_t end = 0;

    do {
        begin = end > 0 ? end + 1 : 0;
        end = fileMemory.find('\n', begin);
        if (end == std::string::npos) {
            if (lineCount < lineNum) {
                begin = fileMemory.length();
            }
            break;
        }
    } while (lineCount++ < lineNum);

    *beginP = begin;
    *endP = end;
}

std::string ArrayBuffer::getLine(uint lineNum) {
    // Get n-th line from text file's representation in memory.
    // Since stored as a contiguous array, have to stream line by line
    // which is inefficient but is an intrinsic weakness of this buffer
    // implementation that would require added complexity to improve.

    size_t begin, end;
    getLineBounds(lineNum, &begin, &end);
    return fileMemory.substr(begin, end - begin + 1);

    // std::istringstream memoryStream(fileMemory);
    // std::string line;
    // uint lineCnt = 0;
    // do {
    //     getline(memoryStream, line);
    // } while (lineCnt++ < lineNum && memoryStream.good());
    // return line + (line.length() ? "\n" : ""); // TODO: Preserve original EOL
}

void ArrayBuffer::save() {
    std::ofstream fileStream(filename, std::ofstream::trunc);
    if (!fileStream.is_open()) {
        throw std::string("Unable to write to file: ") + filename;
    }
    fileStream << fileMemory;
    fileStream.close();
}

void ArrayBuffer::delChar(int line, int col) {
    // TODO: Check bounds
    size_t begin, end;
    getLineBounds(line, &begin, &end);
    if (col > end - begin)
        return;
    fileMemory.erase(begin + col, 1);
}

void ArrayBuffer::insertChar(char c, int line, int col) {
    // TODO: Check bounds
    size_t begin, end;
    getLineBounds(line, &begin, &end);
    if (col > end - begin)
        return;
    fileMemory.insert(fileMemory.begin() + begin + col, c);
}