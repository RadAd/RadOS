#include "vmodes.h"

// http://www.minuszerodegrees.net/video/bios_video_modes.txt

struct video_mode_s video_mode_data[256] = {
    /* 0x00 */ { VT_TEXT, 40, 25, 16, (BYTE far*) 0xB8000000 }, // CGA gray, EGA gray, MCGA color, VGA color
    /* 0x01 */ { VT_TEXT, 40, 25, 16, (BYTE far*) 0xB8000000 }, // 8 back
    /* 0x02 */ { VT_TEXT, 80, 25, 16, (BYTE far*) 0xB8000000 }, // CGA gray, EGA gray, MCGA color, VGA color
    /* 0x03 */ { VT_TEXT, 80, 25, 16, (BYTE far*) 0xB8000000 }, // 8 back
};
