/* File name: execute.c
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: defitions of functions called by function pointer in
 * execute() of cpu.c.
 * Last Modified: 2019-07-27
 */

#include "execute.h"
#include "cpu.h"
#include "memory.h"

unsigned carry[2][2][2] = {0, 0, 1, 0, 1, 0, 1, 1};
unsigned overflow[2][2][2] = {0, 1, 0, 0, 0, 0, 1, 0};

void updatePSWarith(unsigned short src, unsigned short dst,
                    unsigned short res, int WB){
  unsigned short mss, msd, msr; /* Most significant src, dst, and res bits */
  if (WB == WORD){
  mss = (src & BIT15 != 0);
  msd = (dst & BIT15 != 0);
  msr = (res & BIT15 != 0);
  }
  else /* Byte */
  {
  mss = (src & BIT7 != 0);
  msd = (dst & BIT7 != 0);
  msr = (res & BIT7 != 0);
  res &= BYTE_MSK; /* Mask high byte for 'z' check */
  }
  /* Carry */
  PSW->psw.C = carry[mss][msd][msr];
  /* Zero */
  PSW->psw.Z = (res == 0);
  /* Negative */
  PSW->psw.N = (msr == 1);
  /* oVerflow */
  PSW->psw.V = overflow[mss][msd][msr];
}

void updatePSWbit(unsigned short res, int WB){
  switch(WB){
  case WORD:
    PSW->psw.N = (res & BIT15 != 0); // negative word
    PSW->psw.Z = (res == 0); // result is zero
    break;
  case BYTE:
    PSW->psw.N = (res & BIT7 != 0); // negative byte
    PSW->psw.Z = ((res & BYTE_MSK) == 0); // result is zero
    break;
  }
}

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
      offset -= WORD_MEM_WIDTH;
    }
    if(opcode->bf.INC){
      offset += WORD_MEM_WIDTH;
    }
    break;
  case BYTE:
    if(opcode->bf.DEC){
      offset -= BYTE_MEM_WIDTH;
    }
    if(opcode->bf.INC){
      offset += BYTE_MEM_WIDTH;
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
  union shiftOpCode *opcode = (union shiftOpCode *)&IR;
  switch(opcode->bf.operation){
  case SWAP:
  {
    unsigned short temp = regFile[opcode->bf.S][REG];
    regFile[opcode->bf.S][REG] = regFile[opcode->bf.D][REG];
    regFile[opcode->bf.D][REG] = temp;
    break;
  }
  case SRA:
  case RRC:
    PSW->psw.C = (regFile[opcode->bf.D][REG] & BIT0);
    switch(opcode->bf.WB){
    case WORD:
      regFile[opcode->bf.D][REG] >>= 1; // shift right
      if(opcode->bf.operation == RRC)
        regFile[opcode->bf.D][REG] |= (PSW->psw.C << 15); // set bit 15 to carry
      break;
    case BYTE:
    {
      unsigned char temp = (regFile[opcode->bf.D][REG] & BYTE_MSK);
      temp >>= 1; // shift right;
      regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear low byte
      regFile[opcode->bf.D][REG] |= temp; // fill in byte
      if(opcode->bf.operation == RRC)
        regFile[opcode->bf.D][REG] |= (PSW->psw.C << 7); // set bit 7 to carry
      break;
    }
    }
    break;
  case SWPB:
  {
    unsigned char temp = (regFile[opcode->bf.D][REG] & BYTE_MSK); // save LSB
    regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear low byte
    // fill LSB
    regFile[opcode->bf.D][REG] |= regFile[opcode->bf.D][REG] >> BYTE_SZ;
    regFile[opcode->bf.D][REG] &= BYTE_MSK; // clear high byte
    regFile[opcode->bf.D][REG] |= (temp << BYTE_SZ); // fill MSB
    break;
  }
  case SXT:
    if(regFile[opcode->bf.D][REG] & BIT7){ // negative
      regFile[opcode->bf.D][REG] |= ~BYTE_MSK; // extend sign with 1
    }else{
      regFile[opcode->bf.D][REG] &= BYTE_MSK; // extend sign with 0
    }
    break;
  default:
    // illegal instruction fault
    interrupt(ILL_INST);
    return;
  }
  updatePSWbit(regFile[opcode->bf.D][REG], opcode->bf.WB);
}

void bitOp(){
  union bitOpOpCode *opcode = (union bitOpOpCode *)&IR;
  unsigned short srcValue = regFile[opcode->bf.S][opcode->bf.RC];
  unsigned short res;
  if(opcode->bf.WB == BYTE) srcValue &= BYTE_MSK; // mask to byte if .B
  switch(opcode->bf.operation){
  case BIT:
    res = (regFile[opcode->bf.D][REG] & srcValue);
    break;
  case BIC:
    res = (regFile[opcode->bf.D][REG] & ~srcValue);
    regFile[opcode->bf.D][REG] = res;
    break;
  case BIS:
    res = (regFile[opcode->bf.D][REG] | srcValue);
    regFile[opcode->bf.D][REG] = res;
    break;
  case MOV:
    regFile[opcode->bf.D][REG] = srcValue; // DST = SRC
    return;
  }
  updatePSWbit(res, opcode->bf.WB);
}

