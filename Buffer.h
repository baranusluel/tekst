/*
 * Buffer is a common interface for all the text buffer implementations.
 * It defines the essential methods that the editor's frontend needs
 * to get and edit text from a file.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>

enum BufferType { ArrayBufferType, ArrayArrayBufferType };

class Buffer {
    public:
        virtual std::optional<std::string> getLine(uint lineNum) = 0;
        virtual void save() = 0;
        virtual void delChar(int line, int col) = 0;
        virtual void insertChar(char c, int line, int col) = 0;

        // Static factory method for instantiating Buffer objects
        static std::unique_ptr<Buffer> createBuffer(BufferType, char* filename);
        static std::string bufferTypeToString(BufferType);
        static BufferType bufferTypeFromString(std::string);

        // Name of file the buffer uses.
        // Non-const because this could change in a save-as.
        std::string filename;
};