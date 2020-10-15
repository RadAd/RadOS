// BIOS Screen
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.html
// http://www.ctyme.com/intr/int-10.htm

extern void bios_set_video_mode(int mode);
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.1.html
// http://www.ctyme.com/intr/rb-0069.htm
#pragma aux bios_set_video_mode =   \
    "mov ah, 00h"                   \
    "int 10h"                       \
    parm [al]                       \
    modify [AX SP BP SI DI];
    
extern void bios_set_cursor_position(int page, int row, int col);
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.3.html
#pragma aux bios_set_cursor_position =  \
    "mov ah, 02h"                       \
    "int 10h"                           \
    parm [bh] [dh] [dl]                 \
    modify [AX SP BP SI DI];
    
extern int bios_get_cursor_position(int page);
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.4.html
// Returns
// DL         column
// DH         row
#pragma aux bios_get_cursor_position =  \
    "mov ah, 03h"                       \
    "int 10h"                           \
    parm [bh]                           \
    value [dx]                          \
    modify [AX CX SP BP SI DI];

extern void bios_scroll_window_up(int lines, int attribute, int start_row, int start_col, int end_row, int end_col);
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.1.html
// http://www.ctyme.com/intr/rb-0069.htm
#pragma aux bios_scroll_window_up =     \
    "mov ah, 06h"                       \
    "int 10h"                           \
    parm [al] [bh] [ch] [cl] [dh] [dl]  \
    modify [AX SP BP SI DI];

extern void bios_write_char_and_attribute_at_cursor(int page, char c, int attribue, int count);
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.10.html
#pragma aux bios_write_char_and_attribute_at_cursor = \
    "mov ah, 09h"                       \
    "int 10h"                           \
    parm [bh] [al] [bl] [cx]            \
    modify [AX SP BP SI DI];

extern void bios_write_char_at_cursor(int page, char c, int count);
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.11.html
#pragma aux bios_write_char_at_cursor = \
    "mov ah, 0ah"                       \
    "int 10h"                           \
    parm [bh] [al] [cx]                 \
    modify [AX SP BP SI DI];

// attribue - only used in graphics mode
extern void bios_print_char(int page, char c, int attribue);
// http://vitaly_filatov.tripod.com/ng/asm/asm_023.15.html
// http://www.ctyme.com/intr/rb-0106.htm
#pragma aux bios_print_char =   \
    "mov ah, 0eh"               \
    "int 10h"                   \
    parm [bh] [al] [bl]         \
    modify [ah bh bl];
 
 // Didn't work. Not sure if not correct or not implemented in bios I'm using
extern void bios_print_char_attribute(int page, char c, int attribue);
// http://www.ctyme.com/intr/rb-0107.htm
#pragma aux bios_print_char_attribute = \
    "mov ah, 0eh"                       \
    "mov cx, 0abcdh"                    \
    "mov bp, 0abcdh"                    \
    "int 10h"                           \
    parm [bh] [al] [bl]                 \
    modify [ah bh bl];
