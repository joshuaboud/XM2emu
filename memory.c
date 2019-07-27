#include "memory.h"
#include "cpu.h"

union mem_ex memory;

union psw_ex *PSW = (union psw_ex *)&(memory.word_mem[PSW_ADDR >> 1]);

struct vector *vectorTbl = (struct vector *)&(memory.word_mem[VECTORBASE>>1]);

void bus(unsigned short MAR, unsigned short *MBR, direction dir, wb size){
  switch(dir){
  case RD:
    *MBR = (size == W) ? memory.word_mem[MAR >> 1] : (unsigned short)memory.byte_mem[MAR];
    break;
  case WR:
    if (size == W){
      memory.word_mem[MAR >> 1] = *MBR;
    }else{
      memory.byte_mem[MAR] = (unsigned char)(*MBR & BYTE_MSK);
    }
    break;
  default:
    // error
    break;
  }
  if((int)MAR < VECTORBASE){
    clock += 2;
  }
}
