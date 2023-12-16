#include <stdio.h>

#include "mem.h"

int main() {
    uint8_t* ptr = heap_init(4096);

    int *arr[10];
    for (int i = 0; i < 8; ++i) {
        arr[i] = _malloc(1000);
        printf("Allocating arr[%d] = %p\n", i, arr[i]);
    }

    for (int i = 0; i < 8; ++i) {
        _free(arr[i]);
        arr[i] = NULL;
        printf("Allocating arr[%d] = %p\n", i, arr[i]);
    }
    
    return 0;
}