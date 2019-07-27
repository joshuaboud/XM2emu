#ifndef EXECUTE_H
#define EXECUTE_H

enum { WORD = 0, BYTE };

struct relMem_bf{
  unsigned D : 3;
  unsigned S : 3;
  unsigned WB : 1;
  unsigned relOff : 7;
  unsigned LDST : 1;
  unsigned : 1;
};

union relMemOpCode{
  unsigned short word;
  struct relMem_bf bf;
};

enum { LDR = 0, STR };

void relMem(void);
// executes LDR and STR

struct bra13_bf{
  unsigned off : 13;
  unsigned : 3;
};

union bra13OpCode{
  unsigned short word;
  struct bra13_bf bf;
};

struct bra10_bf{
  unsigned off : 10;
  unsigned cond : 3;
  unsigned : 3;
};

union bra10OpCode{
  unsigned short word;
  struct bra10_bf bf;
};

enum { BEQ, BNE, BC, BNC, BN, BGE, BLT, BRA }; // branch conditions

#define SXT13 0xC000 // sign extend for bra13
#define SXT10 0xF800 // sign extend for bra10

void branching(void);
// executes BL; BEQ/BZ, BNE/BNZ, etc. All branching.

int braCond(unsigned cond);
// evaluates branching condition for void branching(). Returns FALSE or
// TRUE, 0 or 1, respectively.

struct regInit_bf{
  unsigned D : 3;
  unsigned B : 8;
  unsigned type : 2;
  unsigned : 3;
};

union regInitOpCode{
  unsigned short word;
  struct regInit_bf bf;
};

enum {MOVL, MOVLZ, MOVLS, MOVH};

void regInit(void);
// executes MOVL, MOVLZ, MOVLS, MOVH

struct ctrlOpCode_bf{
  unsigned : 10; // placeholder for svc/cex specific fields
  unsigned SVCCEX : 1;
  unsigned : 5; 
};

struct SVC_bf{
  unsigned SA : 4;
  unsigned : 12;
};

struct CEX_bf{
  unsigned F : 3;
  unsigned T : 3;
  unsigned C : 4;
  unsigned : 6;
};

union ctrlOpCode{
  unsigned short word;
  struct ctrlOpCode_bf bf;
  struct SVC_bf svc_bf;
  struct CEX_bf cex_bf;
};

enum {SVC, CEX};

enum {EQ, NE, CS, CC, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL}; // cex

void control(void);
// executes CEX and SVC

int cexCond(unsigned cond);
// evaluates condition for CEX, returns cexF or cexT, 0 or 1, resp.

struct mem_bf{
  unsigned D : 3;
  unsigned S : 3;
  unsigned WB : 1;
  unsigned INC : 1;
  unsigned DEC : 1;
  unsigned PRPO : 1;
  unsigned LDST : 1;
  unsigned : 5;
};

union memOpCode{
  unsigned short word;
  struct mem_bf bf;
};

enum {LD, ST};
enum {POST = 0, PRE};

void mem(void);
// executes LD and ST

void shifting(void);

void bitOp(void);

void ALUtest(void);

void ALU(void);

#endif
