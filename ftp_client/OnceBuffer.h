#include "header.h"
using std::string;

#pragma once
class OnceBuffer {
   public:
    OnceBuffer(size_t s = 128);
    OnceBuffer(const OnceBuffer &);
    ~OnceBuffer();
    operator char *() { return buf.get(); }

    int write(char *ch);
    string read();
    size_t length();

   private:
    unique_ptr<char> buf;
    size_t len;
};

typedef OnceBuffer ob;