#include "debugger.h"
#include "memory.h"
#include "cpu.h"
#include "loader.h"
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#define NUM_ERR 8
char *errors[NUM_ERR] = { "Input error!",
                          "Input out of bounds!",
                          "Input must be even!",
                          "File does not exist!",
                          "Loader error!",
                          "Invalid option!",
                          "Start must come before end!",
                          "Chekcsum error!" };

void debuggerMenu(){
  char ch;
  while(ch != 'Q' && ch != 'q'){
    printMenu();
    ch = getch();
    // process choice
    switch(ch){
    case 'C':
    case 'c':
      changeMemory();
      break;
    case 'B':
    case 'b':
      breakpoint();
      break;
    case 'L':
    case 'l':
      load();
      break;
    case 'R':
    case 'r':
      printRegFile();
      break;
    case 'S':
    case 's':
      setReg();
      break;
    case 'T':
    case 't':
      reset();
      break;
    case 'M':
    case 'm':
      printMem();
      break;
    case 'P':
    case 'p':
      printPSW();
      break;
    case 'G':
    case 'g':
      FDE(); // calls FDE() in cpu.c
      break;
    case 'Q':
    case 'q':
      break;
    default:
      error(INV_OPT);
      break;
    }
  }
}

void printMenu(){
  clear();
  printw("XM2 Emulator 0.1 Josh Boudreau 2019\n");
  printw("PC = %04x\n",regFile[PC]);
  printw("# of clock cycles: %d",clock);
  (BRKPT != NEVER)? printw("Current breakpoint: %04x\n",BRKPT) : addch('\n');
  printw("C - Change memory location\n"
         "B - Set Breakpoint\n"
         "L - Load XME file\n"
         "R - Print Register File\n"
         "S - Set Register Value\n"
         "T - Reset Machine through Reset Vector\n"
         "M - Print Memory Contents at Specified Location(s)\n"
         "P - Print Program Status Word\n"
         "G - Go (start execution)\n"
         "Q - Quit\n\n"
         "Choice: ");
  refresh();
}

void changeMemory(){
  int loc, val;
  //unsigned short loc;
  printw("\nModify word or byte? [W/b]: ");
  char choice = getch();
  switch(choice){
  case 'B':
  case 'b':
  {
    printw("\nEnter memory location to modify [0000..FFFF]: ");
    if(scanw("%x ",&loc) == EOF){
      error(INPUT);
      return;
    }
    if(loc < 0 || loc >= BYTEMAXMEM){
      error(OOB);
      return;
    }
    printw("Enter byte to store at %04x (hex): ", loc);
    if(scanw("%x ",&val) == EOF){
      error(INPUT);
      return;
    }
    memory.byte_mem[(unsigned short)loc] = (unsigned char)val;
    break;
  }
  default: // default to word
  {
    printw("\nEnter memory location to modify [0000..FFFE]: ");
    if(scanw("%x ",&loc) == EOF){
      error(INPUT);
      return;
    }
    if(loc%2 != 0) loc -= 1;
    if(loc < 0 || loc >= BYTEMAXMEM){
      error(OOB);
      return;
    }
    printw("Enter word to store at %04x (hex): ", loc);
    if(scanw("%x ",&val) == EOF){
      error(INPUT);
      return;
    }
    memory.word_mem[loc >> 1] = (unsigned short)val;
    break;
  }
  }
}

void breakpoint(){
  int bp;
  printw("\nEnter breakpoint [0000..FFFE] (-1 to clear): ");
  if(scanw("%x ",&bp) == EOF){
    error(INPUT);
    return;
  }
  if(bp == NEVER){
    BRKPT = bp;
    return;
  }
  if(bp%2 != 0){
    error(EVEN);
    return;
  }
  if(bp > 0xFFFE || bp < 0x0000){
    error(OOB);
    return;
  }
  BRKPT = bp;
}

void load(){
  char path[256];
  FILE * fptr;
  printw("\nEnter path to file (relative or absolute): ");
  if(scanw("%s ",path) == EOF){
    error(INPUT);
    return;
  }
  if((fptr = fopen(path,"r")) == NULL){
    error(FDNE);
    return;
  }
  if(loader(fptr) == LOADER_FAIL){ // call loader with file
    error(LOADER);
  }
}

void printRegFile(){
  clear();
  for(int i = 0; i < NUM_REG; i++){
    printw("\nR%d: %04x", i, (int)regFile[i]);
  }
  getch(); // pause
}

void setReg(){
  int reg, value;
  printw("\nWhich register to modify? [0..7]: ");
  if(scanw("%x ",&reg) == EOF){
    error(INPUT);
    return;
  }
  if(reg < 0 || reg >= NUM_REG){
    error(OOB);
    return;
  }
  printw("Enter value for register (currently %04x) (hex): ",
  (int)(regFile[reg] & WORD_MSK));
  if(scanw("%x ",&value) == EOF){
    error(INPUT);
    return;
  }
  regFile[reg] = (unsigned short)(value & WORD_MSK);
  return;
}

void reset(){
  regFile[PC] = memory.word_mem[RESET_PC>>1];
  clock = 0;
}

void printMem(){
  int start, end;
  printw("\nEnter starting location [0000..FFFF]: ");
  if(scanw("%x ",&start) == EOF){
    error(INPUT);
    return;
  }
  if(start < 0 || start >= BYTEMAXMEM){
    error(OOB);
    return;
  }
  start -= start%MEM_PRINT_WIDTH; // round down to nearest mul of 0x10
  printw("\nEnter ending location [0000..FFFF]: ");
  if(scanw("%x ",&end) == EOF){
    error(INPUT);
    return;
  }
  if(end < 0 || end >= BYTEMAXMEM){
    error(OOB);
    return;
  }
  // round up to nearest mul of 0x10
  end += (MEM_PRINT_WIDTH - end%MEM_PRINT_WIDTH);
  if((end - start) < 0){
    error(ENDSTART);
    return;
  }
  clear(); // clear screen
  for(int i = 0; i < (end - start); i += MEM_PRINT_WIDTH){
    // print hex
    for(int j = 0; j < MEM_PRINT_WIDTH && ((start + i + j) < BYTEMAXMEM);
    j++){
      printw("%02x ",memory.byte_mem[start + i + j]);
    }
    // print ascii
    for(int j = 0; j < MEM_PRINT_WIDTH && ((start + i + j) < BYTEMAXMEM);
    j++){
      char out = memory.byte_mem[start + i + j];
      if(out < ' ' || out > '~')
        out = '.';
      printw("%c",out);
    }
    addch('\n');
  }
  getch(); // pause
}

void printPSW(){
  clear();
  printw("Current PSW: %04x", (int)(memory.word_mem[PSW>>1]));
  getch();
}

void error(error_enum e){
  printw("\n%s",errors[e]);
  getch(); // pause
}
