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
#define BIT9  1<<9
#define BIT8  1<<8
#define BIT7  1<<7
#define BIT6  1<<6
#define BIT5  1<<5
#define BIT4  1<<4
#define BIT3  1<<3
#define BIT2  1<<2
#define BIT1  1<<1
#define BIT0  1<<0

#define REGCONST 2
enum {REG = 0, CONST};

typedef enum {FALSE = 0, TRUE} bool;
typedef enum {FETCH, DECODE, EXECUTE} FDE_STATE;
typedef enum {NORMAL_MODE, CEX_MODE} CPU_MODE;
typedef enum {cexF,cexT} CEX_TF;

extern unsigned CEX_T_CNT;
extern unsigned CEX_F_CNT;
extern CEX_TF cexTF;
extern CPU_MODE cpuMode;

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

void interrupt(int vec_num);
// changes execution to interrupt vector

void updateScreen(void);
// print clock cycles to screen while running

void fetch(void);
// fetch instruction from memory into instruction register

void decode(void);
// decode instruction register

void execute(void);
// execute instruction

#endif
