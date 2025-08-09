#include <stdlib.h>    // malloc, free
#include <string.h>    // strcpy, etc.
#include <pthread.h>
#include "monitor.h"

#include "consumer_producer.h"

//TODO: #7 implement consumer_producer.c functions
const char* consumer_producer_init(consumer_producer_t* queue, int capacity);

void consumer_producer_destroy(consumer_producer_t* queue);

const char* consumer_producer_put(consumer_producer_t* queue, const char* item);

char* consumer_producer_get(consumer_producer_t* queue);

void consumer_producer_signal_finished(consumer_producer_t* queue);

int consumer_producer_wait_finished(consumer_producer_t* queue);


