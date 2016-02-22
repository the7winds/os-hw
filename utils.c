#include <stdarg.h>

#include "utils.h"
#include "UART.h"

int memcmp(void* ptr1, void* ptr2, size_t num) {
    char* p1 = (char*) ptr1;
    char* p2 = (char*) ptr2;
    
    while (num) {
        if (*p1 - *p2) {
            return *p1 - *p2;
        }
        --num;
        ++p1;
        ++p2;
    }
    return 0;
}

int printf(char* format, ...) {
    va_list args;
    va_start(args, format)    ;
    int r = vprintf(format, args);
    va_end(args);
    return r;
}

int vprintf(char* format, va_list args) {
    return printfHelper(format, args, UARTprinter, NULL);
}

int printStep2(void* arg, size_t argSize, char type, void (*printer)(char c, void* wrapper), void* wrapper) {
    int r = 0;
    switch (type) {
        case 'c':
            printer(*((char*)arg), wrapper);
            break;
        case 'd':
        case 'i':        
            r = printNum(arg, argSize, 1, 10, printer, wrapper);
            break;
        case 'o':
            r = printNum(arg, argSize, 0, 8, printer, wrapper);
            break;
        case 'u':
            r = printNum(arg, argSize, 0, 10, printer, wrapper);
            break;        
        case 'x':
            r = printNum(arg, argSize, 0, 16, printer, wrapper);
            break;
        case 's': ;
            char* s = *(char**)arg;
            while (*s) {
                printer(*(s++), wrapper);
            }
            break;
        case 'p':
            r = printNum(arg, argSize, 0, 10, printer, wrapper);
            break;
        default:
            r = 1;
        }
    return r;
}

int printNum(void* arg, size_t argSize, char sign, int base, void (*printer)(char c, void* wrapper), void* wrapper) {
    if (sign) {
        return printSignedNum(arg, argSize, base, printer, wrapper);
    } else {
        return printUnsignedNum(arg, argSize, base, printer, wrapper);
    }
}

int printSignedNum(void* arg, size_t argSize, int base, void (*printer)(char c, void* wrapper), void* wrapper) {
    char chars[20];
    int n = 0;
    switch (argSize) {
        case sizeof(char): {
            char num = *((char*) arg);
            if (num < 0) {
                printer('-', wrapper);
                num = -num;
            }
            do {
                chars[n++] = '0' + num % base;
                num /= base;
            } while (num);
        }
            break;
        case sizeof(short): {
            short num = *((short*) arg);
            if (num < 0) {
                printer('-', wrapper);
                num = -num;
            }            
            do {
                chars[n++] = '0' + num % base;
                num /= base;
            } while (num);        
        }
            break;
        case sizeof(int): {
            int num = *((int*) arg);
            if (num < 0) {
                printer('-', wrapper);
                num = -num;
            }            
            do {
                chars[n++] = '0' + num % base;
                num /= base;
            } while (num);
        }
            break;
        /* case sizeof(long): {
            long num = *((long*) arg);
            while (num) {
                chars[n++] = '0' + num % base;
                num /= base;
            }
        }
            break; */
        case sizeof(long long): {
            long long num = *((long long*) arg);
            if (num < 0) {
                printer('-', wrapper);
                num = -num;
            }            
            do {
                chars[n++] = '0' + num % base;
                num /= base;
            } while (num);
        }
            break;
        default:
            return 1;
    }

    for (int i = n - 1; i >= 0; --i) {
        printer(chars[i], wrapper);
    }

    return 0;
}

int printUnsignedNum(void* arg, size_t argSize, int base, void (*printer)(char c, void* wrapper), void* wrapper) {
    char chars[20];
    int n = 0;
    switch (argSize) {
        case sizeof(char): {
            unsigned char num = *((unsigned char*) arg);
            do {
                chars[n++] = (num % base > 9 ? 'A' + num % base - 10 : '0' + num % base);
                num /= base;
            } while (num);
        }
            break;
        case sizeof(short): {
            unsigned short num = *((unsigned short*) arg);
            do {
                chars[n++] = (num % base > 9 ? 'A' + num % base - 10 : '0' + num % base);
                num /= base;
            } while (num);        
        }
            break;
        case sizeof(int): {
            unsigned int num = *((unsigned int*) arg);
            do {
                chars[n++] = (num % base > 9 ? 'A' + num % base - 10 : '0' + num % base);
                num /= base;
            } while (num);
        }
            break;
        /* case sizeof(long): {
            long num = *((long*) arg);
            while (num) {
                chars[n++] = '0' + num % base;
                num /= base;
            }
        }
            break; */
        case sizeof(unsigned long long): {
            unsigned long long num = *((unsigned long long*) arg);
            do {
                chars[n++] = (num % base > 9 ? 'A' + num % base - 10 : '0' + num % base);
                num /= base;
            } while (num);
        }
            break;
        default:
            return 1;
    }

    for (int i = n - 1; i >= 0; --i) {
        printer(chars[i], wrapper);
    }

    return 0;
}

int snprintf(char *s, size_t n, char *format, ...) {
    va_list args;
    va_start(args, format);
    int r = vsnprintf(s, n, format, args);
    va_end(args);
    return r;
}

int vsnprintf(char *s, size_t n, char *format, va_list args) {
    struct BufferWrapper bw;
    bw.i = 0;
    bw.bufSize = n;
    bw.buf = s;
    return printfHelper(format, args, bufferPrinter, &bw);
}

int printfHelper(char *format, va_list args, void (*printer)(char c, void* wrapper), void* wrapper) {
    int r = 0;

    while (*format != 0) {
        if (*format == '%') {
            ++format;
            if (memcmp(format, "hh", 2) == 0) {
                format += 2;
                char arg = va_arg(args, int);   // char
                r = printStep2(&arg, sizeof(char), *format, printer, wrapper);
            } else if (memcmp(format, "h", 1) == 0) {
                format += 1;
                short arg = va_arg(args, int);  // short
                r = printStep2(&arg, sizeof(short), *format, printer, wrapper);
            } else if (memcmp(format, "ll", 2) == 0) {
                format += 2;
                long long arg = va_arg(args, long long);
                r = printStep2(&arg, sizeof(long long), *format, printer, wrapper);
            } else if (memcmp(format, "l", 1) == 0) {
                format += 1;
                long arg = va_arg(args, long);
                r = printStep2(&arg, sizeof(long), *format, printer, wrapper);
            } else if (memcmp(format, "z", 1) == 0) {
                format += 1;
                size_t arg = va_arg(args, size_t);
                r = printStep2(&arg, sizeof(size_t), *format, printer, wrapper);
            } else {
                if (*format == 's' || *format == 'p') {
                    void* arg = va_arg(args, void*);
                    r = printStep2(&arg, sizeof(void*), *format, printer, wrapper);
                } else {
                    int arg = va_arg(args, int);
                    r = printStep2(&arg, sizeof(int), *format, printer, wrapper);
                }
            }
        } else {
            printer(*format, wrapper);
        }

        if (r) {
            return r;
        }

        ++format;
    }

    return r;
}


void UARTprinter(char c, void* wrapper) {
    if (wrapper == NULL) {}
    UARTputchar(c);
}

void bufferPrinter(char c, void* wrapper) {
    struct BufferWrapper* bw = (struct BufferWrapper*) wrapper;
    if (bw->i < bw->bufSize) {
        bw->buf[bw->i++] = c;
    }
}
