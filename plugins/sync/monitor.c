#include <pthread.h>   // mutex, condition variables
#include <stdlib.h>    // malloc/free if needed
#include <stdio.h>     // only if printing errors during testing

#include "monitor.h"

int monitor_init(monitor_t* monitor);

void monitor_destroy(monitor_t* monitor);

void monitor_signal(monitor_t* monitor);

void monitor_reset(monitor_t* monitor);

int monitor_wait(monitor_t* monitor);


//TODO: #1 implement functions

