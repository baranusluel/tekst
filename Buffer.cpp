#include "Buffer.h"
#include "ArrayBuffer.h"

std::unique_ptr<Buffer> Buffer::createBuffer(BufferType type) {
    switch (type) {
        case BufferType::ArrayBufferType:
            return std::make_unique<ArrayBuffer>();
            break;
        // No default case so that type enum and switch statement are kept synced.
    }
}