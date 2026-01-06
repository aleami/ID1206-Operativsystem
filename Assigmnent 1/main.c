#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

struct timeval start, end;

void randomize(double *arr, int N) {
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        arr[i] = (double)rand() / RAND_MAX;
    }
}


double sum(double *arr, int start, int end) {
    double tot = 0;
    for (int i = start; i < end; i++) {
        tot += arr[i];
    }
    return tot;
}

double timer(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) +
           (end.tv_usec - start.tv_usec) / 1000000.0;
}


int main(int argc, char *args[]) {
    if (argc != 2) {
    printf("Input: %s <N>\n", args[0]);
    return 1;
    }

    int N = atoi(args[1]);                        
    double *arr = malloc(N * sizeof(double));
    randomize(arr, N);                         

    //two pipes for one-way communication with each child
    int fd1[2], fd2[2];                         
    pipe(fd1);
    pipe(fd2);

    // Start timing before process creation
    gettimeofday(&start, NULL);                 
    
    //child1
    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(fd1[0]);
        // Computes the sum of the first half of the array
        double sum1 = sum(arr, 0, N / 2);
        //result to parent via the pipe
        write(fd1[1], &sum1, sizeof(sum1));
        exit(0);
    }
    
    //child2
    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(fd2[0]);
        // Computes the sum of the second half of the array
        double sum2 = sum(arr, N / 2, N);
        //result to parent via the pipe
        write(fd2[1], &sum2, sizeof(sum2));
        exit(0);
    }
    
    // Closing unused write of both pipes
    close(fd1[1]);
    close(fd2[1]);

    // Receive data from both children 
    double sum1, sum2;
    read(fd1[0], &sum1, sizeof(sum1));
    read(fd2[0], &sum2, sizeof(sum2));

    // Waiting for both child processes to terminate
    wait(NULL);
    wait(NULL);

    double tot = sum1 + sum2;
    printf("\nTotal sum: %.6f\n", tot);

    gettimeofday(&end, NULL);
    double t = timer(start, end);
    printf("Time: %.6f s\n", t);

    free(arr);
    return 0;
}
