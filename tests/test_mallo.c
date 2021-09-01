#include <stdio.h>
#include <stdlib.h>

int main(){
    char * orig = "test_malloc pass";

    char * string = malloc(17 * sizeof(char));

    memcpy(string, orig, 17 * sizeof(char));

    printf("%s\n", string);

    return 0;
}