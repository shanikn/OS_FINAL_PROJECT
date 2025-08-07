#ifndef QUEUE_H
#define QUEUE_H

// for MAX_THREAD_NUM access
#include "uthreads.h"  


// lenght of a quantom=1000 microseconds
// fixed capcity (array size) for static queue implementation
#define QUEUE_CAPACITY MAX_THREAD_NUM  

// we use queue as the data structure to maintain the threads in the READY state
// (implemented with a *static array of a fixed size* so this isnt a dynamic memory allocation)
typedef struct{
    int data[QUEUE_CAPACITY];
    int front;
    int rear;
    int size;
}queue_t;

void queue_init(queue_t* q);
int queue_is_empty(queue_t* q);
void queue_push(queue_t* q, int value);
int queue_pop(queue_t* q);

#endif