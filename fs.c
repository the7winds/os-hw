#include "fs.h"
#include "buddy_alloc.h"
#include "fixed_alloc.h"

FixedAllocator* fsNodeAllocator;
FixedAllocator* dataNodesAllocator;
FSNode* ROOT;


void initFS() {
    fsNodeAllocator = newFixedAllocator(sizeof(FSNode), 4);
    dataNodesAllocator = newFixedAllocator(sizeof(DataNode), 4);

    ROOT = fixedAllocate(fsNodeAllocator);
    ROOT->isDir = true;
    ROOT->isOpen = false;
    ROOT->lock = false;
    ROOT->child = NULL;
    ROOT->right = NULL;
    ROOT->name[0] = '/';
    ROOT->name[1] = 0;
}


void getName(char* pathname, uint32_t begin, char* name) {
    for (int j = 0, i = begin; pathname[i] && pathname[i] != '/'; ++i, ++j) {
        name[j] = pathname[i];
    }
}

// return node from tree
FSNode* find(char* pathname) {
    uint32_t end = findPathEnd(pathname);
    FSNode* dir = find2(pathname, end);
    for (FSNode* file = dir->child; file; file = file->right) {
        int i = 0;
        for (i = 0; pathname[end + (pathname[end] == '/' ? 1 : 0) + i]
            && file->name[i] == pathname[end + (pathname[end] == '/' ? 1 : 0) + i]; ++i);
        if (file->name[i] == pathname[end + (pathname[end] == '/' ? 1 : 0) + i] && file->name[i] == 0) {
            return file;
        }
    }
    return NULL;
}

// return node from tree
FSNode* find2(char* pathname, uint32_t end) {
    FSNode* current = ROOT;
    uint32_t i = 0;
    while (i < end) {
        for (FSNode* node = current->child; node; node = node->right) {
            int j = i;
            for (int k = 0; node->name[k] && pathname[j] != '/' && node->name[k] == pathname[j]; ++k, ++j);
            if (pathname[j] == '/') {
                i = j + 1;
                current = node;
                break;
            }
        }
    }
    return current;
}


FSNode* open(char* pathname) {
    FSNode* file = find(pathname);

    if (file == NULL) {
        file = createFile(pathname);
    }

    printf("try open %llx\n", file);
    while (file->isOpen);

    file->isOpen = true;

    printf("opened %llx\n", file);

    return file;
}


int findPathEnd(char* pathname) {
    int i; // iter
    int p; // prev
    int d; // delim
    for (i = p = d = 0; pathname[i]; ++i) {
        if (pathname[i] == '/') {
            p = d;
            d = i;
        }
    }
    return (i == d ? p : d);
}


FSNode* createFile(char* pathname) {
    int end = findPathEnd(pathname);
    FSNode* dir = find2(pathname, end);

    if (dir && dir->isDir) {
        FSNode* file = fixedAllocate(fsNodeAllocator);
        // fill name field
        getName(pathname, end + (pathname[end] == '/' ? 1 : 0), file->name);
        // add to FS tree
        file->right = dir->child;
        dir->child = file;
        // sets that isn't directory
        file->isDir = false;
        // sets basic size
        file->size = 0;
        // sets data
        file->data = NULL;
        // sets child
        file->child = NULL;
        // sets closed state
        file->isOpen = false;

        file->lock = false;

        return file;
    }
    return NULL;
}


void close(FSNode* file) {
    // printf("close %llx\n", file);
    file->isOpen = false;
}


FSNode* mkdir(char* pathname) {
    uint32_t end = findPathEnd(pathname);

    FSNode* dir = find2(pathname, end);

    if (dir && dir->isDir) {
        FSNode* newDir = fixedAllocate(fsNodeAllocator);
        // fill name field
        getName(pathname, end + (pathname[end] == '/' ? 1 : 0), newDir->name);
        // add to FS tree
        newDir->right = dir->child;
        dir->child = newDir;
        // sets that isn't directory
        newDir->isDir = true;
        // sets basic size
        newDir->size = 0;
        // sets data
        newDir->data = NULL;
        // sets child
        newDir->child = NULL;
        // sets closed state
        newDir->isOpen = false;

        newDir->lock = false;

        return newDir;
    }

    return NULL;
}


