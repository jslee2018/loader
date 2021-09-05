#include <stdio.h>

int main(){
    int pid = getpid();
    char target[30], buf[4096];

    sprintf(target, "/proc/%d/status", pid);
    FILE* f = fopen(target, "r");
    fread(buf, 1, 4095, f);
    buf[4095] = '\0';
    fclose(f);

    int mem;
    char* ptr = strstr(buf, "VmPeak:");
    sscanf(ptr, "%*s %d", &mem);
    printf("Use %dB\n", mem);

    printf("test pass\n");

    return 0;
} 