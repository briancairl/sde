#include "libtest.h"
#include <cstdio>

void func(const char* s, int v) { std::fprintf(stderr, "%s -- %d \n", s, v); }