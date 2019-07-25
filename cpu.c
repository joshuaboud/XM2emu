#include "cpu.h"
#include "memory.h"
#include <ncurses.h>
#include <stdlib.h>
#include <signal.h> /* Signal handling software */

unsigned short regFile[NUM_REG] = {0};

unsigned short IR = 0; // instruction register
unsigned short MAR = 0; // memory address register
unsigned short MBR = 0; // memory buffer register

bool killed;
int BRKPT = NEVER; // default to run forever

int clock = 0;

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
  killed = FALSE;
  signal(SIGINT, sigint_hdlr);
  while(!killed && regFile[PC] != (unsigned short)BRKPT){
    updateScreen();
    fetch();
    clock++;
    decode();
    clock++;
    execute();
    clock++;
  }
}

void updateScreen(){
  clear();
  printw("Running. # of clock cycles: %d",clock);
  refresh();
}

void fetch(){
  clock++;
  MAR = regFile[PC];
  bus(MAR, &MBR, RD, W);
  IR = MBR;
}

void decode(){
  clock++;
}

void execute(){
  clock++;
}
