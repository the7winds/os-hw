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

static Thread* deadThreads;

uint16_t allocId() {
    uint64_t RFLAGS = atomicBegin();

    uint16_t id = freeIdList[freeIdFirst++];

    atomicEnd(RFLAGS);

    return id;
}


void freeId(uint16_t id) {
    uint64_t RFLAGS = atomicBegin();

    freeIdList[--freeIdFirst] = id;

    atomicEnd(RFLAGS);
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
    MAIN_THREAD->joined = false;
    MAIN_THREAD->next = NULL;
    MAIN_THREAD->stackPtr = NULL;
    MAIN_THREAD->stackMem = NULL;

    threadMap[0] = MAIN_THREAD;
}


uint16_t createThread(void (*functionPtr)(void*), void* args) {
    Thread* newThread = fixedAllocate(THREAD_ALLOCATOR);

    uint16_t id = allocId();
    newThread->id = id;
    newThread->isDead = false;
    newThread->joined = false;
    newThread->stackMem = buddyVAlloc(STACK_SIZE_ORDER);
    newThread->next = NULL;
    newThread->prev = NULL;
    threadMap[id] = newThread;

    initThreadStack(newThread, (uint64_t) functionPtr, args);
    addThreadToTaskQueue(newThread);

    printf("create thread #%d\n", newThread->id);

    return id;
}


void initThreadStack(Thread* thread, uint64_t func, void* args) {
    thread->stackPtr = (void*) ((uint64_t) thread->stackMem + (1 << STACK_SIZE_ORDER) * PAGE_SIZE - 10 * sizeof(uint64_t));
    void* data = thread->stackPtr;

    extern void* caller;

    ((uint64_t*) data)[8] = func;    // store func address
    ((void**) data)[7] = args;       // store func's args
    ((void**) data)[6] = &caller;
    ((void**) data)[5] = 0;          // rbp
    ((uint64_t*) data)[4] = 0;       // rbx
    ((uint64_t*) data)[3] = 0;       // r12
    ((uint64_t*) data)[2] = 0;       // r13
    ((uint64_t*) data)[1] = 0;       // r14
    ((uint64_t*) data)[0] = 0;       // r15
}


void freeThreadResources(Thread* thread) {
    printf("thread's #%d resources are released\n", thread->id);
    threadMap[thread->id] = NULL;
    removeFromDead(thread);
    freeId(thread->id);
    buddyVFree(thread->stackMem, STACK_SIZE_ORDER);
    fixedFree(thread);
}


void joinThread(uint16_t id) {
    uint64_t RFLAGS = atomicBegin();

    printf("joins thread #%d\n", id);

    Thread* thread = threadMap[id];
    bool catched = false;
    if (thread && !thread->joined) {
        thread->joined = true;
        catched = true;
    }

    atomicEnd(RFLAGS);

    if (catched) {
        threadMap[id] = NULL;
        while (!thread->isDead);
        freeThreadResources(thread);
    }
}

// ---- scheduler -----

// threads queue
Thread* threadsQueueFirst;
Thread* threadsQueueLast;

// timer
uint8_t TIME_COUNTER;

void initThreadScheduler() {
    threadsQueueFirst = MAIN_THREAD;
    threadsQueueFirst->next = threadsQueueLast = createCleaner();

    // enables interrupts, so from there the scheduler starts his work
    __asm__ volatile("sti");

    printf("\ncleaner started in thread #%d\n");
}


void printAliveThreads() {
    uint64_t RFLAGS = atomicBegin();

    for (Thread* t = threadsQueueFirst; t; t = t->next) {
        printf("%d -> ", t->id);
    }
    printf("NULL\n");

    atomicEnd(RFLAGS);
}


void printDeadThreads() {
    uint64_t RFLAGS = atomicBegin();

    for (Thread* t = deadThreads; t; t = t->next) {
        printf("%d -> ", t->id);
    }
    printf("NULL\n");

    atomicEnd(RFLAGS);
}


void addThreadToTaskQueue(Thread* thread) {
    uint64_t RFLAGS = atomicBegin();

    threadsQueueLast->next = thread;
    threadsQueueLast = thread;
    thread->next = NULL;

    atomicEnd(RFLAGS);
}


void changeCurrentThread() {
    ++TIME_COUNTER;
    if (TIME_COUNTER > TIME_QUANT) {
        TIME_COUNTER = 0;

        Thread* oldCurrent = threadsQueueFirst;

        printAliveThreads();
        printDeadThreads();

        if (!oldCurrent->isDead) {
            threadsQueueFirst = oldCurrent->next;
            oldCurrent->next = NULL;
            threadsQueueLast->next = oldCurrent;
            threadsQueueLast = oldCurrent;
        } else {
            while (threadsQueueFirst->isDead) {
                Thread *thread = threadsQueueFirst;
                threadsQueueFirst = thread->next;
                if (deadThreads) {
                    deadThreads->prev = thread;
                }
                thread->next = deadThreads;
                deadThreads = thread;
            }
        }

        printAliveThreads();
        printDeadThreads();

        if (oldCurrent != threadsQueueFirst) {
            printf("switch: thread #%d (stored at %llx) --> thread #%d (stored at %llx)\n", oldCurrent->id, oldCurrent, threadsQueueFirst->id, threadsQueueFirst);
            switch_threads(&oldCurrent->stackPtr, threadsQueueFirst->stackPtr);
        }
    }
}

void killThread() {
    uint64_t RFLAGS = atomicBegin();

    threadsQueueFirst->isDead = true;
    TIME_COUNTER = 100;     // guarantees that will be handled

    atomicEnd(RFLAGS);

    __asm__ volatile("int $32");
}

void killThreadById(uint16_t id) {
    uint64_t RFLAGS = atomicBegin();

    threadMap[id]->isDead = true;
    TIME_COUNTER = 100;     // guarantees that will be handled

    atomicEnd(RFLAGS);

    __asm__ volatile("int $32");
}

uint16_t getCurrentId() {
    uint64_t RFLAGS = atomicBegin();

    uint16_t id = threadsQueueFirst->id;

    atomicEnd(RFLAGS);

    return id;
}

Thread* createCleaner() {
    Thread* thread = fixedAllocate(THREAD_ALLOCATOR);
    thread->id = allocId();
    thread->stackMem = buddyVAlloc(STACK_SIZE_ORDER);
    thread->isDead = false;
    thread->joined = false;
    thread->next = NULL;
    initThreadStack(thread, (uint64_t) cleaner, NULL);

    return thread;
}

void cleaner(void* ignored) {
    while (ignored == NULL) {  // else I have unused variable
        uint64_t RFLAGS = atomicBegin();

        Thread* thread = deadThreads;
        deadThreads = (thread ? thread->next : NULL);

        atomicEnd(RFLAGS);

        if (thread) {
            joinThread(thread->id);
        } else {
            TIME_COUNTER = 100;
            __asm__ volatile ("int $32");
        }
    }
}

void cut(Thread* thread) {
    if (thread->next) {
        thread->next->prev = thread->prev;
    }
    if (thread->prev) {
        thread->prev->next = thread->next;
    }
}

void removeFromDead(Thread* thread) {
    cut(thread);
    if (deadThreads == thread) {
        deadThreads = thread->next;
    }
}