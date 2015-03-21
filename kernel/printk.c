/*
 * Library stuff might end in there
 */

#include <stdarg.h>
#include <kernel.h>
#include <console.h>


void _print_str(char *str)
{
    while (*str) {
        putchar(*str++);
    }
}

void _print_dec(unsigned long x)
{
    unsigned char digits[] = "0123456789";
    char tmp[128] = { 0 };
    int i = 127;

    do {
        tmp[--i] = digits[x%10];
        x /= 10;
    } while(x && i > 1);

    _print_str(&tmp[i]);
}

void _print_x64(unsigned long x)
{
    unsigned char digits[] = "0123456789abcdef";
    unsigned char b = 0;
    int c = 16;

    _print_str("0x");
    while(c--) {
        b = (x >> 60) & 0xf;
        putchar(digits[b]);
        x <<= 4;
    }
}

void vprintk(char *fmt, va_list ap)
{
    while (*fmt) {
        switch(*fmt) {
            case '%':
                switch(*++fmt) {
                    case 's':
                        _print_str(va_arg(ap, char *));
                        break;
                    case 'x':
                        _print_x64(va_arg(ap, unsigned long));
                        break;
                    case 'd':
                        _print_dec(va_arg(ap, unsigned long));
                        break;
                    default:
                        putchar('%');
                        putchar(*fmt);
                }
                fmt++;
                break;
           default:
                putchar(*fmt++);
                break;
        }
    }
}

void
printk(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    vprintk(fmt, ap);
}
