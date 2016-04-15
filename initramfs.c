#include "initramfs.h"
#include "fs.h"
#include "utils.h"
#include "mem_info.h"
#include "memory.h"

extern uint32_t mboot_info;
ModDscrpt* fsDscr;

void initramfs() {
    uint32_t flags = *(uint32_t*) ((uint64_t) mboot_info);
    if (flags & (1 << 3)) {
        uint32_t mods_count = *(uint32_t*) ((uint64_t) mboot_info + 20);
        ModDscrpt* mods_addr = (ModDscrpt*) ((uint64_t) (*(uint32_t*) ((uint64_t) mboot_info + 24)));
        for (uint32_t i = 0; i < mods_count; ++i) {
            struct cpio_header* begin = (struct cpio_header*) ((uint64_t) mods_addr[i].start);
            if (begin->magic[0] == '0' &&
                begin->magic[1] == '7' &&
                begin->magic[2] == '0' &&
                begin->magic[3] == '7' &&
                begin->magic[4] == '0' &&
                begin->magic[5] == '1') {
                    printf("find fs module\n");
                    fsDscr = mods_addr + i;
                    fsDscr = va((uint64_t) fsDscr);
                    break;
                }
        }
        if (fsDscr) {
            reserveIsoMemory();
        } else {
            printf("fs module not found\n");
        }
    } else {
        printf("haven't 3rd flag\n");
    }
}


void reserveIsoMemory() {
    void* begin = (void*) ((uint64_t) fsDscr->start);
    uint32_t length = fsDscr->end - fsDscr->start + 1;
    reserveMemory(begin, length);
    printf("fs module memory reserved\n");
}


void parseIso() {
    char* iter = (char*) va((uint64_t) fsDscr->start);
    printf("BEGIN %llx\n", iter);
    while (iter) {
        iter = parseRecord(iter);
    }
}


void* parseRecord(char* iter) {
    struct cpio_header* header = (struct cpio_header*) iter;
    printf("HEADER %llx\n", header);
    if (header->magic[0] == '0'
        && header->magic[1] == '7'
        && header->magic[2] == '0'
        && header->magic[3] == '7'
        && header->magic[4] == '0'
        && header->magic[5] == '1') {

        printf("OK\n");
        // set name ptr
        char* name = (char*) header + sizeof(struct cpio_header);

        uint32_t nameLen = getNameLen(header);
        uint32_t mode = getMode(header);

        char tmp = name[nameLen];
        name[nameLen] = 0;

        if (S_ISREG(mode)) {
            printf("FILE: %s\n", name);
            FSNode* file = open(name);
            uint32_t fileSize = getFileSize(header);
            printf("SIZE %d\n", fileSize);
            uint8_t* data = alignPtr(name + nameLen);
            name[nameLen] = tmp;
            if (fileSize) {
                write(file, 0, data, fileSize);
            }
            close(file);
            return alignPtr((char*)data + fileSize);
        } else if (S_ISDIR(mode)) {
            printf("DIR: %s\n", name);
            mkdir(name);
            name[nameLen] = tmp;
            return alignPtr(name + nameLen);
        }
    }

    return NULL;
}


void* alignPtr(char* ptr) {
    uint64_t n = (uint64_t) ptr;
    return (void*) (((n / 4) + (n % 4 ? 1 : 0)) * 4);
}


uint32_t getNameLen(struct cpio_header* header) {
    uint32_t nameLen = 0;
    for (int i = 0; i < 8; ++i) {
        nameLen = nameLen * 16 + CHAR_TO_INT(header->namesize[i]);
    }
    return nameLen;
}


uint32_t getMode(struct cpio_header* header) {
    uint32_t mode = 0;
    for (int i = 0; i < 8; ++i) {
        mode = mode * 16 + CHAR_TO_INT(header->mode[i]);
    }
    return mode;
}


uint32_t getFileSize(struct cpio_header* header) {
    uint32_t size = 0;
    for (int i = 0; i < 8; ++i) {
        size = size * 16 + CHAR_TO_INT(header->filesize[i]);
    }
    return size;
}
