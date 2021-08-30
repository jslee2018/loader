#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "loadelf.h"
#include "call.h"

void bp(){
    return;
}

int main(int argc, char * argv[], char ** envp){
    bp();
    char * file_name;
    int length;
    char * file_raw;
    void * load_addr;
    int test;
    void * entry;
    
    if(argc < 2)
        return -1;
    
    file_name = argv[1];

    int file = open(file_name, O_RDONLY);

    if(file == -1){
        printf("load file failed\n");
        return -1;
    }


    length = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);

    // printf("length: %d\n", length);

    file_raw = mmap(0, length, PROT_READ, MAP_PRIVATE, file, 0);

    if(file_raw == -1)
        printf("mmap failed %d\n", file_raw);

    entry = load_elf(file_raw, &load_addr);

    if(!entry){
        printf("NULL entry\n");
        return 0;
    }

    // printf("entry: %x\n", (void *)entry - load_addr);

    munmap(file_raw, length);

    bp();

    // printf("envp %x %x %x\n", envp, *envp, **envp);

    jump_entry(entry, argc - 1, &argv[1], envp);

    printf("don't print\n");

    return 0;
}