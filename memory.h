#ifndef MEMORY_H
#define MEMORY_H

#define BYTE_MSK 0xFF
#define BYTE_SZ 8
#define WORD_MSK 0xFFFF
#define WORD_SZ 16

#define BYTEMAXMEM (1<<16) /* 2^16 bytes */
#define WORDMAXMEM (1<<15) /* 2^15 words (starting on even bytes) */
#define DEVMEM 0x0010
#define VECTORBASE 0xFFC0 /* Base address of vectors */

#define PSW_ADDR 0xFFFC
#define RESET_PC 0xFFFE

union mem_ex{
  unsigned char byte_mem[BYTEMAXMEM];
  unsigned short word_mem[WORDMAXMEM];
};

struct psw_bf{
  int C : 1;
  int Z : 1;
  int N : 1;
  int SLP : 1;
  int V : 1;
  int CURR_PRIO : 3;
  int res : 5;
  int PREV_PRIO : 3;
};

union psw_ex{
  unsigned short word;
  struct psw_bf psw;
};

extern union psw_ex * PSW;

enum { ILL_INST = 8, INV_ADDR, PRIO_FAULT };

struct vector{
  union psw_ex PSW;
  unsigned short ADDR;
};

extern struct vector *vectorTbl;

extern union mem_ex memory;

typedef enum { RD, WR } direction;
typedef enum { W, B } wb;

void bus(unsigned short MAR, unsigned short *MBR, direction dir, wb size);
// takes memory address, returns value in mem buffer register if dir is
// RD, writes MBR to mem[MAR] if WR.

#endif
