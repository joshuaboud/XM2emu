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

#define PSW 0xFFFC
#define RESET_PC 0xFFFE

union mem_ex{
  unsigned char byte_mem[BYTEMAXMEM];
  unsigned short word_mem[WORDMAXMEM];
};

extern union mem_ex memory;

typedef enum { RD, WR } direction;
typedef enum { W, B } wb;

void bus(unsigned short MAR, unsigned short *MBR, direction dir, wb size);
// takes memory address, returns value in mem buffer register if dir is
// RD, writes MBR to mem[MAR] if WR.

#endif
