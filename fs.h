#ifndef __FS_H__
#define __FS_H__

#include <stdbool.h>
#include <stdint.h>
#include "lock.h"

typedef struct FSNode FSNode;
typedef struct DataNode DataNode;

struct FSNode {
    char name[256];
    FSNode* right;
    FSNode* child;
    DataNode* data;
    uint64_t size;
    bool isDir;
    bool isOpen;
    Lock lock;
};

struct DataNode {
    uint32_t size;
    DataNode* next;
    void* data;
};

void initFS();

FSNode* find(char* pathname);

void getName(char* pathname, uint32_t begin, char* name);

FSNode* find2(char* pathname, uint32_t end);

FSNode* open(char* pathname);

int findPathEnd(char* pathname);

FSNode* createFile(char* pathname);

void close(FSNode* file);

FSNode* mkdir(char* pathname);

FSNode* readDir(char* pathname);

void read(FSNode* file, uint64_t shift, uint8_t* buf, uint32_t count);

void write(FSNode* file, uint64_t shift, uint8_t* buf, uint32_t count);

void printTree();

void printNode(FSNode* node, uint8_t space);

DataNode* newDataNode();

DataNode* seek(DataNode* start, uint64_t* shift);

#endif /* end of include guard: __FS_H__ */
