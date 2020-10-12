#include "system.h"

#include "bios/disk.h"
#include "bool.h"
#include "vmodes.h"
#include "terminal.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

void (far * reboot_v)() = MK_FP(0xFFFF, 0x0000);
#pragma aux reboot_v aborts;    // TODO unlcear if this is working - it should turn it from a call to a jmp

void reboot();
#pragma aux reboot = \
    0xea 0x00 0x00 0xFF 0xFF /* far jmp */;
    
void system_reboot()
{
    (*reboot_v)();
}
