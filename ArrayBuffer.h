#pragma once

#include "Buffer.h"

class ArrayBuffer : public Buffer {
    public:
        int getLine(uint64_t lineNum, char** buffer, int bufferLen);
};