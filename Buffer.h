#pragma once

#include <memory>
#include <string>

enum BufferType { ArrayBufferType };

class Buffer {
    public:
        virtual std::string getLine(uint lineNum) = 0;
        virtual void save() = 0;
        virtual void delChar(int line, int col) = 0;
        virtual void insertChar(char c, int line, int col) = 0;

        static std::unique_ptr<Buffer> createBuffer(BufferType, char* filename);
        static std::string bufferTypeToString(BufferType);

        std::string filename;
};