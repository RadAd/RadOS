#ifndef BYTE_H
#define BYTE_H

#define BIT_SIZE (8)

typedef unsigned char BYTE;
typedef unsigned int WORD;

int LB(int x);
int HB(int x);
int LW(long x);
int HW(long x);
int LWFP(void far* x);
int HWFP(void far* x);

#endif
