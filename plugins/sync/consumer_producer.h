#ifndef CONSUMER_PRODUCER_H
#define CONSUMER_PRODUCER_H

#include "monitor.h"

/**
 * Consumer-Producer queue structure for thread-safe producer-consumer pattern
 * Now using monitors for simpler implementation
 */
typedef struct
{
    char** items; /* Array of string pointers */
    int capacity; /* Maximum number of items */
    int count; /* Current number of items */
    int head; /* Index of first item */
    int tail; /* Index of next insertion point */
    monitor_t not_full_monitor; /* Monitor for "not full" state */
    monitor_t not_empty_monitor; /* Monitor for "not empty" state */
    monitor_t finished_monitor; /* Monitor for finished signal */
} consumer_producer_t;


/**
 * Initialize a consumer-producer queue
 * @param queue Pointer to queue structure
 * @param capacity Maximum number of items
 * @return NULL on success, error message on failure
 */
const char* consumer_producer_init(consumer_producer_t* queue, int capacity);


/**
 * Destroy a consumer-producer queue and free its resources
 * @param queue Pointer to queue structure
 */
void consumer_producer_destroy(consumer_producer_t* queue); 

/**
 * Add an item to the queue (producer).
 * Blocks if queue is full.
 * @param queue Pointer to queue structure
 * @param item String to add (queue takes ownership)
 * @return NULL on success, error message on failure
 */
const char* consumer_producer_put(consumer_producer_t* queue, const char* item);


/**
 * Remove an item from the queue (consumer) and returns it.
 * Blocks if queue is empty.
 * @param queue Pointer to queue structure
 * @return String item or NULL if queue is empty
 */
char* consumer_producer_get(consumer_producer_t* queue);

/**
 * Signal that processing is finished
 * @param queue Pointer to queue structure
 */
void consumer_producer_signal_finished(consumer_producer_t* queue);

/**
 * Wait for processing to be finished
 * @param queue Pointer to queue structure
 * @return 0 on success, -1 on timeout
 */
int consumer_producer_wait_finished(consumer_producer_t* queue);

#endif