#include "loader.h"
#include "memory.h"
#include "cpu.h"
#include "debugger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BYTE_STR_LEN 2
#define MEM_LOC_LEN 4
#define MEM_LOC_BYTES 2
#define REC_BUFF_LEN 256

int loader(FILE * fptr){
  char *recBuff = (char *)malloc(sizeof(char) * REC_BUFF_LEN);
  size_t buffSize = 256;
  char *byteBuff = (char *)malloc(sizeof(char) * (BYTE_STR_LEN + 1));
  char *memBuff = (char *)malloc(sizeof(char) * (MEM_LOC_LEN + 1));
  int memLoc;
  int byteCnt;
  int byte;
  char chksum;
  
  while(getline(&recBuff, &buffSize, fptr) != EOF){
    chksum = 0; // reset checksum
    strncpy(byteBuff,recBuff,BYTE_STR_LEN);
    recBuff += BYTE_STR_LEN;
    if(!(byteBuff[0] == 's' || byteBuff[0] == 'S'))
      return LOADER_FAIL; // not an s record
    switch(byteBuff[1]){
    case '0':
      // ignore S0 record
      break;
    case '1':
      // process S1 record
      // get byte count
      strncpy(byteBuff,recBuff,BYTE_STR_LEN);
      recBuff += BYTE_STR_LEN;
      sscanf(byteBuff,"%x",&byteCnt);
      chksum += (char)byteCnt;
      // get memory location
      strncpy(memBuff,recBuff,MEM_LOC_LEN);
      recBuff += MEM_LOC_LEN;
      byteCnt -= MEM_LOC_BYTES;
      sscanf(memBuff,"%x",&memLoc);
      chksum += (char)(memLoc & BYTE_MSK);
      chksum += (char)(memLoc >> BYTE_SZ);
      while(byteCnt > 1){ // 1 byte left for CHKSUM
        // get a byte
        strncpy(byteBuff,recBuff,BYTE_STR_LEN);
        recBuff += BYTE_STR_LEN;
        byteCnt--;
        sscanf(byteBuff,"%x",&byte);
        if(memLoc > 0xFFFF || memLoc < 0){
          error(MEM_OOB);
          return LOADER_FAIL;
        }
        memory.byte_mem[memLoc++] = (unsigned char)byte;
        chksum += (char)byte;
      }
      // get checksum from record
      strncpy(byteBuff,recBuff,BYTE_STR_LEN);
      recBuff += BYTE_STR_LEN;
      byteCnt--;
      sscanf(byteBuff,"%x",&byte);
      chksum += (char)byte;
      chksum = ~chksum;
      if(chksum != 0){
        error(CHKSUM);
        return LOADER_FAIL;
      }
      break;
    case '9':
      // S9 record
      recBuff += BYTE_STR_LEN; // skip byte count
      strncpy(memBuff,recBuff,MEM_LOC_LEN);
      recBuff += MEM_LOC_LEN;
      sscanf(memBuff,"%x",&memLoc);
      regFile[PC][REG] = (unsigned short)(memLoc & WORD_MSK);
      break;
    default:
      return LOADER_FAIL;
    }
  }
  return LOADER_SUCC;
}
