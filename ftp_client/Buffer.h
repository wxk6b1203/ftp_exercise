#include "header.h"

using std::string;
using std::unique_ptr;

#pragma once

namespace local {
class Buffer {
   private:
    unique_ptr<char> buf;
    int len;

   public:
    Buffer(size_t s = 128);
    ~Buffer();

    int Write(char*, int);
    string ReadString();
    unique_ptr<char> Read();
    void Flush();
};

typedef Buffer bf;

}  // namespace local
