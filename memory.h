/* File name: memory.h
 * Author: Josh Boudreau
 * School: Dalhousie University
 * Course: ECED 3403 - Computer Architecture
 * Purpose: Provide declarations of the machine's memory, a pointer to
 * the PSW, a pointer to the vector table, and a bus function to
 * interface the CPU with the memory. Useful memory-related macros and
 * enums are also defined here.
 * Last Modified: 2019-07-27
 */

#ifndef MEMORY_H
#define MEMORY_H

#define BYTE_MSK 0xFF
#define BYTE_SZ 8
#define WORD_MSK 0xFFFF
#define WORD_SZ 16

#define BYTEMAXMEM (1<<16) /* 2^16 bytes */
#define WORDMAXMEM (1<<15) /* 2^15 words (starting on even bytes) */
#define VECTORBASE 0xFFC0 /* Base address of vectors */
#define DEV_MEM 0
#define NUM_DEV 10

#define PSW_ADDR 0xFFFC
#define RESET_PC 0xFFFE

#define RESET_VEC 15

enum dev_num { TIMER = 0, KB, SCR };
enum dev_io { OUT = 0, IN };

union mem_ex{
  unsigned char byte_mem[BYTEMAXMEM];
  unsigned short word_mem[WORDMAXMEM];
};

struct psw_bf{
  unsigned short C : 1;
  unsigned short Z : 1;
  unsigned short N : 1;
  unsigned short SLP : 1;
  unsigned short V : 1;
  unsigned short CURR_PRIO : 3;
  unsigned short res : 5;
  unsigned short PREV_PRIO : 3;
};

union psw_ex{
  unsigned short word;
  struct psw_bf psw;
};

extern union psw_ex * PSW; // pointer to PSW

enum { ILL_INST = 8, INV_ADDR, PRIO_FAULT };

struct vector{
  union psw_ex PSW;
  unsigned short ADDR;
};

extern struct vector *vectorTbl; // pointer to first interrupt vector

struct dev_bf{
  unsigned short ie   : 1;
  unsigned short io   : 1;
  unsigned short dba  : 1;
  unsigned short of   : 1;
  unsigned short ena  : 1;
  unsigned short      : 3;
  unsigned short data : 8;
};

union dev{
  unsigned short word;
  struct dev_bf bf;
};

extern union dev *devices; // pointer to device memory

extern union mem_ex memory; // main machine memory

typedef enum { RD, WR } direction;
typedef enum { W, B } wb;

void bus(unsigned short MAR, unsigned short *MBR, direction dir, wb size);
// takes memory address, returns value in mem buffer register if dir is
// RD, writes MBR to mem[MAR] if WR.

#endif
