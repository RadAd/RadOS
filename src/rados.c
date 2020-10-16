#include "bios/disk.h"
#include "bios/clock.h"
#include "bool.h"
#include "vmodes.h"
#include "system.h"
#include "terminal.h"
#include "fatfs.h"
#include "shell.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

// http://www.brokenthorn.com/Resources/OSDevIndex.html
// https://wiki.osdev.org/Floppy_Disk_Controller

// TODO
// Find number of drives attatched
// See https://stanislavs.org/helppc/bios_data_area.html
// 40:10 contains # of diskette drives, less 1 (See INT 11h)
// 40:75 Number of hard disks attached

// https://wiki.osdev.org/Memory_Map_(x86)

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

void command()
{
    char *buf = (char *) malloc(1024 * (int) sizeof(char));
    
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
            shell_type("0:/help.txt");
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
		else if (strcmp(tokv[0], "cd") == 0 || strcmp(tokv[0], "chdir") == 0)
		{
			switch (tokc)
			{
			case 1:
				terminal_print_fmt("%s <dir>\n", tokv[0]);
				break;

			case 2:
				shell_chdir(tokv[1]);
				break;

			default:
				terminal_print_str("Too many arguments\n");
			}
		}
		else if (strcmp(tokv[0], "cat") == 0 || strcmp(tokv[0], "type") == 0)
		{
			switch (tokc)
			{
			case 1:
				terminal_print_fmt("%s <file>\n", tokv[0]);
				break;

			case 2:
				shell_type(tokv[1]);
				break;

			default:
				terminal_print_str("Too many arguments\n");
			}
		}
		else if (strcmp(tokv[0], "cp") == 0 || strcmp(tokv[0], "copy") == 0)
		{
			switch (tokc)
			{
			case 1:
				terminal_print_fmt("%s <dir>\n", tokv[0]);
				break;

			case 2:
			case 3:
                shell_copy(tokv[1], tokv[2]);
				break;

			default:
				terminal_print_str("Too many arguments\n");
			}
		}
		else if (strcmp(tokv[0], "rm") == 0 || strcmp(tokv[0], "del") == 0)
		{
			switch (tokc)
			{
			case 1:
				terminal_print_fmt("%s <file>\n", tokv[0]);
				break;

			case 2:
				shell_del(tokv[1]);
				break;

			default:
				terminal_print_str("Too many arguments\n");
			}
		}
        else if (strcmp(tokv[0], "dir") == 0)
        {
			switch (tokc)
			{
			case 1:
			case 2:
                shell_dir(tokv[1]);
				break;

			default:
				terminal_print_str("Too many arguments\n");
			}
        }
		else if (strcmp(tokv[0], "md") == 0 || strcmp(tokv[0], "mkdir") == 0)
		{
			switch (tokc)
			{
			case 1:
				terminal_print_fmt("%s <dir>\n", tokv[0]);
				break;

			case 2:
				shell_mkdir(tokv[1]);
				break;

			default:
				terminal_print_str("Too many arguments\n");
			}
		}
		else if (strcmp(tokv[0], "rd") == 0 || strcmp(tokv[0], "rmdir") == 0)
		{
			switch (tokc)
			{
			case 1:
				terminal_print_fmt("%s <dir>\n", tokv[0]);
				break;

			case 2:
				shell_rmdir(tokv[1]);
				break;

			default:
				terminal_print_str("Too many arguments\n");
			}
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
}

extern unsigned int bios_low_mem();
#pragma aux bios_low_mem =  \
    "clc"               \
    "int 12h"           \
    value [ax];

extern unsigned int bios_extended_mem();
#pragma aux bios_extended_mem =  \
    "clc"               \
    "MOV AH, 0x88"      \
    "int 15h"           \
    value [ax];

extern BYTE get_boot_dev();
#pragma aux get_boot_dev =  \
    value [dl];

// http://www.uruk.org/orig-grub/mem64mb.html
// http://www.uruk.org/orig-grub/mem64mb.html#int15e801
// http://www.uruk.org/orig-grub/mem64mb.html#int1588

void main()
{
    BYTE boot_dev = get_boot_dev();
    
    terminal_init();
    terminal_print_str("*** Rad OS\n");
    terminal_print_fmt("low mem %u KB\n", bios_low_mem() + 1);
    terminal_print_fmt("ext mem %u KB\n", bios_extended_mem());
    
    {
        FRESULT r;
        r  = mount(0, boot_dev);
        if (r != FR_OK)
            terminal_print_fmt("ERROR: mount: %d\n", r);
        //r = f_chdir("0:/");
        //if (r != FR_OK)
            //terminal_print_fmt("ERROR: f_chdir: %d\n", r);
    }
    
    command();
    
#pragma warning 111 5
    while (TRUE);
#pragma warning 111 1
}
