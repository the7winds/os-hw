# os-hw 02: memory


### mem_info.h/mem_info.c

**функция/структура** | **описание**
---|---
`MMAPStruct` | структура для хранения информации о блоке памяти
`int initMMAPInfo()` | читает карту памяти
`int MMAP_end(void* ptr)` | проверяет конец карты памяти
`void* getNextPtr(void *ptr)` | возвращает следующий элемент карты памяти
`MMAPStruct* getNodeByPtr(void* nodePtr)` | возвращает указатель на структуру с информацие о блоке памяти
`void printMMAP()` | выводит карту памяти
`int reserveKernelMemory()` | резервирует память ядра
`int reserveMemory(void* begin, uint64_t length)` | резервирует память

### boot_alloc.h/boot_alloc.c

**функция/структура** | **описание**
---|---
`void* boot_alloc(uint64_t length)` | возвращает указатель на блок памяти нужной длины и резервирует её в карте
`void* search_free_memory(uint64_t length)` | ищет первый свободный кусок нужной длины

### buddy_alloc.h/buddy_alloc.c

**функция/структура** | **описание**
---|---
`PageDscrptr` | структура для дескриптора страницы
`int initBuddyAllocator()` | инициализация аллокатора
`void* buddyAlloc(uint8_t order)` | возвращает физический адрес блока размера `2^order * PAGE_SIZE`
`void* buddyVAlloc(uint8_t order)` | возвращает виртуальный адрес блока нужного размера
`void buddyFree(void* ptr, uint8_t order)` | принимает физический адрес и порядок блока и освобождает
`void buddyVFree(void* ptr, uint8_t order)` | принимает виртуальный адрес
`void printOrders()` | функция для отладки, печатает списки нод
`uint64_t getPageNumByPtr(void* ptr)` | возвращает индекс дескриптора по указателю
`void coverMemory()` | покрывает "страницами" до максимального адреса из карты памяти
`void coverBlock(uint64_t begin, uint64_t end, uint32_t curBlockIdx)` | покрывает блок указанный в карте памяти
`int isLess(uint64_t idx, uint64_t order, uint64_t end)` | проверяет не выходит ли блок за пределы доступного блока из карты памяти


### paging.c

**функция/структура** | **описание**
---|---
`void setUpPaging()` | инициализирует paging по требованиям задания
`void clearPage(void* page)` | отчищает физическую страницу
`void ifFeildEmptyCreate(pte_t* field)` | создаёт таблицу если её не было
`void addToTable(pte_t* pml4, uint64_t virt, uint64_t phys)` | добавляет отображает `virt` на `phys` и добавляет это в табилицу `pml4`

### slab_alloc.h/slab_alloc.c

**функция/структура** | **описание**
---|---
`SlabNode` | структура для организации списка данных slab'а
`Slab` | структура для slab'а
основные функции, они выполняют ветвление для выбора функция для slab'а объектов нужного размера, поэтому описание будет совпадать с частными случаями `Slab* newSlab(uint16_t size, uint16_t align)`|
`Slab* newSlab(uint16_t size, uint16_t align)` | возвращает указатель на новый slab
`void* slabAlloc(Slab* slab)` | выделение slab'ом объекта, возращается виртуальный адрес
`void slabFree(void* ptr)` | освобождение памяти
`void deleteSlab(Slab* slab)` | удаление slab'а

### fixed_alloc.h/fixed_alloc.c

**функция/структура** | **описание**
---|---
`FixedAllocator` | структура для аллокатора фиксированного размера
`int initFixedAllocator()` | инициализирует возможность создания аллокаторов и больших slab (создаёт аллокатор для аллокаторов, и аллокатор для больших slab'ов)
`FixedAllocator* newFixedAllocator(uint16_t size, uint16_t align)` | создаёт новый аллокатор
`void* fixedAllocate(FixedAllocator* fixedAllocator)` | аллоцирует блок
`void fixedFree(void* ptr)` | освобождает блок
`void deleteFixedAllocator(FixedAllocator* fixedAllocator)` | удаляет аллокатор
`void moveSlabToEmpty(FixedAllocator* fixedAllocator, Slab* slab)` | перемещает slab в списках внутри аллокатора, а именно в список тех, кто не может аллоцировать
`void moveSlabToPartly(FixedAllocator* fixedAllocator, Slab* slab)` | перемещает slab в списках внутри аллокатора, а именно в список тех, кто может аллоцировать, но уже что-то аллоцировал
`int isSlabEmpty(Slab* slab)` | проверяет, что slab не может аллоцировать
`int isSlabPartly(Slab* slab)` | проверяет, что может аллоцировать, но и уже что-то аллоцировал
`inline int isSlabFull(Slab* slab)` | проверяет, что может аллоцировать и список объетов которые может аллоцировать полон