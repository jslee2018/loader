#include <signal.h>
#include <stdio.h>

static void sigsegv_handler(int signal, siginfo_t *si, void * unused){
    return;
}