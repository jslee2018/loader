#include <signal.h>
#include <stdio.h>
#include "signal.h"

static void sigsegv_handler(int signal, siginfo_t *si, void * unused){
    
    return;
}

void register_handler(){
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    
}

