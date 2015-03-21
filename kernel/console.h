#ifndef _CONSOLE_H
#define CONSOLE_H

void init_console(void);
void set_console_attribute(char);

void putchar(unsigned char c);

#define VIDEO (unsigned char *) 0xb8000

#define BLUE 1
#define BLACK 0
#define GREEN 2
#define RED 4
#define WHITE (RED|GREEN|BLUE)

#define BG(x) (char) (x<<4)
#define BRIGHT (char) (1<<3)
#define BLINK (char) (1<<7)



#endif