FSNode* readDir(char* pathname) {
    FSNode* dir = find(pathname);
    if (dir && dir->isDir) {
        return dir->child;
    }

    return NULL;
}


void read(FSNode* file, uint64_t shift, uint8_t* buf, uint32_t count) {
    lock(&(file->lock));

    if (file->isOpen && file->size >= shift) {
        bool adds = false;
        // printf("FILENAME=%s\tFILE=%llx\tDATA=%llx\tSIZE=%lld\n", file->name, file, file->data, file->size);
        DataNode* data = seek(file->data, &shift);
        // printf("START=%llx BLOCKSIZE=%lld\n", data, data->size);
        if (data != NULL) {
            uint8_t* dest = (uint8_t*) data->data + shift;
            uint8_t* end = (uint8_t*) data->data + data->size;
            for (uint32_t i = 0; data && i < count; ++i, file->size += (adds ? 1 : 0)) {
            //    printf("IN LOOP\n");
                if (dest + i == end) {
            //        printf("GO TO NEXT=%llx\n", data->next);
                    data = data->next;
                    if (data == NULL) {
                        break;
                    }
                    dest = data->data;
                    end = dest + data->size;
                }
                buf[i] = dest[i];
//                buf[i + 1] = 0;
//                printf("%s\n", &buf[i]);
            }
        }
    }

    unlock(&file->lock);
}


void write(FSNode* file, uint64_t shift, uint8_t* buf, uint32_t count) {
    lock(&file->lock);

    if (file->isOpen && shift <= file->size) {
        bool adds = false;
        DataNode* data = seek(file->data, &shift);

        if (data == NULL) {
            file->data = data = newDataNode();
            printf("WRITE TO: N=%s FA=%llx DA=%llx\n", file->name, file, file->data);
            adds = true;
        }

        uint8_t* dest = (uint8_t*) data->data + shift;
        uint8_t* end = (uint8_t*) data->data + data->size;
        for (uint32_t i = 0; i < count; ++i, file->size += (adds ? 1 : 0), data->size += (adds ? 1 : 0)) {
            if (dest + i == end) {
                adds = true;
            }
            if (adds && (uint64_t) dest + i == (uint64_t) data + PAGE_SIZE) {
                    if (data->next == NULL) {
                        data->next = newDataNode();
                        adds = true;
                    }
                data = data->next;
                dest = data->data;
                end = dest + data->size;
            }
            dest[i] = buf[i];
            //  dest[i+1] = 0;
            //  printf("WRITE=\"%s\"\n", &dest[i]);
        }

        for (data = data->next; data != NULL; data = data->next) {
            buddyVFree(data->data, 0);
            fixedFree(data);
        }
    }

    unlock(&file->lock);
}


DataNode* newDataNode() {
    DataNode* node;
    node = fixedAllocate(dataNodesAllocator);
    node->size = 0;
    node->data = buddyVAlloc(0);
    node->next = NULL;
    return node;
}


DataNode* seek(DataNode* start, uint64_t* shift) {
    if (start) {
        DataNode* node;
        for (node = start;
            node && (node->size <= *shift);
            node = node->next, *shift -= node->size);
        return node;
    } else {
        return NULL;
    }
}


void printTree() {
    printf("/\n");
    for (FSNode* child = ROOT->child; child; child = child->right) {
        printNode(child, 1);
    }
}


void printNode(FSNode* node, uint8_t space) {
    for (int i = 0; i < space; ++i) {
        printf("|");
    }
    printf("%s size:%d\n", node->name, node->size);
    for (FSNode* child = node->child; child; child = child->right) {
        printNode(child, space + 1);
    }
}
