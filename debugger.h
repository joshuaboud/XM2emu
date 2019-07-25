#ifndef DEBUGGER_H
#define DEBUGGER_H

#define MEM_PRINT_WIDTH 16

typedef enum { INPUT, OOB, EVEN, FDNE, LOADER, INV_OPT, ENDSTART,
              CHKSUM } error_enum;

void debuggerMenu(void);
// main control of debugger

void printMenu(void);
// prints menu options for debugger

void changeMemory(void);
// modify memory location value

void breakpoint(void);
// set breakpoint

void load(void);
// get file name, call loader

void printRegFile(void);
// print register values

void setReg(void);
// modify register value

void reset(void);
// set PSW to reset vector PSW, set PC to reset vector PC

void printMem(void);
// print memory between two locations

void printPSW(void);
// print the current PSW

void error(error_enum e);
// print error and pause

#endif
