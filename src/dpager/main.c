#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "loadelf.h"
#include "call.h"



int main(int argc, char * argv[], char ** envp){
    char * file_name;
    void * load_addr;
    void * entry;
    void * stack;
    
    if(argc < 2)
        return -1;

    file_name = argv[1];

    register_handler();
    dl_init();

    FILE * file = fopen(file_name, "rb");

    if(!file){
        printf("load file failed\n");
        return -1;
    }

    page_init(file);

    entry = load_elf(file, &load_addr, &stack);

    if(!entry){
        printf("NULL entry\n");
        return 0;
    }

    jump_entry(entry, argc - 1, &argv[1], envp, stack);

    printf("don't print\n");

    return 0;
}