#include <stdio.h>
#include <stdlib.h>

void bp2(){
    return;
}

int jump_entry(void * entry, int argc, char * argv[], char ** envp){
    // int (*func)() = entry;

    // func();

    int result;
    asm volatile(
        "and $0xfffffff0, %%esp \n"
        "sub $0x8, %%esp \n"
        "push %2 \n"
        "push %1 \n"
        "mov %3, %%edi \n"
        "mov %4, %%edx \n"
        "jmp %3"
        : "=r" (result)
        : "r" (argc) ,
        "r" (argv) ,
        "r" (entry) ,
        "r" (*(int *)(envp - 0x1c))
        );
}