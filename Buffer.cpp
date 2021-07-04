#include "Buffer.h"

#include "ArrayBuffer.h"

std::unique_ptr<Buffer> Buffer::createBuffer(BufferType type, char* filename) {
    switch (type) {
        case BufferType::ArrayBufferType:
            return std::make_unique<ArrayBuffer>(filename);
            break;
        // No default case so that type enum and switch statement synchronization
        // checked by compiler.
    }
}

std::string Buffer::bufferTypeToString(BufferType type) {
    switch (type) {
        case BufferType::ArrayBufferType:
            return "ArrayBuffer";
    }
}