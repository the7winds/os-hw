#ifndef __THREADS_H__
#define __THREADS_H__

#include <stdint.h>
#include <stdbool.h>

// ----- common thread part --------

#define MAX_THREAD_ID 65535	// (1 << 16) - 1
#define STACK_SIZE_ORDER 0

typedef struct Thread Thread;

struct Thread {
    void* stackPtr;     // points to stack top
    void* stackMem;     // points to stack-page
    uint16_t id;        // thread_id
    bool isDead;        // flag is thread dead
    Thread* next;       // to put this to lists
    bool joined;
};

uint16_t allocId();

void freeId(uint16_t id);

void initMultithreading();

uint16_t createThread(void (*functionPtr)(void*), void* args);

void initThreadStack(Thread* thread, uint64_t func, void* args);

void joinThread(uint16_t id);

void killThreadById(uint16_t id);

// ----- scheduler part ---------

#define TIME_QUANT 3

void initThreadScheduler();

void addThreadToTaskQueue(Thread* thread);

void changeCurrentThread();

void switch_threads(void **old_sp, void *new_sp);

uint16_t getCurrentId();

Thread* createCleaner();

void cleaner(void* ignored);

void printAliveThreads();

#endif /* __THREADS_H__ */
