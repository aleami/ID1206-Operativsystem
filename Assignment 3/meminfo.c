#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int page_size = getpagesize();
    int num_pages = atoi(argv[1]);
    size_t total_size = num_pages * page_size;
           
    char *memory = malloc(total_size);
    
    // initialise memory
    memset(memory, 0, total_size);

    free(memory);

    return 0;
}