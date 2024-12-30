#include "libtest.h"
#include <cstdio>

void __func(const char* s, int v) { std::fprintf(stderr, "%s -- %d \n", s, v); }


SDE_EXPORT void func(const char* s, int v) { __func(s, v); }
