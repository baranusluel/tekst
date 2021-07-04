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
    private:
        std::string fileMemory;
        void getLineBounds(uint lineNum, size_t* beginP, size_t* endP);
};