#include "main.h"
#include "FrontEnd.h"
#include "header.h"

int argc;
char **argv;

int main(int argc1, char **argv1) {
    argc = argc1;
    argv = argv1;
    f::Impl().Start();
    return 0;
}
