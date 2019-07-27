#ifndef CPU_H
#define CPU_H

#define NEVER -1
#define NUM_REG 8
#define PC 7
#define SP 6
#define LR 5

#define RTN_FROM_ITR 0xFFFF

#define MEM_ACC_CLK 2

#define PC_INCR 2

// for decode:
#define BIT15 1<<15
#define BIT14 1<<14
#define BIT13 1<<13
#define BIT12 1<<12
#define BIT11 1<<11
#define BIT10 1<<10

#define REGCONST 2
enum {REG = 0, CONST};

typedef enum {FALSE = 0, TRUE} bool;
typedef enum {FETCH, DECODE, EXECUTE} FDE_STATE;
typedef enum {NORMAL, CEX} CPU_MODE;
typedef enum {cexF,cexT} CEX_TF;

extern int BRKPT;
extern unsigned short regFile[NUM_REG][REGCONST]; // registers and constants
extern unsigned short IR; // instruction register
extern unsigned short MAR; // memory address register
extern unsigned short MBR; // memory buffer register
extern int clock;
extern int step;

void (*function)(void); // global function pointer for execute

void initXM2(void);
// set initial values of globals

void FDE(void);
// fetch, decode, execute cycle

void pull(unsigned short * bucket);
// pulls from stack

void push(unsigned short bucket);
// pushes to stack

void updateScreen(void);
// print clock cycles to screen while running

void fetch(void);
// fetch instruction from memory into instruction register

void decode(void);
// decode instruction register

void execute(void);
// execute instruction

#endif
