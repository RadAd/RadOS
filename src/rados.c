// https://superuser.com/questions/974581/chs-to-lba-mapping-disk-storage
// http://kernelx.weebly.com/text-console.html

#include "bios/disk.h"
#include "bios/clock.h"
#include "bool.h"
#include "vmodes.h"
#include "system.h"
#include "terminal.h"
#include "fatfs.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>


BOOL isempty(const char* s)
{
	return s[0] == '\0';
}

int split_string(char* s, const char* delim, const char* tokv[])
{
    int tokc = 0;
    const char *ptr = strtok(s, delim);
    
    while(ptr != NULL)
    {
        tokv[tokc++] = ptr;
        ptr = strtok(NULL, delim);
    }
    
    return tokc;
}

FRESULT printdir(const TCHAR* dir)
{
	FRESULT r;
	DIR d = { 0 };
	FILINFO fi = { 0 };
	
	if (dir == NULL)
        dir = ".";

	r = f_opendir(&d, dir);
	if (r != FR_OK)
		return r;
	while (((r = f_readdir(&d, &fi)) == FR_OK) && (!isempty(fi.fname)))
	{
		const struct FatTime ftime = UnpackTime(fi.ftime);
		const struct FatDate fdate = UnpackDate(fi.fdate);

		terminal_print_fmt("%c%c%c%c%c %-11s %8lu %02d/%02d/%04d %02d:%02d:%02d\n",
			fi.fattrib & AM_DIR ? 'd' : '-', fi.fattrib & AM_RDO ? 'r' : '-', fi.fattrib & AM_ARC ? 'a' : '-', fi.fattrib & AM_SYS ? 's' : '-', fi.fattrib & AM_HID ? 'h' : '-',
			fi.fname, fi.fsize, fdate.day, fdate.month, fdate.year + 1980, ftime.hours, ftime.minutes, ftime.seconds * 2);
	}
	if (r != FR_OK)
	{
		f_closedir(&d);
		return r;
	}
	r = f_closedir(&d);
	return r;
}

