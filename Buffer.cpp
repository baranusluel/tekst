#include "Buffer.h"

#include "ArrayArrayBuffer.h"
#include "ArrayBuffer.h"

std::unique_ptr<Buffer> Buffer::createBuffer(BufferType type, char* filename) {
    switch (type) {
        case BufferType::ArrayBufferType:
            return std::make_unique<ArrayBuffer>(filename);
            break;
        case BufferType::ArrayArrayBufferType:
            return std::make_unique<ArrayArrayBuffer>(filename);
            break;
        // No default case so that type enum and switch statement synchronization
        // checked by compiler.
    }
}

std::string Buffer::bufferTypeToString(BufferType type) {
    switch (type) {
        case BufferType::ArrayBufferType:
            return "ArrayBuffer";
        case BufferType::ArrayArrayBufferType:
            return "ArrayArrayBuffer";
    }
}

BufferType Buffer::bufferTypeFromString(std::string type) {
    if (type == "ArrayBuffer") return BufferType::ArrayBufferType;
    if (type == "ArrayArrayBuffer") return BufferType::ArrayArrayBufferType;
    return BufferType::ArrayBufferType;
}