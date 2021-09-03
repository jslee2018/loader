#include <stdio.h>
#include <stdlib.h>

int jump_entry(void * entry, int argc, char * argv[], char ** envp){

    int result;
    asm volatile(
        "and $0xfffffff0, %%esp \n"
        "sub $0x8, %%esp \n"
        "push %2 \n"
        "push %1 \n"
        "mov %3, %%edi \n"
        "mov $0x0, %%edx \n"
        "jmp %3"
        : "=r" (result)
        : "r" (argc) ,
        "r" (argv) ,
        "r" (entry)
        );
}