void ALUtest(){
  union ALUtestOpCode *opcode = (union ALUtestOpCode *)&IR;
  unsigned short srcValue = regFile[opcode->bf.S][opcode->bf.RC];
  unsigned short res = regFile[opcode->bf.D][REG]; // grab dest value
  if(opcode->bf.WB == BYTE){
    srcValue &= BYTE_MSK;
    res &= BYTE_MSK;
  }
  switch(opcode->bf.operation){
  case DADD:
  {
    unsigned short carry = PSW->psw.C;
    unsigned short sum;
    bcd_add(NIBBLE(srcValue,0),NIBBLE(res,0), carry, &sum, &carry);
    res &= ~NIBBLE_MSK; // clear nibble 0
    res |= (sum & NIBBLE_MSK); // fill nibble 0
    bcd_add(NIBBLE(srcValue,1),NIBBLE(res,1),carry, &sum, &carry);
    res &= ~(NIBBLE_MSK << NIBBLE_SZ); // clear nibble 1
    res |= ((sum & NIBBLE_MSK) << NIBBLE_SZ); // fill nibble 1
    // save low byte of result:
    regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear LSB
    regFile[opcode->bf.D][REG] |= (res & BYTE_MSK); // fill LSB
    if(opcode->bf.WB == BYTE){
      PSW->psw.C = carry;
      return;
    } // word -> keep going
    bcd_add(NIBBLE(srcValue,2),NIBBLE(res,2),carry, &sum, &carry);
    res &= ~(NIBBLE_MSK << (NIBBLE_SZ * 2)); // clear nibble 2
    res |= ((sum & NIBBLE_MSK) << (NIBBLE_SZ * 2)); // fill nibble 2
    bcd_add(NIBBLE(srcValue,3),NIBBLE(res,3),carry, &sum, &carry);
    res &= ~(NIBBLE_MSK << (NIBBLE_SZ * 3)); // clear nibble 3
    res |= ((sum & NIBBLE_MSK) << (NIBBLE_SZ * 3)); // fill nibble 3
    // save high byte of result:
    regFile[opcode->bf.D][REG] &= BYTE_MSK; // clear MSB
    regFile[opcode->bf.D][REG] |= (res & ~BYTE_MSK); // fill MSB
    PSW->psw.C = carry;
    return;
  }
  case CMP:
    res -= srcValue; // res = dst - src
    updatePSWarith(srcValue, regFile[opcode->bf.D][REG], res, opcode->bf.WB);
    return;
  case XOR:
    res ^= srcValue;
    updatePSWbit(res, opcode->bf.WB);
    break;
  case AND:
    res &= srcValue;
    updatePSWbit(res, opcode->bf.WB);
    break;
  }
  switch(opcode->bf.WB){
  case WORD:
    regFile[opcode->bf.D][REG] = res;
    break;
  case BYTE:
    regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear LSB
    regFile[opcode->bf.D][REG] |= (res & BYTE_MSK); // fill in byte
    break;
  }
}

int bcd_add(unsigned short sd, unsigned short dd, unsigned short c, 
    unsigned short *sum, unsigned short *carry)
{
/*
 - Binary Coded Decimal addition
 - *sum = sd + dd + c
 - if sum >= 10 -> 10s carry, otherwise no carry
*/

*sum = sd + dd + c;
if (*sum >= 10)
{
     /* 10s carry */
     *sum -= 10; /* Sum between 0 and 9 */
     *carry = 1; /* Carry set */
}
else
     /* No carry */
     *carry = 0;  
}

void ALU(){
  union ALUopCode *opcode = (union ALUopCode *)&IR;
  unsigned short srcValue = regFile[opcode->bf.S][opcode->bf.RC];
  unsigned short dstValue = regFile[opcode->bf.D][REG];
  unsigned short res;
  if(opcode->bf.WB == BYTE){
    srcValue &= BYTE_MSK;
    dstValue &= BYTE_MSK;
  }
  if(opcode->bf.N == 0){
    res = srcValue;
  }else{ // negative
    res = ~srcValue;
  }
  if(opcode->bf.C == 1){ // carry
    res += PSW->psw.C; // add carry from PSW
  }else if(opcode->bf.N == 1){
    // carry = 0 AND negative
    res += 1; // one's to two's compliment
  }
  res += dstValue; // calculate final result
  updatePSWarith(srcValue, dstValue, res, opcode->bf.WB);
  switch(opcode->bf.WB){
  case WORD:
    regFile[opcode->bf.D][REG] = res;
    break;
  case BYTE:
    regFile[opcode->bf.D][REG] &= ~BYTE_MSK; // clear LSB
    regFile[opcode->bf.D][REG] |= (res & BYTE_MSK); // fill LSB
    break;
  }
}
