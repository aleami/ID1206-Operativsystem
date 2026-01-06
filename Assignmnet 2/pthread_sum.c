#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
     
int num_threads = 0;
int size = 1000000;
float *array;
float *partial_sums;
struct timeval timer;
     
void *thread_func(void *arg); /* the thread function */

double mytime() {             
    gettimeofday(&timer, NULL);
    return timer.tv_sec + timer.tv_usec / 1000000.0;
}

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);

     /* Initialize an array of random values */ 
     array = malloc(sizeof(float) * size);
     for (int i = 0; i < size; i++) {
          array[i] = (float) rand() / RAND_MAX;
          }

     /* Perform Serial Sum */
     float  sum_serial = 0.0;
     double time_serial = 0.0;
     //Timer Begin
     double t1 = mytime();
     for (int i = 0; i < size; i++) {
          sum_serial += array[i];
     }
     //Timer End
     double t2 = mytime();
     time_serial = t2 - t1;
     printf("Serial Sum = %f, time = %.3f \n", sum_serial, time_serial);

     /* Create a pool of num_threads workers and keep them in workers */ 
     pthread_t *workers = malloc(sizeof(pthread_t) * num_threads);
     int *thread_id = malloc(sizeof(int) * num_threads);
     partial_sums = malloc(sizeof(float) * num_threads);

     double time_parallel = 0.0;
     double sum_parallel = 0.0;

     //Timer Begin
     double t3 = mytime();

     for (int i = 0; i < num_threads; i++) { 
          thread_id[i] = i;
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, &thread_id[i]); 
     }
     
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

      for (int i = 0; i < num_threads; i++)
        sum_parallel += partial_sums[i];


     //Timer End
     double t4 = mytime();
     time_parallel = t4 - t3;
     printf("Parallel Sum = %f, time = %.3f \n", sum_parallel, time_parallel);
     
     /*free up resources properly */
     free(array);
     free(partial_sums);
     free(workers);
     free(thread_id);
}

void *thread_func(void *arg) { 
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     int my_id = *(int *)arg;

     int range = size / num_threads;
     int start = my_id * range;
     int end = start + range;
     
     if (my_id == num_threads - 1) {
          end = size;
          }

     /* Perform Partial Parallel Sum Here */
     float my_sum = 0.0;
     for (int i = start; i < end; i++) {
        my_sum += array[i];
    }
    partial_sums[my_id] = my_sum;

     printf("Thread %d sum = %f\n", my_id, my_sum);
     pthread_exit(0);
}