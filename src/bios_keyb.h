// BIOS Keyboard
// http://vitaly_filatov.tripod.com/ng/asm/asm_001.8.html

// http://vitaly_filatov.tripod.com/ng/asm/asm_002.7.html
// TODO Define constants for status byte
#define bios_keyb_status() ((BYTE far*) 0x00000417)

extern int bios_keyb_read();
// http://vitaly_filatov.tripod.com/ng/asm/asm_027.1.html
// Returns
// AL         ASCII character code
// AH         Scan code
#pragma aux bios_keyb_read =    \
    "mov ah, 00h"               \
    "int 16h"                   \
    value [ax];
