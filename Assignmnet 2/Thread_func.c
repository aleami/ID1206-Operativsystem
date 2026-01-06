#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

int num_threads = 0;
int size = 0;
double *array;
struct timeval timer;
int **local_hist; // each thread's private histogram (30 bins)

void *thread_func(void *arg); /* the thread function */

// Returns current time in seconds with microsecond precision
double mytime() {
    gettimeofday(&timer, NULL);
    return timer.tv_sec + timer.tv_usec / 1000000.0;
}

int main(int argc, char *argv[])
{
    num_threads = atoi(argv[1]);   // number of worker threads
    size = atoi(argv[2]);          // size of the data array

    /* Initialize an array of random values */
    array = malloc(sizeof(double) * size);
    for (int i = 0; i < size; i++) {
        array[i] = (double) rand() / RAND_MAX;
    }
    //histogram
    int *hist_serial = calloc(30, sizeof(int));          // 30 histogram bins
    local_hist = malloc(sizeof(int*) * num_threads);     // will hold each thread’s local bins
    double t1 = mytime();  // start timing serial histogram

    for (int i = 0; i < size; i++) {
        int bin = (int)(array[i] * 30);   // map value into a bin
        if (bin == 30) bin = 29;
        hist_serial[bin]++;               // increment serial bin count
    }

    double t2 = mytime();  // end timing
    double serial_time = t2 - t1;

    printf("Serial Histogram (bins 0-29):\n");
    for (int i = 0; i < 30; i++)
        printf("%d ", hist_serial[i]);   // print histo data
    printf("\nSerial time = %f\n\n", serial_time);

    int *hist_parallel = calloc(30, sizeof(int));   // final parallel histogram

    // Allocate and zero-initialize per-thread histograms
    for (int i = 0; i < num_threads; i++){
        local_hist[i] = calloc(30, sizeof(int));  // each thread gets its own 30-bin array
    }
    /* Create a pool of num_threads workers and keep them in workers */
    pthread_t *workers = malloc(sizeof(pthread_t) * num_threads);
    int *thread_id = malloc(sizeof(int) * num_threads);

    double time_parallel = 0.0;

    double t3 = mytime();  // start timing parallel computation

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_id[i] = i;    //unique thread-id
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&workers[i], &attr, thread_func, &thread_id[i]);
    }

    // wait for thread to complete
    for (int i = 0; i < num_threads; i++)
        pthread_join(workers[i], NULL);

    /* Aggregate all local histograms into one global histogram */
    for (int t = 0; t < num_threads; t++) {
        for (int b = 0; b < 30; b++) {
            hist_parallel[b] += local_hist[t][b];
        }
    }


    double t4 = mytime(); // end parallell timing
    double parallel_time = t4 - t3;

    printf("Parallel Histogram (bins 0-29):\n");
    for (int i = 0; i < 30; i++)
        printf("%d ", hist_parallel[i]);  // print parallell histo
    printf("\nParallel time = %f\n", parallel_time);


    /*free up resources properly */
    free(array);
    free(workers);
    free(thread_id);
    free(hist_serial);
    free(hist_parallel);
    for (int i = 0; i < num_threads; i++)
        free(local_hist[i]);
    free(local_hist);
}

void *thread_func(void *arg) {
    /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
    int my_id = *(int *)arg;

    /* Determine this thread's portion of the array */
    int range = size / num_threads;
    int start = my_id * range;
    int end = start + range;

    if (my_id == num_threads - 1) {
        end = size; // last thread handles any leftover elements
    }

    /* Build this thread's local histogram */
    for (int i = start; i < end; i++) {
        int bin = (int)(array[i] * 30);
        if (bin == 30) bin = 29;
        local_hist[my_id][bin]++;
    }
    pthread_exit(0);
}