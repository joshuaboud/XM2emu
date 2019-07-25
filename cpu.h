#ifndef CPU_H
#define CPU_H

#define NEVER -1
#define NUM_REG 8
#define PC 7
#define SP 6
#define LR 5

typedef enum {FALSE = 0, TRUE} bool;

extern int BRKPT;
extern unsigned short regFile[NUM_REG];
extern int clock;

void FDE(void);
// fetch, decode, execute cycle

void updateScreen(void);
// print clock cycles to screen while running

void fetch();
// fetch instruction from memory into instruction register

void decode();
// decode instruction register

void execute();
// execute instruction

#endif
