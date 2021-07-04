#include "ArrayBuffer.h"

#include <sstream>
#include "Utils.h"

ArrayBuffer::ArrayBuffer(char* filename) {
    fileStream.open(filename);
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

    // Read file line by line and append to array.
    std::string line;
    while (getline(fileStream, line)) {
        fileMemory.insert(fileMemory.end(), line.begin(), line.end());
        fileMemory.push_back('\n'); // TODO: Preserve original EOL
    }
}

std::string ArrayBuffer::getLine(uint lineNum) {
    // Get n-th line from text file's representation in memory.
    // Since stored as a contiguous array, have to stream line by line
    // which is inefficient but is an intrinsic weakness of this buffer
    // implementation that would require added complexity to improve.
    std::istringstream memoryStream(fileMemory);
    std::string line;
    uint lineCnt = 0;
    do {
        getline(memoryStream, line);
    } while (lineCnt++ < lineNum && memoryStream.good());
    return line + (line.length() ? "\n" : ""); // TODO: Preserve original EOL
}