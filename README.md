# os-hw 03: threads


### lock.h/lock.c

**функция/структура** | **описание**
---|---
`Lock` | собственно lock
`void atomicBegin()` | выключает прерывания, чтобы никто не смог вытеснить нас и сохраняет в флаги
`void atomicEnd()` | восстанавливает старые флаги
`void lock(Lock* lock)` | берёт lock
`void lock(Lock* lock)` | освобождает lock

### threads.h/threads.c

*общие функции потоков*

**функция/структура** | **описание**
---|---
`Thread` | структура для потока, содержит указатели на стек с разных концов, id, и указатель на поток, чтобы можно было 
построи ть очередь на списке
`uint16_t allocId()` | выделяет свободный номер
`void freeId(uint16_t id)` | освобождает, выделенный ранее номер
`void initMultithreading()` | инициализирует начальные значения, создаёт структуру для главного потока
`uint16_t createThread(void (*functionPtr)(void*), void* args)` | создаёт и запускает поток, возвращает id
`void initThreadStack(Thread* thread, void* func, void* args)` | вспомогательная функция, инициализирующая стек
нового потока
`void joinThread(uint16_t id)` | дожидается выполнения потока и освобождает его ресурсы
`void killThreadById(uint16_t id)` | снимает поток с выполнения, без освобождения ресурсов

*функции планировщика*

**функция/структура** | **описание**
---|---
`void initThreadScheduler()` | инициализирует планировщик (создаёт очередь, включает прерывания)
`void addThreadToTaskQueue(Thread* thread)` | добавляет поток в очередь
`void changeCurrentThread()` | переключает на следующий в очереди поток
`void switch_threads(void **old_sp, void *new_sp)` | переключение с одного потка на другой (реализация в switch.S)
`uint16_t getCurrentId()` | возвращает id исполняющегося потока

### threadsTest.h/threadsTest.c

**функция/структура** | **описание**
---|---
`void simpleNThreadTest(int n, int delay)` | запускает n потоков, каждый delay раз выводит своё имя
`void delayedPrintIdTask(void* arg)` | функция, исполняемая потоками из `simpleNThreadTest`
`void lockTest(int n, int t)` | запускает n потоков, которые делают в несколько шагов +1 в общую переменную (пытаемся создать data race), каждый по t раз
`void nIncTask(void* arg)` | функция, исполняемая потоками из `lockTest`
`void killByIdTest()` | запускает один поток параллельно исходному, который должен выводить свой id дольше чем запустивший.   заупустивший под не дожидаясь убивает его и join'ит для освобождения ресурсов, в конце выводится сколько успел поработать убитый поток
`KilledArg` | структура хранящая веремя работы, и сколько порботал поток на самом деле в `killByIdTest`
`killedTask(void* arg)` | функция, исполняемая потокам из `killByIdTest`
