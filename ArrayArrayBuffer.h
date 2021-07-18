/*
 * ArrayArrayBuffer is the second most simple text buffer implementation,
 * in which data is stored as an array of lines / strings.
 */

#pragma once

#include <vector>
#include "Buffer.h"

class ArrayArrayBuffer : public Buffer {
    public:
        ArrayArrayBuffer(char* filename);
        std::optional<std::string> getLine(uint lineNum);
        void save();
        void delChar(int line, int col);
        void insertChar(char c, int line, int col);
    private:
        // ArrayArrayBuffer stores the text as an array of strings (managed 2D array)
        std::vector<std::string> fileMemory;
};