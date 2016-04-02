#ifndef __THREADS_TESTS_H__
#define __THREADS_TESTS_H__

#include <stdint.h>

void simpleNThreadTest(int n, int delay);

void delayedPrintIdTask(void* arg);

void lockTest(int n, int t);

void nIncTask(void* arg);

void killByIdTest();

struct KilledArg {
    uint32_t delay;
    uint64_t ticks;
};

typedef struct KilledArg KilledArg;

void killedTask(void* arg);

#endif /* __THREADS_TESTS_H__ */