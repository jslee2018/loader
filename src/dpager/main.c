#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "loadelf.h"
#include "call.h"
#include "page.h"
#include "signal.h"

void bp(){
    return;
}

int main(int argc, char * argv[], char ** envp){
    bp();
    char * file_name;
    char * file_raw;
    void * load_addr;
    void * entry;
    
    if(argc < 2)
        return -1;
    
    file_name = argv[1];

    register_handler();

    int file = open(file_name, O_RDONLY);

    page_init(file);

    if(file == -1){
        printf("load file failed\n");
        return -1;
    }

    entry = load_elf(file, &load_addr);

    if(!entry){
        printf("NULL entry\n");
        return 0;
    }

    jump_entry(entry, argc - 1, &argv[1], envp);

    printf("don't print\n");

    return 0;
}