#include "terminal.h"

#include "bios/screen.h"
#include "bios/keyb.h"
#include "vmodes.h"
#include "bool.h"

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#define VMD() video_mode_data[g_video_mode]

int g_video_mode;
int g_video_page = 0;
int g_video_attribute = 0x1E;

void terminal_set_video_mode(int mode)
{
    bios_set_video_mode(mode);
    g_video_mode = mode;
}

void terminal_init()
{
    terminal_set_video_mode(0x03);
    //sterminal_et_video_mode(0x10);
    terminal_clear_screen();
}

const struct video_mode_s* terminal_get_mode_details()
{
    return &VMD();
}

int terminal_get_attribute()
{
    return g_video_attribute;
}

void terminal_set_attribute(int attr)
{
    g_video_attribute = attr;
}

void terminal_clear_screen()
{
    bios_scroll_window_up(0, g_video_attribute, 0, 0, VMD().height - 1, VMD().width - 1);
    bios_set_cursor_position(g_video_page, 0, 0);
}

struct pos_s terminal_get_cursor_position()
{
    int cp = bios_get_cursor_position(g_video_page);
    struct pos_s p;
    p.x = LB(cp);
    p.y = HB(cp);
    return p;
}

void fix_for_qemu()
{
    struct pos_s pos = terminal_get_cursor_position();
    if (pos.x == 0 && pos.y == VMD().height - 1)
        bios_write_char_and_attribute_at_cursor(g_video_page, ' ', g_video_attribute, VMD().width);
}

void terminal_print_char(char c)
{
    // TODO end of screen scroll up should be using attribute from last character but doesn't appear to work
    // color ignored in text mode
    if (c == '\t')
    {
        struct pos_s cp = terminal_get_cursor_position();
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
        if (c != '\r')
            fix_for_qemu();
    }
    //bios_print_char_attribute(g_video_page, c, g_video_attribute);
}

void terminal_print_fmt(const char *fmt, ...)
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
     
        terminal_print_str(buf);
    }
    else
    {
        char *buf = malloc(len + 1);
        va_start(args, fmt);
        vsnprintf(buf, len + 1, fmt, args);
        va_end(args);
     
        terminal_print_str(buf);
        free(buf);
    }
}

char terminal_keyb_read(int* scan)
{
    int c = bios_keyb_read();
    if (scan != NULL)
        *scan = HB(c);
    return LB(c);
}

void terminal_keyb_input(char* buf, int size)
{
    // TODO support arrows, home, end
    int len = 0;
    BOOL cont = TRUE;
    while (cont)
    {
        char c = terminal_keyb_read(NULL);
        //print_fmt("%02X %02X ", c, '\b');
        
        switch (c)
        {
        case '\r':  // ENTER
            cont = FALSE;
            break;
            
        case '\b':  // Backspace
            if (len > 0)
            {
                struct pos_s cp = terminal_get_cursor_position();
                if (cp.x == 0)
                {
                    bios_set_cursor_position(g_video_page, cp.y - 1, VMD().width - 1);
                }
                else
                {
                    terminal_print_char(c);
                }
                bios_write_char_at_cursor(g_video_page, ' ', 1);
                --len;
            }
            break;
            
        case 0x1B: // escape
            {
                struct pos_s cp = terminal_get_cursor_position();
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
                terminal_print_char(c);
                buf[len++] = c;
            }
            break;
        }
    }
    buf[len] = '\0';
}
