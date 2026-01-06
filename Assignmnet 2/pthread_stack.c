#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int num_threads = 0;

pthread_mutex_t mutex;
int node_counter = 0;
int thread_id_counter = 0;

typedef struct node
{
     int node_id; // a unique ID assigned to each node
     struct node *next;
} Node;

Node *top; // top of stack

/* Option 1: Mutex Lock */
void push_mutex()
{
     Node *old_node;
     Node *new_node = malloc(sizeof(Node));

     // update top of the stack below
     // assign a unique ID to the new node
     pthread_mutex_lock(&mutex);
     new_node->node_id = node_counter++;
     new_node->next = top;
     top = new_node;
     pthread_mutex_unlock(&mutex);
}

int pop_mutex()
{
     Node *old_node;

     // update top of the stack below
     pthread_mutex_lock(&mutex);
     if (top != NULL)
     {
          old_node = top;
          top = top->next;
          int id = old_node->node_id;
          free(old_node);
          pthread_mutex_unlock(&mutex);
          return id;
     }
     pthread_mutex_unlock(&mutex);
     return -1;
}

/* Option 2: Compare-and-Swap (CAS) */
void push_cas()
{
     Node *old_node;
     Node *new_node = malloc(sizeof(Node));

     // update top of the stack below
     // assign a unique ID to the new node
     new_node->node_id = __sync_fetch_and_add(&node_counter, 1);
     do
     {
          old_node = top;
          new_node->next = old_node;
     } while (!__sync_bool_compare_and_swap(&top, old_node, new_node));
}

int pop_cas()
{
     Node *old_node;
     Node *new_node;

     // update top of the stack below
     do
     {
          old_node = top;
          if (old_node == NULL)
          {
               return -1;
          }
          new_node = old_node->next;
     } while (!__sync_bool_compare_and_swap(&top, old_node, new_node));

     int id = old_node->node_id;
     free(old_node);

     return id;
}

/* the thread function */
void *thread_func(void *arg)
{
     /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
     int opt = (int)(intptr_t)arg;
     int my_id = __sync_fetch_and_add(&thread_id_counter, 1);

     if (opt == 0)
     {
          push_mutex();
          push_mutex();
          pop_mutex();
          pop_mutex();
          push_mutex();
     }
     else
     {
          push_cas();
          push_cas();
          pop_cas();
          pop_cas();
          push_cas();
     }

     printf("Thread %d: exit\n", my_id);
     pthread_exit(NULL);
     return NULL;
}

int main(int argc, char *argv[])
{
     num_threads = atoi(argv[1]);

     /* Option 1: Mutex */
     pthread_t *workers = malloc(sizeof(pthread_t) * num_threads);

     // reset stack and counters
     top = NULL;
     node_counter = 0;
     thread_id_counter = 0;
     pthread_mutex_init(&mutex, NULL);

     for (int i = 0; i < num_threads; i++)
     {
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, (void *)0);
     }

     for (int i = 0; i < num_threads; i++)
          pthread_join(workers[i], NULL);

     // Print out all remaining nodes in Stack
     printf("Mutex: Remaining nodes \n");
     while (top != NULL)
     {
          Node *temp = top;
          top = top->next;
          printf("%d\n", temp->node_id);
          free(temp);
     }

     /* free up resources properly */
     pthread_mutex_destroy(&mutex);

     /* Option 2: CAS */
     // reset stack and counters
     top = NULL;
     node_counter = 0;
     thread_id_counter = 0;

     for (int i = 0; i < num_threads; i++)
     {
          pthread_attr_t attr;
          pthread_attr_init(&attr);
          pthread_create(&workers[i], &attr, thread_func, (void *)1);
     }
     for (int i = 0; i < num_threads; i++)
          pthread_join(workers[i], NULL);

     // Print out all remaining nodes in Stack
     printf("CAS: Remaining nodes \n");

     /* free up resources properly */
     while (top != NULL)
     {
          Node *temp = top;
          top = top->next;
          printf("%d\n", temp->node_id);
          free(temp);
     }
     free(workers);

     return 0;
}
