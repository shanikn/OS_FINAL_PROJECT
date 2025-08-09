#ifndef MONITOR_H
#define MONITOR_H

/**
 * monitor is a condition variable that makes sure the signals are send on the right time so the threads won't miss them
 * it's a chain, so if a thread (with a plugin in the chain) misses its signal, then the whole operation for that string is ruined
 *
 * it wraps a mutex and condition variable and includes a signaled state flag. 
 * A monitor can “remember” a signal until it is consumed by a waiting thread
 **/

 /**
 * Monitor structure that can remember its state
 * This solves the race condition where signals sent before waiting are lost
 */
typedef struct
{
 pthread_mutex_t mutex; /* Mutex for thread safety */
 pthread_cond_t condition; /* Condition variable */
 int signaled; /* Flag to remember if monitor was signaled */
} monitor_t;

/**
 * Initialize a monitor
 * @param monitor Pointer to monitor structure
 * @return 0 on success, -1 on failure
 */
int monitor_init(monitor_t* monitor);

/**
 * Destroy a monitor and free its resources 
 * @param monitor Pointer to monitor structure
 */
void monitor_destroy(monitor_t* monitor);

/**
 * Signal a monitor (sets the monitor state)
 * @param monitor Pointer to monitor structure
 */
void monitor_signal(monitor_t* monitor);

/**
 * Reset a monitor (clears the monitor state)
 * @param monitor Pointer to monitor structure
 */
void monitor_reset(monitor_t* monitor);

/**
 * Wait for a monitor to be signaled (infinite wait)
 * @param monitor Pointer to monitor structure
 * @return 0 on success, -1 on error
 */
int monitor_wait(monitor_t* monitor);

#endif