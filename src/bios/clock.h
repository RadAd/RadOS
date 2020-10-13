// http://vitaly_filatov.tripod.com/ng/asm/asm_029.html

typedef int int8;
typedef int bool8;

inline int8 bcd_to_int8(int8 bcd)
{
    //assert(((bcd & 0xF0) >> 4) < 10);  // More significant nybble is valid
    //assert((bcd & 0x0F) < 10);         // Less significant nybble is valid
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

struct time
{
    int8 hours;
    int8 minutes;
    int8 seconds;
    bool8 daylight;
};

struct date
{
    int year;
    int8 month;
    int8 day;
};

// http://vitaly_filatov.tripod.com/ng/asm/asm_029.3.html
inline struct time bios_get_time()
{
    union REGS  regs;
    
    regs.h.ah = 0x02;
        
#if defined(__386__) && defined(__DOS__)
    int386(0x1A, &regs, &regs);
#else
    int86(0x1A, &regs, &regs);
#endif

    if (regs.w.cflag & INTR_CF)
    {
        // The BIOS avoids reentrancy by not returning the time if the clock
        // happens to be in the process of being updated.  Thus, if CF is
        // set on return, you should try a few more times before giving up.
        struct time t;
        t.hours = -1;
        return t;
    }
    else
    {
        struct time t;
        t.hours = bcd_to_int8(regs.h.ch);
        t.minutes = bcd_to_int8(regs.h.cl);
        t.seconds = bcd_to_int8(regs.h.dh);
        t.daylight = regs.h.dl;
        return t;
    }
}

// http://vitaly_filatov.tripod.com/ng/asm/asm_029.5.html
inline struct date bios_get_date()
{
    union REGS  regs;
    
    regs.h.ah = 0x04;
        
#if defined(__386__) && defined(__DOS__)
    int386(0x1A, &regs, &regs);
#else
    int86(0x1A, &regs, &regs);
#endif

    if (regs.w.cflag & INTR_CF)
    {
        // The BIOS avoids reentrancy by not returning the time if the clock
        // happens to be in the process of being updated.  Thus, if CF is
        // set on return, you should try a few more times before giving up.
        struct date d;
        d.year = -1;
        return d;
    }
    else
    {
        struct date d;
        d.year = bcd_to_int8(regs.h.ch) * 100 + bcd_to_int8(regs.h.cl);
        d.month = bcd_to_int8(regs.h.dh);
        d.day = bcd_to_int8(regs.h.dl);
        return d;
    }
}
