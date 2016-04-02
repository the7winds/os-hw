#include "lock.h"
#include "threads.h"
#include "utils.h"

static uint64_t RFLAGS;

void atomicBegin() {
    __asm__ volatile("pushfq\n\t"
            "popq %0\n\t"
            "cli" : "=a"(RFLAGS));
    barrier;
}

void atomicEnd() {
    barrier;
    __asm__ volatile("pushq %0\n\t"
            "popfq" : : "a"(RFLAGS));
}

void lock(Lock* lock) {
    atomicBegin();
    while (*lock) {
        // extern uint16_t TIME_COUNTER;
        atomicEnd();
        __asm__ volatile("int $32");
        atomicBegin();
    }
    *lock = true;
    atomicEnd();
}

void unlock(Lock* lock) {
    __asm__ volatile("cli");
    barrier;
    *lock = false;
    barrier;
    __asm__ volatile("sti");
}