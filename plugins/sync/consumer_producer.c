#include <stdlib.h>    // malloc, free
#include <string.h>    // strcpy, etc.
#include <pthread.h>
#include "monitor.h"

#include "consumer_producer.h"


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

    // to prevent race conditions
    // clean up all monitors and memory if mutex init failed
    if(pthread_mutex_init(&queue->mutex, NULL) != 0){
        monitor_destroy(&queue->not_full_monitor);
        monitor_destroy(&queue->not_empty_monitor);
        monitor_destroy(&queue->finished_monitor);
        free(queue->items);
        return "failed initializing mutex";
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

    // free all items with error checks
    if(queue->items){
        // capacity and not count beacuse of the circular buffer
        for(int i=0; i<queue->capacity; i++){
            // free the item in this index (if there is one)
            if(queue->items[i]){
                free(queue->items[i]);
                queue->items[i]= NULL;
            }
        }
        free(queue->items);
    }
    
    //clean up the mutex (to prevent race conditions)
    pthread_mutex_destroy(&queue->mutex);

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

    // critical section ahead 
    pthread_mutex_lock(&queue->mutex);
    // Wait until queue is not full - keep waiting until space available
    while(queue->count >= queue->capacity){
        // unlock before wait
        pthread_mutex_unlock(&queue->mutex);
        monitor_wait(&queue->not_full_monitor);    
        // and lock back after
        pthread_mutex_lock(&queue->mutex);
    }



    // add item at tail (entry point- next available index)
    queue->items[queue->tail] = strdup(item);

    // error: couldn't add item to queue
    if(!(queue->items[queue->tail])){
        pthread_mutex_unlock(&queue->mutex);
        return "failed adding item to the queue";
    }

    //update other queue properties
    queue->count++;
    queue->tail = (queue->tail + 1) % queue->capacity; // circular buffer causes this calculation method
    
    // Signal that queue is not empty (someone might be waiting)
    monitor_signal(&queue->not_empty_monitor);

    // final unlock
    pthread_mutex_unlock(&queue->mutex);
    
    // on success
    return NULL;
}

/**
 * Remove an item from the queue (consumer) and returns it.
 * Blocks if queue is empty.
 * @param queue Pointer to queue structure
 * @return String item or NULL if error (never NULL for empty queue - blocks instead)
 */
char* consumer_producer_get(consumer_producer_t* queue){
    // error: queue is NULL
    if(!queue){
        return NULL; // Only return NULL on error, not empty queue
    }


    // critical section ahead
    pthread_mutex_lock(&queue->mutex);
    // Wait until queue is not empty (blocks until item available)
    while(queue->count == 0){
        // unlock before wait
        pthread_mutex_unlock(&queue->mutex);
        monitor_wait(&queue->not_empty_monitor);   
        // and lock back
        pthread_mutex_lock(&queue->mutex); 
    }

    // remove an item from the head (where we extract next item)
    char* item = queue->items[queue->head];
    queue->items[queue->head] = NULL;

    //update other queue properties
    queue->count--;
    queue->head = (queue->head + 1) % queue->capacity; //circular buffer

    // Signal that queue is not full (producer might be waiting)
    monitor_signal(&queue->not_full_monitor);

    // final unlock
    pthread_mutex_unlock(&queue->mutex);
    
    // on success - return the item (never NULL for empty queue)
    return item;
}

/**
 * Signal that processing is finished
 * @param queue Pointer to queue structure
 */
void consumer_producer_signal_finished(consumer_producer_t* queue){
    // error: queue is empty
    if(!queue){
        return;
    }

    monitor_signal(&queue->finished_monitor);
}

/**
 * Wait for processing to be finished
 * @param queue Pointer to queue structure
 * @return 0 on success, -1 on error
 */
int consumer_producer_wait_finished(consumer_producer_t* queue){
    // error: queue is empty
    if(!queue){
        return -1;
    }

    // monitor_wait function returns "-1" on error
    return monitor_wait(&queue->finished_monitor);
}