// https://superuser.com/questions/974581/chs-to-lba-mapping-disk-storage
// http://kernelx.weebly.com/text-console.html

#include "bios/screen.h"
#include "bios/keyb.h"
#include "bios/disk.h"
#include "bool.h"
#include "byte.h"
#include "vmodes.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <i86.h>
#include <malloc.h>

int g_video_mode;
int g_video_page = 0;
int g_video_attribute = 0x1E;

void set_video_mode(int mode)
{
    bios_set_video_mode(mode);
    g_video_mode = mode;
}

struct pos_s
{
    short x;
    short y;
};

struct pos_s get_cursor_position()
{
    int cp = bios_get_cursor_position(g_video_page);
    struct pos_s p;
    p.x = LB(cp);
    p.y = HB(cp);
    return p;
}

void print_char(char c)
{
    // TODO end of screen scroll up should be using attribute from last character but doesn't appear to work
    // color ignored in text mode
    if (c == '\t')
    {
        struct pos_s cp = get_cursor_position();
        int i = 8 - cp.x%8;
        int j;
        for (j = 0; j < i; ++j)
            bios_print_char(g_video_page, ' ', g_video_attribute);
    }
    else
    {
        if (c == '\n')
            bios_print_char(g_video_page, '\r', g_video_attribute);
        bios_print_char(g_video_page, c, g_video_attribute);
    }
    //bios_print_char_attribute(g_video_page, c, g_video_attribute);
}

void print_str(const char* s)
{
    while (*s != 0)
        print_char(*s++);
}

void print_fmt(const char *fmt, ...)
{
    int len;
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (len < 256)
    {
        char buf[256];
        va_start(args, fmt);
        vsnprintf(buf, 256, fmt, args);
        va_end(args);
     
        print_str(buf);
    }
    else
    {
        char *buf = malloc(len + 1);
        va_start(args, fmt);
        vsnprintf(buf, len + 1, fmt, args);
        va_end(args);
     
        print_str(buf);
        free(buf);
    }
}

void clear_screen()
{
    bios_scroll_window_up(0, g_video_attribute, 0, 0, VMD().height - 1, VMD().width - 1);
    bios_set_cursor_position(g_video_page, 0, 0);
}

char keyb_read(int* scan)
{
    int c = bios_keyb_read();
    if (scan != NULL)
        *scan = HB(c);
    return LB(c);
}

void keyb_input(char* buf, int size)
{
    // TODO support arrows, home, end
    int len = 0;
    BOOL cont = TRUE;
    while (cont)
    {
        char c = keyb_read(NULL);
        //print_fmt("%02X %02X ", c, '\b');
        
        switch (c)
        {
        case '\r':  // ENTER
            cont = FALSE;
            break;
            
        case '\b':  // Backspace
            if (len > 0)
            {
                struct pos_s cp = get_cursor_position();
                if (cp.x == 0)
                {
                    bios_set_cursor_position(g_video_page, cp.y - 1, VMD().width - 1);
                }
                else
                {
                    print_char(c);
                }
                bios_write_char_at_cursor(g_video_page, ' ', 1);
                --len;
            }
            break;
            
        case 0x1B: // escape
            {
                struct pos_s cp = get_cursor_position();
                cp.x -= len;
                while (cp.x < 0)
                {
                    --cp.y;
                    cp.x += VMD().width;
                }
                bios_set_cursor_position(g_video_page, cp.y, cp.x);
                bios_write_char_at_cursor(g_video_page, ' ', len);
                len = 0;
            }
            break;
            
        default:
            if (isprint(c) && len < size)
            {
                print_char(c);
                buf[len++] = c;
            }
            break;
        }
    }
    buf[len] = '\0';
}

int split_string(char* s, const char* delem, const char* tokv[])
{
    int tokc = 0;
    const char delim[] = " ";
    const char *ptr = strtok(s, delim);
    
    while(ptr != NULL)
    {
        tokv[tokc++] = ptr;
        ptr = strtok(NULL, delim);
    }
    
    return tokc;
}


