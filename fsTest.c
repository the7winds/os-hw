#include "fsTest.h"
#include "fs.h"
#include "utils.h"

void fsTest() {
    readTest01();
    readTest02();
    writeTest();
    newFileTest();
}

void readTest01() {
    FSNode* file = open("test/file0.txt");

    // printf("%llx\n", &file);

    char buf[4];
    read(file, 0, (uint8_t*) buf, 3);
    buf[3] = 0;
    // printf("%llx\n", file);

    close(file);

    printf("WAIT=\"ttt\"\tREAD=\"%s\"\n", buf);
}

void readTest02() {
    FSNode* file = open("test/t1/file2.txt");

    char buf[10];
    read(file, 0, (uint8_t*) buf, 10);
    buf[9] = 0;

    close(file);

    printf("WAIT=\"aaaaaaaaa\" READ=\"%s\"\n", buf);
}

void writeTest() {
    FSNode* file = open("test/file0.txt");

    char* s = "TEST!";
    write(file, 0, (uint8_t*) s, 6);
    char buf[9];
    read(file, 0, (uint8_t*) buf, 8);
    buf[8] = 0;

    close(file);

    printf("WAIT=\"TEST!\"\tREAD=\"%s\"\n", buf);
}

void newFileTest() {
    FSNode* file = open("new.txt");
    printTree();

    char* s = "NEW!";
    write(file, 0, (uint8_t*) s, 5);
    char buf[9];
    read(file, 0, (uint8_t*) buf, 8);
    buf[8] = 0;

    close(file);

    printf("WAIT=\"NEW!\"\tREAD=\"%s\"\n", buf);
}
