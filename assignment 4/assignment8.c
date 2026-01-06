#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>

void child(char *child) {
    child[0] = '0'; child[1] = '1'; child[2] = '2'; child[3] = '3'; child[4] = '4';
}

void parent(char *parent) {
    parent[4096] = '5'; parent[4097] = '6'; parent[4098] = '7'; parent[4099] = '8'; parent[4100] = '9';
}

int main() {
    int mmap_fd = open("file_to_map.txt", O_RDWR);

    char *mmaped_ptr = mmap(NULL,  (1024 * 1024), PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, 0);

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child process (pid=%d); mmap address: %p\n", getpid(), mmaped_ptr);
        child(mmaped_ptr);
        printf("Child process read from mmaped_ptr[4096]: ");
        for (int i = 0; i < 5; i++) {
            printf("%c", mmaped_ptr[4096 + i]);
        }
        printf("\n");
        exit(0);
    }else {
        // Parent process
        printf("Parent process (pid=%d); mmap address: %p\n", getpid(), mmaped_ptr);
        parent(mmaped_ptr);
        printf("Parent process read from mmaped_ptr[0]: ");
        for (int i = 0; i < 5; i++) {
            printf("%c", mmaped_ptr[i]);
        }
        printf("\n");
        wait(NULL);
    }

    munmap(mmaped_ptr, (1024 * 1024));
    close(mmap_fd);
    return 0;
}
