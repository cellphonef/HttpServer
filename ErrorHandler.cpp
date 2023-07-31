#include "ErrorHandler.h"

#include <cstdio>
#include <cstdlib>

void errSys(const char* x) {
    perror(x);
    exit(1);
}