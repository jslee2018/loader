#include <signal.h>
#include <stdio.h>
#include "signal.h"

static void sigsegv_handler(int signal, siginfo_t *si, void * unused){
    si->si_addr
    
    return;
}

void register_handler(){
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    
}

