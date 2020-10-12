struct pos_s
{
    short x;
    short y;
};

void terminal_init();
const struct video_mode_s* terminal_get_mode_details();

int terminal_get_attribute();
void terminal_set_attribute(int attr);

void terminal_clear_screen();
struct pos_s terminal_get_cursor_position();
void terminal_print_char(char c);

inline void terminal_print_str(const char* s)
{
    while (*s != 0)
        terminal_print_char(*s++);
}

void terminal_print_fmt(const char *fmt, ...);

char terminal_keyb_read(int* scan);
void terminal_keyb_input(char* buf, int size);
