#include <stdio.h>

static char * bss_var;

int main(){

    bss_var = "test_bss pass";

    printf("%s\n", bss_var);

    return 0;
}