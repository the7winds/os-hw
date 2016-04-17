# os-hw 04: fs


### fs.h/fs.c

пути пишутся без root'а, т.е. /test -> test

**функция/структура** | **описание**
---|---
`FSNode` | структура, описвающая файл
`DataNode` | структура указывающая на блок памяти файла
`void initFS()` | заводит нужные аллокаторы, инициализирует корень
`FSNode* find(char* pathname)` | ищет по пути файл
`void getName(char* pathname, uint32_t begin, char* name)` | принимает путь и возвращает последнее имя (a/b/c -> c)
`FSNode* find2(char* pathname, uint32_t end)` | ищет по пути файл
`FSNode* open(char* pathname)` | открывает файл, если его нет, создаёт
`int findPathEnd(char* pathname)` | ищет где заканчивается последняя дирректория, содержащая файл
`FSNode* createFile(char* pathname)` | создаёт файл, используется внутри `open`
`void close(FSNode* file)` | закрывает файл
`FSNode* mkdir(char* pathname)` | создаёт директорию
`FSNode* readDir(char* pathname)` | возвращает указатель на первого ребёнка в директории
`void read(FSNode* file, uint64_t shift, uint8_t* buf, uint32_t count)` | читает по сдвигу
`void write(FSNode* file, uint64_t shift, uint8_t* buf, uint32_t count)` | пишет по сдвигу, всё что после сдвига затирается
`void printTree()` | выводит дерево файловой системы
`void printNode(FSNode* node, uint8_t space)` | вспомогательная функция для `printTree`
`DataNode* newDataNode()` | создаёт и инициализирует новую ноду
`DataNode* seek(DataNode* start, uint64_t* shift)` | возвращает указатель на блок соответствующий сдвигу

### initramfs.h / initramfs.c

описаны только добавленные мной функции

**функция/структура** | **описание**
---|---
`void initramfs()` | вызывает все остальные функции, инициализирует файловую систему
`void reserveIsoMemory()` | резервирует память образа
`void parseIso()` | парсит образ
`void* parseRecord(char* iter)` | парсит одну запись и создаёт нужный файл
`void* alignPtr(char* ptr)` | выравнивает указатель на границу 4 байта
`uint32_t getMode(struct cpio_header* header)` | вычисляет режим
`uint32_t getNameLen(struct cpio_header* header)` | вычисляет длину имени
`uint32_t getFileSize(struct cpio_header* header)` | вычисляет размер файла

### fsTest.h/fsTest.c

запускать можно только с переданным qemu файлом testIso!

**функция/структура** | **описание**
---|---
`void fsTest()` | вызывает все остальные функции тестирования
`void readTest01()` | проверяет, что прочитало правильно слово из файла
`void readTest02()` | проверяет, что прочитало правильно слово из файла
`void writeTest()` | делает запись, проверяет, что потом прочитано нужное
`void newFileTest()` | создаёт новый файл и пишет в него