#pragma pack (push, 0);
struct boot_sector
{
    BYTE jump[3];
    char oem[8];
    WORD bytes_per_sector;
    BYTE sector_per_cluster;
    WORD reserved_sectors;
    BYTE number_of_fats;
    WORD number_of_root_directory_entries;
    WORD number_of_sectors;
    BYTE media_descriptor;
    WORD sectors_per_fat;
    WORD sectors_per_track;
    WORD number_of_heads;
    WORD number_of_hidden_sectors;
};
#pragma pack (pop);

#pragma pack (push, 0);
struct directory_entry
{
    char name[8];
    char ext[3];
    BYTE attribute;
    BYTE reserved[10];
    WORD time;
    WORD date;
    WORD start_cluster;
    DWORD file_size; // bytes
};
#pragma pack (pop);

enum FileAttribute
{
    FILE_ATTR_READ_ONLY,
    FILE_ATTR_HIDDEN,
    FILE_ATTR_SYSTEM,
    FILE_ATTR_VOLUME,
    FILE_ATTR_DIRECTORY,
    FILE_ATTR_ARCHIVE,
};

void (far * reboot_v)() = MK_FP(0xFFFF, 0x0000);
#pragma aux reboot_v aborts;    // TODO unlcear if this is working - it should turn it from a call to a jmp

void reboot();
#pragma aux reboot = \
    0xea 0x00 0x00 0xFF 0xFF /* far jmp */;

void shutdown_qemu();
#pragma aux shutdown_qemu = \
    "mov ax, 2000h" \
	"mov dx, 604h" \
	"out dx, ax";

