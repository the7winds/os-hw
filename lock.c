#include "lock.h"
#include "threads.h"
#include "utils.h"

uint64_t atomicBegin() {
    uint64_t RFLAGS;
    __asm__ volatile("pushfq\n\t"
            "cli\n\t"
            "popq %0" : "=a"(RFLAGS));
    barrier;
    return RFLAGS;
}

void atomicEnd(uint64_t RFLAGS) {
    barrier;
    __asm__ volatile("pushq %0\n\t"
            "popfq" : : "a"(RFLAGS));
}

void lock(Lock* lock) {
    uint64_t RFLAGS = atomicBegin();

    while (*lock) {
        extern uint16_t TIME_COUNTER;
        TIME_COUNTER = 100;
        atomicEnd(RFLAGS);
        __asm__ volatile("int $32");
        RFLAGS = atomicBegin();
    }
    *lock = true;
    
    atomicEnd(RFLAGS);
}

void unlock(Lock* lock) {
    uint64_t RFLAGS = atomicBegin();
    
    *lock = false;
    
    atomicEnd(RFLAGS);
}