#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "signal.h"
#include "page.h"

void bp2(){
    return;
}

static void sigsegv_handler(int signal, siginfo_t *sip, ucontext_t * ucp){
    // printf("fault addr %x\n", (long) sip -> si_addr);
    if(!load_page(sip -> si_addr)){
        printf("segmentation error\n");
        exit(-1);
    }
    
    bp2();
    return;
}

void register_handler(){
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = sigsegv_handler;

    sigaction(SIGSEGV, &sa, NULL);
    
}