void main()
{
    char *buf;
    
    set_video_mode(0x03);
    //set_video_mode(0x10);
    clear_screen();
    print_str("*** Rad OS\n");
    
    buf = (char *) malloc(1024 * (int) sizeof(char));
    while (TRUE)
    {
        int tokc = 0;
        const char *tokv[100];
        
        print_str("> ");
        keyb_input(buf, 1024);
        print_str("\n");
        
        tokc = split_string(buf, " ", tokv);
        
        if (tokc <= 0)
        {
        }
        else if (strcmp(tokv[0], "help") == 0)
        {
            print_str("cls\tclear the screen\n");
            print_str("m\tdisplay memory range\n");
            print_str("r\tdisplay registers\n");
            print_str("reboot\treboot computer\n");
            print_str("shutdown\tshutdown computer\n");
            print_str("color\tset screen colors\n");
            print_str("sizes\tdisplay type sizes\n");
            print_str("mode\tdisplay info about current video mode\n");
            print_str("drive\tdisplay drive geometry\n");
            print_str("chs\tdisplay lba to chs conversion\n");
            print_str("fat\tdisplay fat parameters\n");
            print_str("dir\tdisplay directory listing\n");
        }
        else if (strcmp(tokv[0], "cls") == 0)
        {
            clear_screen();
        }
        else if (strcmp(tokv[0], "color") == 0)
        {
            if (tokc != 2)
            {
                print_str("color [color]\n");
                print_str("\twhere [color] high byte is background and low byte is foreground\n");
                
            }
            else
            {
                g_video_attribute = strtoul(tokv[1], NULL, 16);
                clear_screen();
            }
        }
        else if (strcmp(tokv[0], "mode") == 0)
        {
            print_fmt("size: %dx%d\n", VMD().width, VMD().height);
            print_fmt("colors: %d\n", VMD().colors);
        }
        else if (strcmp(tokv[0], "drive") == 0)
        {
            if (tokc != 2)
            {
                print_str("drive [drive_num]\n");
                print_str("\twhere [drive_num] is 0 for the first diskette and 80 fo the first disk\n");
            }
            else
            {
                struct drive_param dp = bios_drive_param(strtoul(tokv[1], NULL, 16));
                if (dp.cylinders >= 0)
                {
                    print_fmt("cylinders: %d\n", dp.cylinders);
                    print_fmt("sectors: %d\n", dp.sectors);
                    print_fmt("heads: %d\n", dp.heads);
                }
                else
                    print_fmt("error: bios_drive_param\n");
            }
        }
        else if (strcmp(tokv[0], "chs") == 0)
        {
            if (tokc != 3)
            {
                print_str("chs [drive_num] [lba]\n");
                print_str("\twhere [drive_num] is 0 for the first diskette and 80 fo the first disk\n");
                print_str("\twhere [lba] is in decimal\n");
            }
            else
            {
                struct drive_param dp = bios_drive_param(strtoul(tokv[1], NULL, 16));
                if (dp.cylinders >= 0)
                {
                    struct drive_chs chs = lba_to_chs(&dp, strtoul(tokv[2], NULL, 10));
                    print_fmt("cylinder: %d\n", chs.cylinder);
                    print_fmt("sector: %d\n", chs.sector);
                    print_fmt("heads: %d\n", chs.head);
                    print_fmt("lba: %d\n", chs_to_lba(&dp, &chs));
                }
                else
                    print_fmt("error: bios_drive_param\n");
            }
        }
        else if (strcmp(tokv[0], "load") == 0)
        {
            if (tokc != 4)
            {
                print_str("load [drive_num] [mem] [lba]\n");
                print_str("\twhere [drive_num] is 0 for the first diskette and 80 fo the first disk\n");
                print_str("\twhere [mem] where to load to\n");
                print_str("\twhere [lba] is in decimal\n");
            }
            else
            {
                int drive = strtoul(tokv[1], NULL, 16);
                struct drive_param dp = bios_drive_param(drive);
                if (dp.cylinders >= 0)
                {
                    BYTE far* mem = (BYTE far*) strtoul(tokv[2], NULL, 16);
                    struct drive_chs chs = lba_to_chs(&dp, strtoul(tokv[3], NULL, 10));
                    int r = bios_disk_read(drive, 1, &chs, mem);
                    print_fmt("load: %d\n", r);
                }
                else
                    print_fmt("error: bios_drive_param\n");
            }
        }
        else if (strcmp(tokv[0], "dir") == 0)
        {
            if (tokc != 2)
            {
                print_str("dir [drive_num]\n");
                print_str("\twhere [drive_num] is 0 for the first diskette and 80 fo the first disk\n");
            }
            else
            {
                int drive = strtoul(tokv[1], NULL, 16);
                struct drive_param dp = bios_drive_param(drive);
                if (dp.cylinders >= 0)
                {
                    BYTE far* mem = _fmalloc(512);
                    struct drive_chs chs = lba_to_chs(&dp, 0);
                    int r1 = bios_disk_read(drive, 1, &chs, mem);
                    const struct boot_sector far* bs = (const struct boot_sector far*) mem;
                    int number_of_root_directory_entries = bs->number_of_root_directory_entries;
                    int root_directory = bs->reserved_sectors + bs->number_of_fats * bs->sectors_per_fat;
                    struct drive_chs directory_chs = lba_to_chs(&dp, root_directory);
                    int r2 = bios_disk_read(drive, 1, &directory_chs, mem);
                    print_fmt("load: %d %d\n", r1, r2);
                    //print_fmt("directory_entry: %d\n", sizeof(struct directory_entry));
                    {
                        const struct directory_entry far* de = (const struct directory_entry far*) mem;
                        int i = 0;
                        while (de[i].name[0] != 0 && i < number_of_root_directory_entries)
                        {
                            print_fmt("%c%c%c%c%c",
                                (de[i].attribute & FLAG(FILE_ATTR_DIRECTORY)) ? 'd' : '-',
                                (de[i].attribute & FLAG(FILE_ATTR_READ_ONLY)) ? 'r' : '-',
                                (de[i].attribute & FLAG(FILE_ATTR_ARCHIVE)) ? 'a' : '-',
                                (de[i].attribute & FLAG(FILE_ATTR_SYSTEM)) ? 's' : '-',
                                (de[i].attribute & FLAG(FILE_ATTR_HIDDEN)) ? 'h' : '-');
                            print_fmt(" %.8Fs.%.3Fs", de[i].name, de[i].ext);
                            print_fmt(" %lu", de[i].file_size);
                            print_fmt("\n");
                            
                            ++i;
                        }
                    }
                    _ffree(mem);
                }
                else
                    print_fmt("error: bios_drive_param\n");
            }
        }
        else if (strcmp(tokv[0], "fat") == 0)
        {
            if (tokc != 2)
            {
                print_str("fat [drive_num]\n");
                print_str("\twhere [drive_num] is 0 for the first diskette and 80 fo the first disk\n");
            }
            else
            {
                int drive = strtoul(tokv[1], NULL, 16);
                struct drive_param dp = bios_drive_param(drive);
                if (dp.cylinders >= 0)
                {
                    BYTE far* mem = _fmalloc(512);
                    struct drive_chs chs = lba_to_chs(&dp, 0);
                    int r = bios_disk_read(drive, 1, &chs, mem);
                    print_fmt("load: %d\n", r);
                    {
                        const struct boot_sector far* bs = (const struct boot_sector far*) mem;
                        print_fmt("oem: %Fs\n", bs->oem);
                        print_fmt("bytes_per_sector: %u\n", bs->bytes_per_sector);
                        print_fmt("sector_per_cluster: %u\n", bs->sector_per_cluster);
                        print_fmt("reserved_sectors: %u\n", bs->reserved_sectors);
                        print_fmt("number_of_fats: %u\n", bs->number_of_fats);
                        print_fmt("number_of_root_directory_entries: %u\n", bs->number_of_root_directory_entries);
                        print_fmt("number_of_sectors: %u\n", bs->number_of_sectors);
                        print_fmt("media_descriptor: %u\n", bs->media_descriptor);
                        print_fmt("sectors_per_fat: %u\n", bs->sectors_per_fat);
                        print_fmt("sectors_per_track: %u\n", bs->sectors_per_track);
                        print_fmt("number_of_heads: %u\n", bs->number_of_heads);
                        print_fmt("number_of_hidden_sectors: %u\n", bs->number_of_hidden_sectors);
                        print_fmt("root_directory: %u\n", bs->reserved_sectors + bs->number_of_fats * bs->sectors_per_fat);
                    }
                    _ffree(mem);
                }
                else
                {
                    print_fmt("error: bios_drive_param\n");
                }
            }
        }
        else if (strcmp(tokv[0], "reboot") == 0)
        {
            //reboot();
            (*reboot_v)();
        }
        else if (strcmp(tokv[0], "shutdown") == 0 || strcmp(tokv[0], "exit") == 0)
        {
            shutdown_qemu();
        }
#if 1
        else if (strcmp(tokv[0], "sizes") == 0)
        {
            print_fmt("BYTE:\t\t%d\n", sizeof(BYTE));
            print_fmt("BYTE*:\t\t%d\n", sizeof(BYTE*));
            print_fmt("BYTE far*:\t%d\n", sizeof(BYTE far*));
            print_fmt("WORD:\t\t%d\n", sizeof(WORD));
            print_fmt("char:\t\t%d\n", sizeof(char));
            print_fmt("short:\t\t%d\n", sizeof(short));
            print_fmt("int:\t\t%d\n", sizeof(int));
            print_fmt("long:\t\t%d\n", sizeof(long));
            print_fmt("long long:\t%d\n", sizeof(long long));
        }
#endif
        else if (strcmp(tokv[0], "m") == 0)
        {
            if (tokc != 3)
            {
                print_str("m [start] [end]\n");
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
                        print_fmt("%04X:%04X: ", HW(b), LW(b));
                    print_fmt("%02X ", *b);
                    s[c] = isprint(*b) ? *b : '.';
                    b++;
                    if (c++ >= 15)
                    {
                        s[c] = '\0';
                        c = 0;
                        print_str(" ");
                        print_str(s);
                        print_str("\n");
                    }
                }
                if (c != 0)
                {
                    print_str("\n");
                }
            }
        }
        else if (strcmp(tokv[0], "r") == 0)
        {
            struct SREGS sregs;
            segread(&sregs);
            print_fmt("ss: %04X\n", sregs.ss);
            print_fmt("cs: %04X\n", sregs.cs);
            print_fmt("ds: %04X\n", sregs.ds);
            print_fmt("es: %04X\n", sregs.es);
#ifdef __386__
            print_fmt("fs: %04X\n", sregs.fs);
            print_fmt("gs: %04X\n", sregs.gs);
#endif
        }
        else
        {
            print_str("Unknown command: ");
            print_str(tokv[0]);
            print_str("\n");
        }
    }
    free(buf);
    buf = NULL;
    
#pragma warning 111 5
    while (TRUE);
#pragma warning 111 1
}
