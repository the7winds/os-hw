#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdarg.h>
#define NULL 0

typedef int size_t;

void putc(char c);

void puts(char* str);

int printf(char* format, ...);

int vprintf(char *format, va_list arg);

int snprintf(char *s, size_t n, char *format, ...);

int vsnprintf(char *s, size_t n, char *format, va_list arg);

int memcmp(void* ptr1, void* ptr2, size_t num);

int printStep2(void* arg, size_t argSize, char type, void (*printer)(char c, void* wrapper), void* wrapper);

int printNum(void* arg, size_t argSize, char sign, int base, void (*printer)(char c, void* wrapper), void* wrapper);

int printSignedNum(void* arg, size_t argSize, int base, void (*printer)(char c, void* wrapper), void* wrapper);

int printUnsignedNum(void* arg, size_t argSize, int base, void (*printer)(char c, void* wrapper), void* wrapper);

int printfHelper(char *format, va_list args, void (*printer)(char c, void* wrapper), void* wrapper);

void printer(char c, void* wrapper);

void bufferPrinter(char c, void* wrapper);

struct BufferWrapper {
    int i;
    int bufSize;
    char* buf;
};

#endif /* __UTILS_H__ */