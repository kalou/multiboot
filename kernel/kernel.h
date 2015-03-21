#ifndef _KERNEL_H
#define _KERNEL_H

#define KERNEL_CS 0x08

#include <stdarg.h>
#include <types.h>

void printk(char *fmt, ...);
void vprintk(char *fmt, va_list ap);

void panic(char *fmt, ...);

void *memcpy(void *dest, const void *src, unsigned int n);
void *memset(void *dest, const int c, unsigned int n);

#define BUG_ON(x) { if (x) { panic("BUG at %s line %d\n", __FILE__, __LINE__); }}

#endif
