
#include "OnceBuffer.h"

OnceBuffer::OnceBuffer(size_t s) : len(s), buf(new char[s]) {
    memset(buf.get(), 0, sizeof(char) * len);
}

OnceBuffer::~OnceBuffer() {}

size_t OnceBuffer::length() { return len; }

int OnceBuffer::write(char* ch) {
    memset(buf.get(), 0, sizeof(char) * len);
    memcpy(buf.get(), ch, int(len));
    if (strlen(ch) > len) {
        return -1;
    }
    return 1;
}

std::string OnceBuffer::read() {
    std::string s = buf.get();

    memset(buf.get(), 0, sizeof(char) * len);
    return s;
}

OnceBuffer::OnceBuffer(const OnceBuffer& a) : buf(new char[a.len]), len(a.len) {
    memcpy(buf.get(), a.buf.get(), sizeof(char) * len);
}