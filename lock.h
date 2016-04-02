#include <stdbool.h>

#ifndef __LOCK_H__
#define __LOCK_H__

typedef volatile bool Lock;

#define barrier __asm__ volatile("" : : : "memory");

void lock(Lock* lock);

void unlock(Lock* lock);

#endif /* __LOCK_H__ */
