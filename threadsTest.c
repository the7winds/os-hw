#include "threadsTest.h"
#include "buddy_alloc.h"
#include "threads.h"
#include "lock.h"

void simpleNThreadTest(int n, int delay) {
    uint16_t* ids = buddyVAlloc(0);
    for (int i = 0; i < n; ++i) {
        ids[i] = createThread(delayedPrintIdTask, &delay);
    }
    for (int i = 0; i < n; ++i) {
        joinThread(ids[i]);
    }
}


void delayedPrintIdTask(void* arg) {
    int delay = *((int*) arg);
    for (int i = 0; i < delay; ++i) {
        printf("thread #%u\n", getCurrentId());
    }
}

static int lockTestS = 0;
static Lock testLock;


void lockTest(int n, int t) {
    lockTestS = 0;
    testLock = false;

    uint16_t* ids = buddyVAlloc(0);
    for (int i = 0; i < n; ++i) {
        ids[i] = createThread(nIncTask, &t);
    }

    for (int i = n - 1; i >= 0; --i) {
        joinThread(ids[i]);
    }

    printf("expected: %d\nactual: %d\n", n * t, lockTestS);
}

void nIncTask(void* arg) {
    printf("#%d begin\n", getCurrentId());
    int n = *((int*) arg);
    printf("#%d 1\n", getCurrentId());
    for (int i = 0; i < n; ++i) {
        printf("#%d %d\n", getCurrentId(), i);
        lock(&testLock);
        int t = lockTestS;
        for (int j = 0; j < 1000; ++j);
        t = t + 1;
        for (int j = 0; j < 1000; ++j);
        lockTestS = t;
        unlock(&testLock);
    }
}

void killByIdTest() {
    KilledArg killedArg;

    killedArg.delay = 1000000000;
    killedArg.ticks = 0;

    uint16_t id = createThread(killedTask, &killedArg);
    for (long long i = 0; i < 100000; ++i) {
        printf("thread #%d\n", getCurrentId());
    }

    killThreadById(id);
    joinThread(id);
    printf("done %d tiks from %d\n", killedArg.ticks, killedArg.delay);
}

void killedTask(void* arg) {
    KilledArg* killedArg = ((KilledArg*) arg);
    for (long long i = 0; i < killedArg->delay; ++i) {
        printf("thread #%d\n", getCurrentId());
        killedArg->ticks++;
    }
}