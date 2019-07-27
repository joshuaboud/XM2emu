#ifndef EXECUTE_H
#define EXECUTE_H

enum { WORD = 0, BYTE };

struct relMem_bf{
  unsigned int D : 3;
  unsigned int S : 3;
  unsigned int WB : 1;
  unsigned int relOff : 7;
  unsigned int LDST : 1;
  unsigned int res : 1;
};

union relMemOpCode{
  unsigned short word;
  struct relMem_bf bf;
};

enum { LD = 0, ST };

void relMem(void);
// executes LDR and STR

struct bra13_bf{
  unsigned int off : 13;
  unsigned int res : 3;
};

union bra13OpCode{
  unsigned short word;
  struct bra13_bf bf;
};

struct bra10_bf{
  unsigned int off : 10;
  unsigned int cond : 3;
  unsigned int res : 3;
};

union bra10OpCode{
  unsigned short word;
  struct bra10_bf bf;
};

#define SXT13 0xC000
#define SXT10 0xF800

void branching(void);
// executes BL, BEQ/BZ, BNE/BNZ, etc. All branching.

void regInit(void);

void control(void);

void mem(void);

void shifting(void);

void bitOp(void);

void ALUtest(void);

void ALU(void);

#endif
