#include "Utils.h"

int getCleanStrLen(const std::string& s) {
    int len = s.length();
    if (len == 0)
        return 0;
    if (s.back() == '\n')
        return len - 1;
    return len;
}