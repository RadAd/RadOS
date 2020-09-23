#include "byte.h"

int LB(int x)
{
    return x & 0xFF;
}

int HB(int x)
{
    return LB(x >> (BIT_SIZE * sizeof(BYTE)));
}

int LW(long x)
{
    return x & 0xFFFF;
}

int HW(long x)
{
    return LW(x >> (BIT_SIZE * sizeof(WORD)));
}

int LWFP(void far* x)
{
    return (long) x & 0xFFFF;
}

int HWFP(void far* x)
{
    return LW((long) x >> (BIT_SIZE * sizeof(WORD)));
}
