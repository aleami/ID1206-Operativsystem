#define _GNU_SOURCE
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char** argv){

    // Använder högupplöst timer (nanosekunder) istället för time_t som är i sekunder
    //time_t start, end;
    struct timespec start, end;


    // Kontrollera att programmet får exakt två argument: antal sidor och alternativ (1 eller 2)
    if (argc != 3) {
        printf("Usage: %s <num_pages> <option>\n", argv[0]);
        printf("  option = 1 (normal pages), 2 (huge pages)\n");
        exit(1);
    }

    int num_pages = atoi(argv[1]);
    int option = atoi(argv[2]);

    int page_size = getpagesize();

    printf("Allocating %d pages of %d bytes \n", num_pages, page_size);

    char *addr;

    // @Add the start of Timer here
    //start = time(NULL);
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Option 1: @Add mmap below to allocate num_pages anonymous pages
    // Option 2: @Add mmap below to allocate num_pages anonymous pages but using
    // Ausing "huge" pages

    if (option == 1) {
        // Allokera num_pages * page_size anonyma sidor med läs/skriv
        addr = (char*) mmap(NULL,
                            num_pages * page_size,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS,
                            -1, 0);

    } else if (option == 2) {
        // Allokera huge pages — kräver root + att huge pages finns i systemet
        addr = (char*) mmap(NULL,
                            num_pages * page_size,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                            -1, 0);
    } else {
        printf("Invalid option.\n");
        exit(1);
    }

    // Kontrollera om mmap misslyckades
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // the code below updates the pages
    char c = 'a';
    for (int i = 0; i < num_pages; i++) {
        addr[i * page_size] = c;
        c++;
    }

    // @Add the end of Timer here
    //end = time(NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);

    // @Add printout of elapsed time in cycles
    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9; //omvandla till nanoseconds

    printf("\n%f seconds time elapsed\n", elapsed);

    for (int i = 0; (i < num_pages && i<16); i++) {
        printf("%c ", addr[i * page_size]);
    }
    printf("\n");

    if (option == 1)
        munmap(addr, num_pages * page_size);
    else
        munmap(addr, num_pages * (2 * 1024 * 1024));

    return 0;
}