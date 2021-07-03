#pragma once

#include <stdint.h>
#include <memory>

enum BufferType { ArrayBufferType };

class Buffer {
    public:
        virtual int getLine(uint64_t lineNum, char** buffer, int bufferLen) = 0;
        static std::unique_ptr<Buffer> createBuffer(BufferType);
};