#include "bios/disk.h"
#include "bios/clock.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"

#include "fatfs.h"
#include "terminal.h"   // TODO Remove

// https://superuser.com/questions/974581/chs-to-lba-mapping-disk-storage

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

int block_read(int drive, int lba, int count, void far* mem)
{
    struct drive_param dp = bios_drive_param(drive);
    struct drive_chs chs;
    if (dp.cylinders < 0)
        return -1;
        
    chs = lba_to_chs(&dp, lba);
    return bios_disk_read(drive, count, &chs, mem);
}

int block_write(int drive, int lba, int count, void far* mem)
{
    struct drive_param dp = bios_drive_param(drive);
    struct drive_chs chs;
    if (dp.cylinders < 0)
        return -1;
        
    chs = lba_to_chs(&dp, lba);
    return bios_disk_write(drive, count, &chs, mem);
}

DSTATUS disk_initialize(BYTE pdrv)
{
    //terminal_print_str("disk_initialize\n");
    return disk_status(pdrv);
}

DSTATUS disk_status(BYTE pdrv)
{
    BYTE dev = get_dev(pdrv);
    if (dev == 255)
        return FR_INT_ERR;
    //terminal_print_str("disk_status\n");
    return FR_OK;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    BYTE dev = get_dev(pdrv);
    int r = block_read(dev, sector, count, buff);
    //terminal_print_fmt("disk_read %d %d\n", pdrv, dev);
    if (r < 0)
    {
        terminal_print_fmt("block_read %d\n", r);
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    BYTE dev = get_dev(pdrv);
    int r = block_write(dev, sector, count, buff);
    //terminal_print_fmt("disk_write %d %d\n", pdrv, dev);
    if (r < 0)
    {
        terminal_print_fmt("block_write %d\n", r);
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    switch (cmd)
    {
    case CTRL_SYNC:
        return RES_OK;
        
    default:
        terminal_print_fmt("disk_ioctl %d %d\n", pdrv, cmd);
        return RES_ERROR;
    }
}

DWORD get_fattime(void)
{
	struct time t = bios_get_time();
    struct date d = bios_get_date();
	union
	{
		struct FatDateTime ft;
		DWORD dw;
	} u;
	u.ft.time.seconds = t.seconds / 2;
	u.ft.time.minutes = t.minutes;
	u.ft.time.hours = t.hours;
	u.ft.date.day = d.day;
	u.ft.date.month = d.month;
	u.ft.date.year = d.year - 1980;
	return u.dw;
}
