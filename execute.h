#ifndef EXECUTE_H
#define EXECUTE_H

struct relMem_bf{
  int D : 3;
  int S : 3;
  int WB : 1;
  int relOff : 7;
  int LDST : 1;
  int res : 1;
};

union relMemOpCode{
  unsigned short word;
  struct relMem_bf bf;
};

void relMem(void);

void branching(void);

void regInit(void);

void control(void);

void mem(void);

void shifting(void);

void bitOp(void);

void ALUtest(void);

void ALU(void);

#endif
