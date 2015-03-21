/*
 * Temporary string functions
 */

void *memset(void *dest, int c, unsigned int n)
{
    char *p = dest;
    for (p = dest; p < (char *) dest + n; p++)
        *p = (char) c;
    return dest;
}

void *memcpy(void *dest, const void *src, unsigned int n)
{
    char *p = dest;
    const char *r;

    for (r = (char *) src; r < (char *) src+n; r++, p++)
        *p = *r;

    return dest;
}
