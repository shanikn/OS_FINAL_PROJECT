#include <pthread.h>   // mutex, condition variables
#include <stdlib.h>    // malloc/free if needed
#include <stdio.h>     // only if printing errors during testing

#include "monitor.h"

int monitor_init(monitor_t* monitor){
    // it's NULL
    if(!monitor){
        return -1;  
    }    

    // pthread functions return 0 on success
    if(pthread_mutex_init(&monitor->mutex, NULL)!=0){
        return -1;
    }

    // pthread functions return 0 on success
    if(pthread_cond_init(&monitor->condition, NULL)!=0){
        // destroy the mutex if the condition variable is NULL
        pthread_mutex_destroy(&monitor->mutex);
        return -1;
    }

    monitor->signaled=0;
    return 0;
}

void monitor_destroy(monitor_t* monitor){
    if(!monitor){
        return;  
    }    

    // with pthread destroy functions
    pthread_mutex_destroy(&monitor->mutex);
    pthread_cond_destroy(&monitor->condition);
}

void monitor_signal(monitor_t* monitor){
    if(!monitor){
        return;  
    }    
    pthread_mutex_lock(&monitor->mutex);

    // set the signal to 1
    monitor->signaled=1;
    pthread_cond_broadcast(&monitor->condition);

    pthread_mutex_unlock(&monitor->mutex);
    
}

void monitor_reset(monitor_t* monitor){
    if(!monitor){
        return;  
    }    
    pthread_mutex_lock(&monitor->mutex);

    // reset signal to 0
    monitor->signaled=0;

    pthread_mutex_unlock(&monitor->mutex);
}

int monitor_wait(monitor_t* monitor){
    if(!monitor){
        return -1;  
    }    
    int rc=0;

    pthread_mutex_lock(&monitor->mutex);
    
    while(!monitor->signaled){
        if(pthread_cond_wait(&monitor->condition, &monitor->mutex)!=0){
            pthread_mutex_unlock(&monitor->mutex);
            return -1;
        }
    }

    pthread_mutex_unlock(&monitor->mutex);
    // on success
    return 0;
}
