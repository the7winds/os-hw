#include "threads.h"
#include "mem_info.h"
#include "fixed_alloc.h"
// ---- init functions -----

// main thread
static Thread* MAIN_THREAD = NULL;

// allocator for threads;
static FixedAllocator* THREAD_ALLOCATOR;

// contains free ids
static uint16_t* freeIdList;
// inedx of first free id;
static uint16_t freeIdFirst;

// id ----> thread
static Thread** threadMap;


uint16_t allocId() {
    atomicBegin();
    uint16_t id = freeIdList[freeIdFirst++];
    atomicEnd();
    return id;
}


void freeId(uint16_t id) {
    atomicBegin();
    freeIdList[--freeIdFirst] = id;
    atomicEnd();
}


void initMultithreading() {
    THREAD_ALLOCATOR = newFixedAllocator(sizeof(Thread), 4);

    // create list of ids
    freeIdList = buddyVAlloc(0);
    freeIdFirst = 0;
    for (uint16_t i = 0; i < MAX_THREAD_ID; ++i) {
        freeIdList[i] = i;
    }

    // create map id ---> thread
    threadMap = buddyVAlloc(7);

    // create struct for main thread
    MAIN_THREAD = (Thread*) fixedAllocate(THREAD_ALLOCATOR);
    MAIN_THREAD->id = allocId();
    MAIN_THREAD->isDead = false;
    __asm__ volatile("movq %%rbp, %0" : "=r"(MAIN_THREAD->stackBase));


    threadMap[0] = MAIN_THREAD;
}


uint16_t createThread(void (*functionPtr)(void*), void* args) {
    Thread* newThread = fixedAllocate(THREAD_ALLOCATOR);

    newThread->id = allocId();
    newThread->isDead = false;
    newThread->stackMem = buddyVAlloc(STACK_SIZE_ORDER);
    newThread->stackBase = (void*) ((uint64_t) newThread->stackMem + (1 << STACK_SIZE_ORDER) * PAGE_SIZE);
    threadMap[newThread->id] = newThread;

    initThreadStack(newThread, (void*) ((uint64_t) functionPtr), args);

    addThreadToTaskQueue(newThread);

    return newThread->id;
}


void initThreadStack(Thread* thread, void* func, void* args) {
    void* info = thread->stackPtr = (void*) ((uint64_t) thread->stackBase - 9 * sizeof(uint64_t));

    extern void* caller;

    ((void**) info)[8] = func;                 // store func address
    ((void**) info)[7] = args;                 // store func's args
    ((void**) info)[6] = &caller;
    ((void**) info)[5] = thread->stackBase;    // rbp
    ((uint64_t*) info)[4] = 0;                 // rbx
    ((uint64_t*) info)[3] = 0;                 // r12
    ((uint64_t*) info)[2] = 0;                 // r13
    ((uint64_t*) info)[1] = 0;                 // r14
    ((uint64_t*) info)[0] = 0;                 // r15

    if (thread->id == 5) {
        printf("THREAD %d\n", thread->id);
        printf("caller %llx", ((void**) info)[6]);
        printf("base %llx", ((void**) info)[5]);
        printf("stack %llx", thread->stackPtr);
    }
}


void freeThreadResources(Thread* thread) {
    printf("thread's #%d resources are released\n", thread->id);
    freeId(thread->id);
    buddyVFree(thread->stackMem, STACK_SIZE_ORDER);
    fixedFree(thread);
}


void joinThread(uint16_t id) {
    Thread* thread = threadMap[id];
    while (!thread->isDead);
    freeThreadResources(thread);
}

// ---- scheduler -----

// threads queue
Thread* threadsQueueFirst;
Thread* threadsQueueLast;

// timer
uint8_t TIME_COUNTER;

void initThreadScheduler() {
    threadsQueueFirst = threadsQueueLast = MAIN_THREAD;
    threadsQueueFirst->next = threadsQueueLast;

    // enables interrupts, so from there the scheduler starts his work
    __asm__ volatile("sti");
}


void printAliveThreads() {
    atomicBegin();

    Thread* t = threadsQueueFirst;
    do {
        printf("%d -> ", t->id);
        t = t->next;
    } while (t != threadsQueueLast -> next);
    printf("NULL\n");

    atomicEnd();
}


void addThreadToTaskQueue(Thread* thread) {
    atomicBegin();
    threadsQueueLast->next = thread;
    threadsQueueLast = thread;
    atomicEnd();
}


void changeCurrentThread() {
    ++TIME_COUNTER;
    if (TIME_COUNTER > TIME_QUANT) {
        TIME_COUNTER = 0;

        Thread* oldCurrent = threadsQueueFirst;

        if (!threadsQueueFirst->isDead) {
            threadsQueueLast->next = threadsQueueFirst;
            threadsQueueLast = threadsQueueFirst;
        }

        do {
            threadsQueueFirst = threadsQueueFirst->next;
        } while(threadsQueueFirst->isDead);

        printAliveThreads();

        if (oldCurrent != threadsQueueFirst) {
            printf("switch: thread #%d (stored at %llx) --> thread #%d (stored at %llx)\n", oldCurrent->id, oldCurrent, threadsQueueFirst->id, threadsQueueFirst);
            switch_threads(&oldCurrent->stackPtr, threadsQueueFirst->stackPtr);
        }
    }
}

void killThread() {
    atomicBegin();
    threadsQueueFirst->isDead = true;
    TIME_COUNTER = 100;     // guarantees that will be handled
    atomicEnd();
    __asm__ volatile("int $32");
}

void killThreadById(uint16_t id) {
    atomicBegin();
    threadMap[id]->isDead = true;
    TIME_COUNTER = 100;     // guarantees that will be handled
    atomicEnd();
    __asm__ volatile("int $32");
}

uint16_t getCurrentId() {
    atomicBegin();
    uint16_t id = threadsQueueFirst->id;
    atomicEnd();
    return id;
}