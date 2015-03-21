/*
 * Basic console functions
 */

#include <asm.h>
#include <kernel.h>

#include <console.h>

static char x, y;
static char attr;

#define COLS 80
#define LINES 24
#define VCHAR(x, y) ((unsigned short *) VIDEO + COLS*(y) + (x))

void setcursor()
{
    outb(0x3d4, 0x0f);
    outb(0x3d5, (y*COLS+x) & 0xff);
    outb(0x3d4, 0x0e);
    outb(0x3d5, (y*COLS+x) >> 8);
}

void init_console(void)
{
    attr = WHITE | BG(BLUE) | BRIGHT;
    for (x = 0; x < COLS; x++)
        for (y = 0; y < LINES; y++)
            *VCHAR(x, y) = (unsigned short) (' ' | attr << 8);
    x = 0;
    y = 0;
    setcursor();
}

void set_console_attribute(char a)
{
    attr = a;
}

void scroll(void)
{
    unsigned char a;
    memcpy(VCHAR(0, 0), VCHAR(0, 1), 2*(LINES-1)*COLS);

    for (a = 0; a < COLS; a++)
        *VCHAR(a, LINES-1) = (' ' | attr << 8);
}

void putchar(unsigned char c)
{
    if (c == '\n') {
        x = COLS-1;
    } else if (c == '\t') {
        x += 4;
        x -= (x % 4);
    } else {
        *VCHAR(x, y) = (c | attr << 8);
        x++;
    }
    if (x >= COLS-1) {
        x = x % (COLS-1);
        y++;
        if (y > LINES-1) {
            y = LINES-1;
            scroll();
        }
    }
    setcursor();
}
