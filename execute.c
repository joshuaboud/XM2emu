#include "execute.h"
#include "cpu.h"
#include "memory.h"

void relMem(){
  union relMemOpCode *opcode = (union relMemOpCode *)&IR;
  MAR = (unsigned short)opcode->bf.relOff; // start with offset
  if(opcode->bf.LDST == LDR){ // LDR
    MAR += regFile[opcode->bf.S][REG]; // add on offset
  }else{ // STR
    MAR += regFile[opcode->bf.D][REG]; // add on offset
    MBR = regFile[opcode->bf.S][REG]; // transfer data for bussing
  }
  bus(MAR, &MBR, opcode->bf.LDST, opcode->bf.WB); // bus data to or from mem
  if(opcode->bf.LDST == LDR){ // LDR
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
    union bra13OpCode *opcode = (union bra13OpCode *)&IR;
    regFile[LR][REG] = regFile[PC][REG]; // LR = PC
    unsigned short offset = (unsigned short)opcode->bf.off;
    offset <<= 1; // left shift offset to decode
    if(offset & BIT13) offset |= SXT13; // sign extend
    regFile[PC][REG] += offset;
    return;
  }
  // else evaluate condition for 10 bit branch
  union bra10OpCode *opcode = (union bra10OpCode *)&IR;
  if(braCond(opcode->bf.cond)){ // if branch cond evaluates to true
    unsigned short offset = (unsigned short)opcode->bf.off;
    offset <<= 1; // left shift offset to decode
    if(offset & BIT10) offset |= SXT10; // sign extend
    regFile[PC][REG] += offset;
  }
}

int braCond(unsigned cond){
  switch(cond){
  case BEQ: // 000
    return (PSW->psw.Z); // zero set
  case BNE: // 001
    return !(PSW->psw.Z); // zero clear
  case BC:  // 010
    return (PSW->psw.C); // carry set
  case BNC: // 011
    return !(PSW->psw.C); // carry clear
  case BN:  // 100
    return (PSW->psw.N); // negative set
  case BGE: // 101
    return !(PSW->psw.N ^ PSW->psw.V); // PSW.N XOR PSW.V == 0
  case BLT: // 110
    return (PSW->psw.N ^ PSW->psw.V); // PSW.N XOR PSW.V == 1
  case BRA: // 111
    return TRUE; // always branch
  }
}

void regInit(){
  union regInitOpCode *opcode = (union regInitOpCode *)&IR;
  switch(opcode->bf.type){
  case MOVH:
    regFile[opcode->bf.D][REG] &= BYTE_MSK; // clear high byte
    regFile[opcode->bf.D][REG] |= (opcode->bf.B << BYTE_SZ); // fill in byte
    return;
  case MOVLZ:
    regFile[opcode->bf.D][REG] &= BYTE_MSK; // clear high byte
    break;
  case MOVLS:
    regFile[opcode->bf.D][REG] |= ~BYTE_MSK; // set high byte
    break;
  case MOVL:
    break;
  }
  regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear low byte
  regFile[opcode->bf.D][REG] |= (opcode->bf.B & BYTE_MSK); // fill in byte
}

void control(){
  union ctrlOpCode *opcode = (union ctrlOpCode *)&IR;
  switch(opcode->bf.SVCCEX){
  case SVC:
    interrupt(opcode->svc_bf.SA);
    break;
  case CEX:
    cexTF = cexCond(opcode->cex_bf.C);
    CEX_T_CNT = opcode->cex_bf.T;
    CEX_F_CNT = opcode->cex_bf.F;
    cpuMode = CEX_MODE;
    break;
  }
}

int cexCond(unsigned cond){
  switch(cond){
  case EQ:
    return (PSW->psw.Z); // zero set: cexT
  case NE:
    return !(PSW->psw.Z); // zero clear: cexF
  case CS:
    return (PSW->psw.C);
  case CC:
    return !(PSW->psw.C);
  case MI:
    return (PSW->psw.N);
  case PL:
    return !(PSW->psw.N);
  case VS:
    return (PSW->psw.V);
  case VC:
    return !(PSW->psw.V);
  case HI:
    return (PSW->psw.C && !(PSW->psw.Z));
  case LS:
    return (!(PSW->psw.C) || (PSW->psw.Z));
  case GE:
    return (PSW->psw.N == PSW->psw.V);
  case LT:
    return (PSW->psw.N != PSW->psw.V);
  case GT:
    return (!(PSW->psw.Z) && (PSW->psw.N == PSW->psw.V));
  case LE:
    return ((PSW->psw.Z) || (PSW->psw.N != PSW->psw.V));
  case AL:
    return cexT;
  default:
    break;
  }
  // else
  return cexF;
}

void mem(){
  union memOpCode * opcode = (union memOpCode *)&IR;
  int offset = 0;
  switch(opcode->bf.WB){
  case WORD:
    if(opcode->bf.DEC){
      offset -= 2;
    }
    if(opcode->bf.INC){
      offset += 2;
    }
    break;
  case BYTE:
    if(opcode->bf.DEC){
      offset -= 1;
    }
    if(opcode->bf.INC){
      offset += 1;
    }
    break;
  }
  switch(opcode->bf.LDST){
  case LD:
    if(opcode->bf.PRPO == PRE) regFile[opcode->bf.S][REG] += offset;
    MAR = regFile[opcode->bf.S][REG];
    break;
  case ST:
    if(opcode->bf.PRPO == PRE) regFile[opcode->bf.D][REG] += offset;
    MAR = regFile[opcode->bf.D][REG];
    MBR = regFile[opcode->bf.S][REG];
    break;
  }
  bus(MAR, &MBR, opcode->bf.LDST, opcode->bf.WB);
  switch(opcode->bf.LDST){
  case LD:
    if(opcode->bf.PRPO == POST) regFile[opcode->bf.S][REG] += offset;
    switch(opcode->bf.WB){
    case WORD:
      regFile[opcode->bf.D][REG] = MBR;
      break;
    case BYTE:
      regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear low byte
      regFile[opcode->bf.D][REG] |= MBR & BYTE_MSK; // fill in byte
      break;
    }
    break;
  case ST:
    if(opcode->bf.PRPO == POST) regFile[opcode->bf.D][REG] += offset;
    break;
  }
}

void shifting(){
  
}

void bitOp(){
  
}

void ALUtest(){
  
}

void ALU(){
  
}
