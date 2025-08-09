#include <pthread.h>   // mutex, condition variables
#include <stdlib.h>    // malloc/free if needed
#include <stdio.h>     // only if printing errors during testing

#include "monitor.h"

//TODO: #1 implement functions

int monitor_init(monitor_t* monitor){
    if(!monitor){
        return -1;  
    }    

    if(pthread_mutex_init(&monitor->mutex, NULL)!=0){
        return -1;
    }

    if(pthread_cond_init(&monitor->condition, NULL)!=0){
        pthread_mutex_destroy(&monitor->mutex);
        return -1;
    }

    monitor->signaled=0;
    return 0;
}

//TODO: implement monitor_destroy
void monitor_destroy(monitor_t* monitor);

//TODO: implement monitor_signal
void monitor_signal(monitor_t* monitor);

//TODO: implement monitor_reset
void monitor_reset(monitor_t* monitor);

//TODO: implement monitor_wait
int monitor_wait(monitor_t* monitor);

