#ifndef LOADER_H
#define LOADER_H
#include <stdio.h>

enum { LOADER_FAIL, LOADER_SUCC };

int loader(FILE * fptr);

#endif
