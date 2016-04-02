#include "lock.h"
#include "threads.h"
#include "utils.h"

void lock(Lock* lock) {
    __asm__ volatile("cli");
    barrier;
    while (*lock) {
        printf("spins\n");
        // extern uint16_t TIME_COUNTER;
        barrier;
        // TIME_COUNTER = 100;
        __asm__ volatile("sti");
        __asm__ volatile("int $32");
        __asm__ volatile("cli");
    }
    barrier;
    *lock = true;
    barrier;
    __asm__ volatile("sti");
}

void unlock(Lock* lock) {
    __asm__ volatile("cli");
    barrier;
    *lock = false;
    barrier;
    __asm__ volatile("sti");
}