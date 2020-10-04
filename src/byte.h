#ifndef BYTE_H
#define BYTE_H

#define BIT_SIZE (8)

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned long DWORD;

#define LB(x) (BYTE) ((WORD) (x) & 0xFF)
#define HB(x) LB((WORD) (x) >> (BIT_SIZE * sizeof(BYTE)))

#define LW(x) (WORD) ((DWORD) (x) & 0xFFFF)
#define HW(x) LW((DWORD) (x) >> (BIT_SIZE * sizeof(WORD)))

#define FLAG(x) (1 << (x))

#endif
