/* File name: loader.h
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Declarations for loader.c
 * Last Modified: 2019-07-25
 */

#ifndef LOADER_H
#define LOADER_H
#include <stdio.h>

enum { LOADER_FAIL, LOADER_SUCC };

int loader(FILE * fptr);

#endif
