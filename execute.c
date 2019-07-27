#include "execute.h"
#include "cpu.h"
#include "memory.h"

#include <ncurses.h>

void relMem(){
  union relMemOpCode *opcode = (union relMemOpCode *)&IR;
  MAR = (unsigned short)opcode->bf.relOff; // start with offset
  if(opcode->bf.LDST == LD){ // LDR
    MAR += regFile[opcode->bf.S][REG]; // add on offset
  }else{ // STR
    MAR += regFile[opcode->bf.D][REG]; // add on offset
    MBR = regFile[opcode->bf.S][REG]; // transfer data for bussing
  }
  bus(MAR, &MBR, opcode->bf.LDST, opcode->bf.WB); // bus data to or from mem
  if(opcode->bf.LDST == LD){ // LDR
    if(opcode->bf.WB == WORD){
      regFile[opcode->bf.D][REG] = MBR;
    }else{
      regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear low byte
      regFile[opcode->bf.D][REG] |= MBR & BYTE_MSK; // fill in low byte
    }
  }
}

void branching(){
  if(!(IR & BIT13)){ // bit 13 clear
    // BL
    union bra13OpCode * opcode = (union bra13OpCode *)&IR;
    regFile[LR][REG] = regFile[PC][REG]; // LR = PC
    unsigned short offset = (unsigned short)opcode->bf.off;
    offset <<= 1; // left shift offset to decode
    if(offset & BIT13) offset |= SXT13; // sign extend
    regFile[PC][REG] += offset;
    return;
  }
}

void regInit(){
  
}

void control(){
  
}

void mem(){
  
}

void shifting(){
  
}

void bitOp(){
  
}

void ALUtest(){
  
}

void ALU(){
  
}
