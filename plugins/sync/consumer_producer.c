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
