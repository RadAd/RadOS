// https://superuser.com/questions/974581/chs-to-lba-mapping-disk-storage
// http://kernelx.weebly.com/text-console.html

#include "bios/screen.h"
#include "bios/keyb.h"
#include "bool.h"
#include "byte.h"
#include "vmodes.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <i86.h>

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
                        print_fmt("%04X:%04X: ", HWFP(b), LWFP(b));
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
