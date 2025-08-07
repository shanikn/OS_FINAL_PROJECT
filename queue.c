#include "queue.h"

void queue_init(queue_t* q){
    if(!q) return;
    q->front=0;
    q->rear=0;
    q->size=0;
}

int queue_is_empty(queue_t* q){
    return q->size==0;
}

void queue_push(queue_t* q, int value){
    // check to prevent overflow
    if (q->size == QUEUE_CAPACITY) {
    fprintf(stderr, "system error: queue overflow\n");
    exit(1);
    }

    q->data[q->rear]=value;
    q->rear=(q->rear +1)%QUEUE_CAPACITY;
    q->size++;
}

int queue_pop(queue_t* q){
    // check to not pop from an empty queue
    if (q->size == 0) {
    fprintf(stderr, "system error: queue is empty, can't pop\n");
    exit(1);
    }

    int val=q->data[q->front];
    q->front=(q->front+1)%QUEUE_CAPACITY;
    q->size--;
    return val;
}

