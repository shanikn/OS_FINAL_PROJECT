#include <stdlib.h>    // malloc, free
#include <string.h>    // strcpy, etc.
#include <pthread.h>
#include "monitor.h"

#include "consumer_producer.h"


 // we use mutex from pthread.h to make sure only 1 thread can access a shared resoutce at a time (to protect the critical section)

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

    // allocate items array + handle error: memory allocation fail
    if(!(queue->items= malloc(capacity* sizeof(char*)))){
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

/**
 * Destroy a consumer-producer queue and free its resources
 * @param queue Pointer to queue structure
 */
void consumer_producer_destroy(consumer_producer_t* queue){
    // the queue is NULL
    if(!queue){
        return;
    }

    free(queue->items);
    
    monitor_destroy(&queue->not_full_monitor);
    monitor_destroy(&queue->not_empty_monitor);
    monitor_destroy(&queue->finished_monitor);
}

/**
 * Add an item to the queue (producer).
 * Blocks if queue is full.
 * @param queue Pointer to queue structure
 * @param item String to add (queue takes ownership)
 * @return NULL on success, error message on failure
 */
const char* consumer_producer_put(consumer_producer_t* queue, const char* item){
    // error: queue is NULL
    if(!queue){
        return "queue is NULL";
    }

    // error: item is NULL
    if(!item){
        return "item is NULL";
    }

    // block the item as long as the queue is full
    while(queue->capacity==queue->count){
        monitor_wait(&queue->not_full_monitor);    
    }

    // add item at tail (entry point- next available index)
    queue->items[queue->tail]=strdup(item);

    // error: couldnt add item to queue
    if(!(queue->items[queue->tail])){
        return "failed adding item to the queue";
    }

    //update other queue properties
    queue->count++;
    queue->tail=(queue->tail+1)%queue->capacity; //circular buffer means the tail can't be out of bound
    if(queue->count==queue->capacity){
        monitor_reset(&queue->not_full_monitor);
    }
    monitor_signal(&queue->not_empty_monitor);
    
    // on success
    return NULL;
}

/**
 * Remove an item from the queue (consumer) and returns it.
 * Blocks if queue is empty.
 * @param queue Pointer to queue structure
 * @return String item or NULL if queue is empty
 */
char* consumer_producer_get(consumer_producer_t* queue){
    // error: can't remove an item from a null queue
    if(!queue){
        return "queue is NULL";
    }

    
}

void consumer_producer_signal_finished(consumer_producer_t* queue);

int consumer_producer_wait_finished(consumer_producer_t* queue);