void main()
{
    char *buf;

    terminal_init();
    terminal_print_str("*** Rad OS\n");
    
    {
        FRESULT r;
        r  = mount(0, 0);
        if (r != FR_OK)
            terminal_print_fmt("ERROR: mount: %d\n", r);
        r = f_chdir("0:/");
        if (r != FR_OK)
            terminal_print_fmt("ERROR: f_chdir: %d\n", r);
    }
    
    buf = (char *) malloc(1024 * (int) sizeof(char));
    while (TRUE)
    {
        int tokc = 0;
        const char *tokv[100] = { NULL };
        char cwd[100];
        
        FRESULT r = f_getcwd(cwd, 100);
        if (r != FR_OK)
            terminal_print_fmt("ERROR: f_getcwd: %d\n", r);
        
        terminal_print_fmt("%s > ", cwd);
        terminal_keyb_input(buf, 1024);
        terminal_print_str("\n");
        
        tokc = split_string(buf, " ", tokv);
        
        if (tokc <= 0)
        {
        }
        else if (strcmp(tokv[0], "help") == 0)
        {
            terminal_print_str("cls\tclear the screen\n");
            terminal_print_str("time\tdisplay time\n");
            terminal_print_str("date\tdisplay date\n");
            terminal_print_str("m\tdisplay memory range\n");
            terminal_print_str("r\tdisplay registers\n");
            terminal_print_str("reboot\treboot computer\n");
            terminal_print_str("shutdown\tshutdown computer\n");
            terminal_print_str("color\tset screen colors\n");
            terminal_print_str("mode\tdisplay info about current video mode\n");
            terminal_print_str("sizes\tdisplay type sizes\n");
            terminal_print_str("drive\tdisplay drive geometry\n");
            terminal_print_str("chs\tdisplay lba to chs conversion\n");
            terminal_print_str("dir\tdisplay directory listing\n");
        }
        else if (strcmp(tokv[0], "cls") == 0)
        {
            terminal_clear_screen();
        }
        else if (strcmp(tokv[0], "color") == 0)
        {
            if (tokc != 2)
            {
                terminal_print_str("color [color]\n");
                terminal_print_str("\twhere [color] high byte is background and low byte is foreground\n");
                
            }
            else
            {
                terminal_set_attribute(strtoul(tokv[1], NULL, 16));
                terminal_clear_screen();
            }
        }
        else if (strcmp(tokv[0], "mode") == 0)
        {
            const struct video_mode_s* details = terminal_get_mode_details();
            terminal_print_fmt("size: %dx%d\n", details->width, details->height);
            terminal_print_fmt("colors: %d\n", details->colors);
        }
        else if (strcmp(tokv[0], "time") == 0)
        {
            struct time t = bios_get_time();
            terminal_print_fmt("time: %02d:%02d:%02d\n", t.hours, t.minutes, t.seconds);
        }
        else if (strcmp(tokv[0], "date") == 0)
        {
            struct date d = bios_get_date();
            terminal_print_fmt("date: %02d/%02d/%04d\n", d.day, d.month, d.year);
        }
        else if (strcmp(tokv[0], "drive") == 0)
        {
            if (tokc != 2)
            {
                terminal_print_str("drive [drive_num]\n");
                terminal_print_str("\twhere [drive_num] is 0 for the first diskette and 80 fo the first disk\n");
            }
            else
            {
                struct drive_param dp = bios_drive_param(strtoul(tokv[1], NULL, 16));
                if (dp.cylinders >= 0)
                {
                    terminal_print_fmt("cylinders: %d\n", dp.cylinders);
                    terminal_print_fmt("sectors: %d\n", dp.sectors);
                    terminal_print_fmt("heads: %d\n", dp.heads);
                }
                else
                    terminal_print_fmt("error: bios_drive_param\n");
            }
        }
        else if (strcmp(tokv[0], "chs") == 0)
        {
            if (tokc != 3)
            {
                terminal_print_str("chs [drive_num] [lba]\n");
                terminal_print_str("\twhere [drive_num] is 0 for the first diskette and 80 fo the first disk\n");
                terminal_print_str("\twhere [lba] is in decimal\n");
            }
            else
            {
                struct drive_param dp = bios_drive_param(strtoul(tokv[1], NULL, 16));
                if (dp.cylinders >= 0)
                {
                    struct drive_chs chs = lba_to_chs(&dp, strtoul(tokv[2], NULL, 10));
                    terminal_print_fmt("cylinder: %d\n", chs.cylinder);
                    terminal_print_fmt("sector: %d\n", chs.sector);
                    terminal_print_fmt("heads: %d\n", chs.head);
                    terminal_print_fmt("lba: %d\n", chs_to_lba(&dp, &chs));
                }
                else
                    terminal_print_fmt("error: bios_drive_param\n");
            }
        }
        else if (strcmp(tokv[0], "dir") == 0)
        {
            FRESULT r;
            r  = printdir(tokv[1]);
            if (r != FR_OK)
                terminal_print_fmt("ERROR: printdir: %d\n", r);
        }
        else if (strcmp(tokv[0], "reboot") == 0)
        {
            system_reboot();
        }
        else if (strcmp(tokv[0], "shutdown") == 0 || strcmp(tokv[0], "exit") == 0)
        {
            system_shutdown_qemu();
        }
#if 1
        else if (strcmp(tokv[0], "sizes") == 0)
        {
            terminal_print_fmt("BYTE:\t\t%d\n", sizeof(BYTE));
            terminal_print_fmt("BYTE*:\t\t%d\n", sizeof(BYTE*));
            terminal_print_fmt("BYTE far*:\t%d\n", sizeof(BYTE far*));
            terminal_print_fmt("WORD:\t\t%d\n", sizeof(WORD));
            terminal_print_fmt("char:\t\t%d\n", sizeof(char));
            terminal_print_fmt("short:\t\t%d\n", sizeof(short));
            terminal_print_fmt("int:\t\t%d\n", sizeof(int));
            terminal_print_fmt("long:\t\t%d\n", sizeof(long));
            terminal_print_fmt("long long:\t%d\n", sizeof(long long));
        }
#endif
        else if (strcmp(tokv[0], "m") == 0)
        {
            if (tokc != 3)
            {
                terminal_print_str("m [start] [end]\n");
            }
            else
            {
                BYTE far* b = (BYTE far*) strtoul(tokv[1], NULL, 16);
                BYTE far* e = (BYTE far*) strtoul(tokv[2], NULL, 16);
                int c = 0;
                char s[17];
                while (b <= e)
                {
                    if (c == 0)
                        terminal_print_fmt("%04X:%04X: ", HW(b), LW(b));
                    terminal_print_fmt("%02X ", *b);
                    s[c] = isprint(*b) ? *b : '.';
                    b++;
                    if (c++ >= 15)
                    {
                        s[c] = '\0';
                        c = 0;
                        terminal_print_str(" ");
                        terminal_print_str(s);
                        terminal_print_str("\n");
                    }
                }
                if (c != 0)
                {
                    terminal_print_str("\n");
                }
            }
        }
        else if (strcmp(tokv[0], "r") == 0)
        {
            struct SREGS sregs;
            segread(&sregs);
            terminal_print_fmt("ss: %04X\n", sregs.ss);
            terminal_print_fmt("cs: %04X\n", sregs.cs);
            terminal_print_fmt("ds: %04X\n", sregs.ds);
            terminal_print_fmt("es: %04X\n", sregs.es);
#ifdef __386__
            terminal_print_fmt("fs: %04X\n", sregs.fs);
            terminal_print_fmt("gs: %04X\n", sregs.gs);
#endif
        }
        else
        {
            terminal_print_str("Unknown command: ");
            terminal_print_str(tokv[0]);
            terminal_print_str("\n");
        }
    }
    free(buf);
    buf = NULL;
    
#pragma warning 111 5
    while (TRUE);
#pragma warning 111 1
}
