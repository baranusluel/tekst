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
    private:
        std::string fileMemory;
};