#include <stdlib.h>    // malloc, free
#include <string.h>    // strcpy, etc.
#include <pthread.h>
#include "monitor.h"

#include "consumer_producer.h"

/**
 * we use mutex from pthread.h to make sure only 1 thread can access a shared resoutce at a time (to protect the critical section)

pthread_mutex_t mutex;
if(pthread_mutex_init(&mutex, NULL) != 0){
    return -1;
}
pthread_mutex_lock(&mutex);
// This is the critical section
pthread_mutex_unlock(&mutex);
// To free/destroy mutex
pthread_mutex_destroy(&mutex);
*/

/**
 * Initialize a consumer-producer queue
 * @param queue Pointer to queue structure
 * @param capacity Maximum number of items
 * @return NULL on success, error message on failure
 */
const char* consumer_producer_init(consumer_producer_t* queue, int capacity){
    // error: can't be NULL
    if(!queue){
        return "Queue pointer is NULL";
    }

    // error: invalid capacity
    if(capacity<=0){
        return "capacity must be positive";
    }

    queue->capacity= capacity;
    queue->count= 0;
    queue->head= 0;
    queue->tail= 0;

    // allocate items array
    queue->items= malloc(capacity* sizeof(char*));
    
    // error: memory allocation fail
    if(!queue->items){
        return "failed to allocate memory for items array";
    }

    //initializing all pointers to null
    for(int i=0; i<capacity; i++){
        queue->items[i]= NULL;
    }

    // initialize 3 monitors + handle any errors with *proper cleanup*
    if(monitor_init(&queue->not_full_monitor)!=0){
        free(queue->items);
        return "failed initializing not_full_monitor";
    }

    if(monitor_init(&queue->not_empty_monitor)!=0){
        monitor_destroy(&queue->not_full_monitor);
        free(queue->items);
        return "failed initializing not_empty_monitor";
    }

    if(monitor_init(&queue->finished_monitor)!=0){
        monitor_destroy(&queue->not_full_monitor);
        monitor_destroy(&queue->not_empty_monitor);
        free(queue->items);
        return "failed initializing finished_monitor";
    }

    // on success
    return NULL;
}

void consumer_producer_destroy(consumer_producer_t* queue);

const char* consumer_producer_put(consumer_producer_t* queue, const char* item);

char* consumer_producer_get(consumer_producer_t* queue);

void consumer_producer_signal_finished(consumer_producer_t* queue);

int consumer_producer_wait_finished(consumer_producer_t* queue);


