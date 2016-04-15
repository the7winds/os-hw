#ifndef __INITRAMFS_H__
#define __INITRAMFS_H__

#define S_IFMT	0xF000
#define S_IFDIR	0x4000
#define S_IFREG	0x8000

#define S_ISDIR(mode)	(((mode) & S_IFMT) == S_IFDIR)
#define S_ISREG(mode)	(((mode) & S_IFMT) == S_IFREG)

#define END_OF_ARCHIVE	"TRAILER!!!"

struct cpio_header {
	char magic[6];
	char inode[8];
	char mode[8];
	char uid[8];
	char gid[8];
	char nlink[8];
	char mtime[8];
	char filesize[8];
	char major[8];
	char minor[8];
	char rmajor[8];
	char rminor[8];
	char namesize[8];
	char chksum[8];
} __attribute__((packed));

#include <stdint.h>

struct ModDscrpt {
	uint32_t start;
	uint32_t end;
	char string[4];
	uint32_t reserved;	// is it 4-byte field?
} __attribute__((packed));

typedef struct ModDscrpt ModDscrpt;

void initramfs();

void reserveIsoMemory();

void parseIso();

void* parseRecord(char* iter);

void* alignPtr(char* ptr);

#define CHAR_TO_INT(c) ('0' <= (c) && (c) <= '9' ? (c) - '0' : (c) - 'A' + 10)

uint32_t getMode(struct cpio_header* header);

uint32_t getNameLen(struct cpio_header* header);

uint32_t getFileSize(struct cpio_header* header);

#endif /*__INITRAMFS_H__*/
