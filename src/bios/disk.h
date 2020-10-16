// BIOS Disk
// http://vitaly_filatov.tripod.com/ng/asm/asm_024.html
// http://www.ctyme.com/intr/int-13.htm

#include <i86.h> // for int386/int86

extern int bios_reset_disk(int drive);
// http://vitaly_filatov.tripod.com/ng/asm/asm_024.1.html
// http://www.ctyme.com/intr/rb-0605.htm
#pragma aux bios_reset_disk =   \
    "mov ah, 00h"               \
    "int 13h"                   \
    parm [dl]                   \
    value [ah];

extern int bios_disk_status(int drive);
// http://vitaly_filatov.tripod.com/ng/asm/asm_024.2.html
// http://www.ctyme.com/intr/rb-0606.htm
#pragma aux bios_disk_status =  \
    "mov ah, 01h"               \
    "int 13h"                   \
    parm [dl]                   \
    value [ah];

struct drive_param
{
    int cylinders;
    int sectors;    // Sectors start at 1
    int heads;
};

inline struct drive_param bios_drive_param(int drive)
{
// http://vitaly_filatov.tripod.com/ng/asm/asm_024.9.html
    union REGS  regs;
    struct drive_param dp = { 0 };
    
    regs.h.ah = 0x08;
    regs.h.dl = drive;
#if defined(__386__) && defined(__DOS__)
    int386(0x13, &regs, &regs);
#else
    int86(0x13, &regs, &regs);
#endif
    if (regs.x.cflag)
    {   // error
        // TODO Can we return anything better here
        dp.cylinders = -1;
    }
    else
    {
        dp.cylinders = (regs.h.ch | ((regs.h.cl & ~0x3F) << 2)) + 1; // TODO Check this
        dp.sectors = regs.h.cl & 0x3F;
        dp.heads = regs.h.dh + 1;
    }
    return dp;
}

struct drive_chs
{
    int cylinder;
    int sector;    // Sectors start at 1
    int head;
};

inline struct drive_chs lba_to_chs(const struct drive_param* dp, int lba)
{
    struct drive_chs chs = { 0 };
    int temp = lba % (dp->heads * dp->sectors);
    chs.cylinder = lba / (dp->heads * dp->sectors);
    chs.head = temp / dp->sectors;
    chs.sector = temp % dp->sectors + 1;
    return chs;
}

inline int chs_to_lba(const struct drive_param* dp, const struct drive_chs* chs)
{
    int lba = ((chs->cylinder * dp->heads + chs->head) * dp->sectors) + chs->sector - 1;
    return lba;
}

inline int bios_disk_read(int drive, int count, const struct drive_chs* chs, void far* mem)
{
// http://vitaly_filatov.tripod.com/ng/asm/asm_024.3.html
    struct SREGS  sregs;
    union REGS  regs;
    int retry = 3;
    
    do
    {
        regs.h.ah = 0x02;
        regs.h.al = count;
        regs.h.ch = chs->cylinder & 0xFF;
        regs.h.cl = chs->sector | ((chs->cylinder >> 2) & 0xC0);;
        regs.h.dh = chs->head;
        regs.h.dl = drive;
        regs.w.bx = FP_OFF(mem);
        
        sregs.es = FP_SEG(mem);
        
#if defined(__386__) && defined(__DOS__)
        int386x(0x13, &regs, &regs, &sregs);
#else
        int86x(0x13, &regs, &regs, &sregs);
#endif
    } while ((regs.x.cflag) && (retry-- > 0));
    
    if (regs.x.cflag)
        return -regs.h.ah; // TODO Can we return anything better here
    else
        return regs.w.ax;
}

inline int bios_disk_write(int drive, int count, const struct drive_chs* chs, void far* mem)
{
// http://vitaly_filatov.tripod.com/ng/asm/asm_024.4.html
    struct SREGS  sregs;
    union REGS  regs;
    int retry = 3;
    
    do
    {
        regs.h.ah = 0x03;
        regs.h.al = count;
        regs.h.ch = chs->cylinder & 0xFF;
        regs.h.cl = chs->sector | ((chs->cylinder >> 2) & 0xC0);;
        regs.h.dh = chs->head;
        regs.h.dl = drive;
        regs.w.bx = FP_OFF(mem);
        
        sregs.es = FP_SEG(mem);
        
#if defined(__386__) && defined(__DOS__)
        int386x(0x13, &regs, &regs, &sregs);
#else
        int86x(0x13, &regs, &regs, &sregs);
#endif
    } while ((regs.x.cflag) && (retry-- > 0));
    
    if (regs.x.cflag)
        return -regs.h.ah; // TODO Can we return anything better here
    else
        return regs.w.ax;
}
