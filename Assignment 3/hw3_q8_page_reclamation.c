#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

typedef struct page {
    int page_id;
    int reference_bit;   
    // @other auxiliary
    int ref_check;  
    struct page *next;
} Node;

/* globala variabler */
int n;
int m;
int ref_string[1000];
int done = 0;
int nr = 1000;

// @create an active list
Node* active_list = NULL;

// @create an inactive list
Node* inactive_list = NULL;

Node** pages = NULL;

/* Mutex för att skydda åtkomst till listorna mellan threads. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


/* Lägger till en nod sist i en given lista. */
void enqueue(Node** list, Node* node) {
    if (*list == NULL){
          *list = node;
     }else{
          Node* pointer = *list;
          while(pointer->next != NULL){
               pointer = pointer->next; 
          }
          pointer->next = node;
          node->next = NULL;
     }
}

/* Tar bort och returnerar den första noden i listan. */
Node* dequeue(Node** list) {
    if (*list == NULL){
          return NULL; 
     }
     Node* first_element = *list;
     *list = first_element->next;
     first_element->next = NULL;
     return first_element; 
}

/* Flyttar en specifik sida till slutet av samma lista */
void move_page_rear(Node** list, Node* node) {
     if (*list == NULL){
          return;
     }
     Node* curr_element = *list;
     Node* prev_element = NULL;

     while (curr_element != NULL && curr_element != node) {
          prev_element = curr_element;
          curr_element = curr_element->next;
     }
     if (prev_element == NULL)
     *list = node->next;
     else{
          prev_element->next = node->next;
     }
     node->next = NULL;
     enqueue(list, node);
}

void *player_thread_func() {
     int page_id;

    // @Add code for player
    for (int i = 0; i < nr; i++) {
        page_id = ref_string[i];
        Node* page = pages[page_id];

        pthread_mutex_lock(&mutex);

       
        page->reference_bit = 1;

        int inactive = 0;
        
        /* Kontrollera om sidan ligger i inactive list. */
        Node* p = inactive_list;
        while (p != NULL) {
            if (p == page) {
                inactive = 1;
                break;
            }
            p = p->next;
        }

        /* Om sidan är i inactive då flytta till active. Annars är den redan aktiv då flytta den till slutet. */
        if (inactive) {
          Node* curr_inactive = inactive_list;
          Node* prev = NULL;
          while (curr_inactive != NULL && curr_inactive != page) {
               prev = curr_inactive;
               curr_inactive = curr_inactive->next;
          }
          
          if (prev == NULL)
               inactive_list = page->next;
          else
               prev->next = page->next;
          page->next = NULL;
          enqueue(&active_list, page);
          } else {
               move_page_rear(&active_list, page);
               }

        /* Räkna antal aktiva sidor. */
        int count = 0;
        Node* count_active_list = active_list;
        while (count_active_list) {
            count++;
            count_active_list = count_active_list->next;
        }

        /* Bestäm om active list är över 70% då flytta 20% sidor. */
        int limit = 0.7 * n;
        int move = 0.2 * n;

        if (count > limit) {
            for (int k = 0; k < move; k++) {
                Node* movedNode = dequeue(&active_list);
                if (!movedNode) break;
                enqueue(&inactive_list, movedNode);
            }
        }

        pthread_mutex_unlock(&mutex);
        /* Vila 10 mikrosekunder*/
        usleep(10);
    }
    
    /* visa att player är klar. */
    done = 1;
    pthread_exit(0);
}

/* Checker thread vaknar varje mikrosekunder, går igenom active list, räknar referenser och nollställer referensbiten. */
void *checker_thread_func() {

    // @Add code for checker
    while (!done) {
        usleep(m);
        pthread_mutex_lock(&mutex);

        Node* curr_active_list = active_list;
        while (curr_active_list) {
            if (curr_active_list->reference_bit == 1)
                curr_active_list->ref_check++;
            curr_active_list->reference_bit = 0;
            curr_active_list = curr_active_list->next;
        }

        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    n = atoi(argv[1]);
    m = atoi(argv[2]);

    // @create a random reference string of N
    srand(time(NULL));
    for (int i = 0; i < nr; i++)
        ref_string[i] = rand() % n;

    pages = malloc(sizeof(Node*) * n);

    /* Allokera alla sidor och lägg dem initialt i inactive list.*/
    for (int i = 0; i < n; i++) {
        Node* p = malloc(sizeof(Node));
        p->page_id = i;
        p->reference_bit = 0;
        p->ref_check = 0;
        p->next = NULL;
        pages[i] = p;

        enqueue(&inactive_list, p);
    }

    /* Create two workers */
    pthread_t player;
    pthread_t checker;

    pthread_create(&player, NULL, player_thread_func, NULL);
    pthread_create(&checker, NULL, checker_thread_func, NULL);

    pthread_join(player, NULL);
    pthread_join(checker, NULL);

    printf("Page_Id, Total_Referenced\n");
    //@Print out the statistics of page references
    for (int i = 0; i < n; i++)
        printf("%d, %d\n", i, pages[i]->ref_check);

    printf("Pages in active list: ");
    //@Print out the list of pages in active list
    Node* p = active_list;
    while (p) {
        printf("%d ", p->page_id);
        p = p->next;
    }

    printf("\nPages in inactive list: ");
    //@Print out the list of pages in inactive list
    p = inactive_list;
    while (p) {
        printf("%d ", p->page_id);
        p = p->next;
    }

    printf("\n");
    /*free up resources properly */
    for (int i = 0; i < n; i++) {
    free(pages[i]);
    }
    free(pages);
    return 0;
}
