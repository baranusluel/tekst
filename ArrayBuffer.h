/*
 * ArrayBuffer is the most naive text buffer implementation, in which the data
 * is stored as a single contiguous array in memory (std::string).
 */

#pragma once

#include <vector>
#include "Buffer.h"

class ArrayBuffer : public Buffer {
    public:
        ArrayBuffer(char* filename);
        std::string getLine(uint lineNum);
        void save();
        void delChar(int line, int col);
        void insertChar(char c, int line, int col);
        uint getNumLines();
    private:
        // ArrayBuffer stores all the text as a managed array / vector / ArrayList / std::string
        std::string fileMemory;
        // Gets the start and end indices of a given line in the contiguous string,
        // by counting delimiters.
        void getLineBounds(uint lineNum, size_t* beginP, size_t* endP);
        // Local counter of number of lines. Should be one more than number of linebreaks
        uint numLines = 1;
};