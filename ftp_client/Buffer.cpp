#include "Buffer.h"
using std::move;
local::Buffer::Buffer(size_t s) : len(s), buf(new char[s]) {
    memset(buf.get(), 0, sizeof(char) * len);
}

string local::Buffer::ReadString() {
    string a = buf.get();
    return a;
}

unique_ptr<char> local::Buffer::Read() {
    unique_ptr<char> uchstring(new char[len]);
    memcpy(uchstring.get(), buf.get(), len);
    return move(uchstring);
}

void local::Buffer::Flush() { memset(buf.get(), 0, sizeof(char) * len); }

int local::Buffer::Write(char* s, int length) {
    memset(buf.get(), 0, sizeof(char) * len);
    int stt = 0;

    if (length < len)
    /**
     * status code:
     * > 0 : bytes copied;
     * = 0 : all in;
     * < 0 : error;
     */
    {
        stt = length;
    }
    int min = length > len ? len : length;
    memset(buf.get(), 0, sizeof(char) * len);
    memcpy(buf.get(), s, sizeof(char) * len);

    return stt;
}

local::Buffer::~Buffer() {}