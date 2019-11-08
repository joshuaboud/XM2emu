/* File name: cpu.c
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Provide defitions of the machine's global registers (IR,
 * MAR, MBR), CPU state variables, and defines the SIG_INT handler, 
 * FDE cycle.
 * Last Modified: 2019-07-27
 */

#include "cpu.h"
#include "memory.h"
#include "execute.h"
#include <ncurses.h>
#include <stdlib.h>
#include <signal.h> /* Signal handling software */

unsigned short regFile[NUM_REG][REGCONST] = {0,0,
                                             0,1,
                                             0,2,
                                             0,4,
                                             0,8,
                                             0,16,
                                             0,32,
                                             0,-1};

unsigned short IR = 0; // instruction register
unsigned short MAR = 0; // memory address register
unsigned short MBR = 0; // memory buffer register

bool killed;
int BRKPT = NEVER; // default to run forever

FDE_STATE fdeState;
CPU_MODE cpuMode;

unsigned CEX_T_CNT;
unsigned CEX_F_CNT;
CEX_TF cexTF;

int clock = 0;

int step = FALSE;

void sigint_hdlr()
{
	/*
	- Invoked when SIGINT (control-C) is detected
	- changes state of waiting_for_signal
	- signal must be reinitialized
	*/
	killed = TRUE;
	signal(SIGINT, sigint_hdlr); /* Reinitialize SIGINT */
}

void FDE(){
  curs_set(0); // cursor is annoying during step, this makes it invisible
  initXM2();
  signal(SIGINT, sigint_hdlr);
  while(!killed && ((int)regFile[PC][REG] & WORD_MSK) != BRKPT){
    updateScreen();
    switch(fdeState){
    case FETCH:
      if(step) getch(); // step execution
      fetch();
      break;
    case DECODE:
      decode();
      break;
    case EXECUTE:
      execute();
      break;
    }
  }
  curs_set(1);
}

void initXM2(){
  fdeState = FETCH;
  CEX_T_CNT = 0;
  CEX_F_CNT = 0;
  cexTF = cexT;
  cpuMode = NORMAL_MODE;
  killed = FALSE;
}

void updateScreen(){
  erase(); //clear();
  printw("Running. # of clock cycles: %d\n",clock);
  printw("\nBlinkenlights:\n");
  for(int i = 0; i < NUM_REG; i++){
    printw("R%d: %04X\n", i, regFile[i][REG]);
  }
  printw("\nPSW: %04X",PSW->word);
  refresh();
}

void pull(unsigned short * bucket){
  regFile[SP][REG] += WORD_MEM_WIDTH;
  MAR = regFile[SP][REG];
  /**bucket = memory.word_mem[regFile[SP][REG]>>1];
  if(regFile[SP][REG] < VECTORBASE)
    clock += MEM_ACC_CLK;*/
  bus(MAR,&MBR,RD,W);
  *bucket = MBR;
}

void push(unsigned short bucket){
  /**memory.word_mem[regFile[SP][REG]>>1] = bucket;
  if(regFile[SP][REG] < VECTORBASE)
    clock += 2;*/
  MAR = regFile[SP][REG];
  MBR = bucket;
  bus(MAR,&MBR,WR,W);
  regFile[SP][REG] -= WORD_MEM_WIDTH;
}

void exception(unsigned vec_num){
  if(vectorTbl[vec_num].PSW.psw.CURR_PRIO <= PSW->psw.CURR_PRIO){
    // priority not high enough, return
    return;
  }
  push(regFile[PC][REG]);
  push(regFile[LR][REG]);
  push(PSW->word);
  // update PC to vector address
  regFile[PC][REG] = vectorTbl[vec_num].ADDR;
  // store prev priority
  int prev_prio = PSW->psw.CURR_PRIO;
  // update PSW to vector PSW
  PSW->word = vectorTbl[vec_num].PSW.word;
  // store previous priority
  PSW->psw.PREV_PRIO = prev_prio;
  PSW->psw.SLP = 0;
  // LR = 0xFFFF
  regFile[LR][REG] = RTN_FROM_ITR;
}

void fetch(){
  clock++;
  if(regFile[PC][REG] %2 != 0){ // PC is odd
    if(regFile[PC][REG] == RTN_FROM_ITR){
      // return from interrupt
      pull(&PSW->word);
      pull(&regFile[LR][REG]);
      pull(&regFile[PC][REG]);
    }else{
      // fault
      exception(INV_ADDR);
    }
    fdeState = FETCH;
    return;
  }
  
  MAR = regFile[PC][REG];
  bus(MAR, &MBR, RD, W);
  IR = MBR;
  
  regFile[PC][REG] += PC_INCR;
  
  if(cpuMode == CEX_MODE){
    if(CEX_T_CNT > 0){
      CEX_T_CNT--;
      if(cexTF == cexT){
        fdeState = DECODE;
        return;
      }
    }else if(CEX_F_CNT > 0){
      CEX_F_CNT--;
      if(cexTF == cexF){
        fdeState = DECODE;
        return;
      }
    }else{
      // both are 0
      cpuMode = NORMAL_MODE;
      fdeState = DECODE;
      return;
    }
    // skip decode if not to be executed
    fdeState = FETCH;
    return;
  }
  fdeState = DECODE;
}

void decode(){
  clock++;
  if(IR & BIT15){
    // bit 15 set
    function = relMem;
    fdeState = EXECUTE;
    return;
  }
  // opcode must be 0xxxxxxxxxxxxxxx
  if(!(IR & BIT14)){
    // bit 14 clear
    function = branching;
    fdeState = EXECUTE;
    return;
  }
  // opcode must be 01xxxxxxxxxxxxxx
  if(IR & BIT13){
    function = regInit;
    fdeState = EXECUTE;
    return;
  }
  // opcode must be 010xxxxxxxxxxxxx
  if(IR & BIT12){
    if(IR & BIT11){
      function = control;
      fdeState = EXECUTE;
      return;
    }
    // opcode must be 01010xxxxxxxxxxx
    function = mem;
    fdeState = EXECUTE;
    return;
  }
  // opcode must be 0100xxxxxxxxxxxx
  if(IR & BIT11){
    if(IR & BIT10){
      // if bits 9:8:7 form 1, 3, or 5 then ILL_INST
      function = shifting;
      fdeState = EXECUTE;
      return;
    }
    // opcode must be 010010xxxxxxxxxx
    function = bitOp;
    fdeState = EXECUTE;
    return;
  }
  // opcode must be 01000xxxxxxxxxxx
  if(IR & BIT10){
    function = ALUtest;
    fdeState = EXECUTE;
    return;
  }
  // opcode must be 010000xxxxxxxxxx
  function = ALU;
  fdeState = EXECUTE;
}

void execute(){
  clock++;
  function(); // calls func ptr pointing to func in execute.c
  fdeState = FETCH;
}
