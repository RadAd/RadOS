#include "byte.h"

enum video_type {
    VT_RESERVED,
    VT_TEXT,
    VT_GRAPHICS,
};

struct video_mode_s
{
    enum video_type text;
    int width;
    int height;
    int colors;
    BYTE far* address;
};

extern struct video_mode_s video_mode_data[];
