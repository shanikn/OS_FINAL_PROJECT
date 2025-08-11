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
//TODO: #7 implement consumer_producer.c functions
const char* consumer_producer_init(consumer_producer_t* queue, int capacity);

void consumer_producer_destroy(consumer_producer_t* queue);

const char* consumer_producer_put(consumer_producer_t* queue, const char* item);

char* consumer_producer_get(consumer_producer_t* queue);

void consumer_producer_signal_finished(consumer_producer_t* queue);

int consumer_producer_wait_finished(consumer_producer_t* queue);


