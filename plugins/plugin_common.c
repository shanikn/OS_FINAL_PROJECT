#include <stdio.h>   // logging output
#include <stdlib.h>  // malloc, free
#include <string.h>  // strcpy, strlen
#include <pthread.h> // threads
#include "sync/consumer_producer.h"
#include "plugin_sdk.h"

#include "plugin_common.h"

// mutex for init function
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;

// static global to hold the plugin state
static plugin_context_t* g_plugin_context = NULL;

/**
 * Generic consumer thread function
 * This function runs in a separate thread and processes items from the queue
 * @param arg Pointer to plugin_context_t
 * @return NULL
 */
void *plugin_consumer_thread(void *arg){
    // we set the global plugin state variable with the input *arg (typecasting into a pointer to a plugin_contex_t structure)
    plugin_context_t* context = (plugin_context_t*)arg;

    // run forever until we get the shutdown signal
    while (1){
        // get next item from the queue
        char* item = consumer_producer_get(context->queue);

        // if the string item is "<END>", meaning the shutdown signal, we shut down gracfully
        if(strcmp(item, "<END>") == 0){
            // foward shutdown signal to next plugin in the chain- to its function next_place_work (if there is one)
            if(context->next_place_work){
                context->next_place_work("<END>");
            }

            // Signal that THIS plugin is finished processing
            consumer_producer_signal_finished(context->queue);

            // and free it too in case there is no next plugin
            free(item);
            break;
        }

        // we get here in case the item isn't the shutdown signal
        // we need to proccess the item using the plugins transofrmation function:
        const char* result = context->process_function(item);

        // free the input item since we're done with it
        free(item);

        // if transformation failed (returned NULL), skip forwarding
        if(result == NULL){
            continue;
        }

        // forward the result to next plugin in the chain (if there is one)
        if(context->next_place_work){
            context->next_place_work(result);
        }
        else{
            // if this is the last plugin in chain, we need to free the result
            free((void *)result);
        }
    }

    // mark as finished when exiting the loop (update flag)
    context->finished = 1;
    return NULL;
}

/**
 * Print error message in the format [ERROR][Plugin Name] - message
 * @param context Plugin context
 * @param message Error message
 */
void log_error(plugin_context_t *context, const char* message){
    fprintf(stderr, "[ERROR][%s] - %s\n", context->name, message);
}

/**
 * Print info message in the format [INFO][Plugin Name] - message
 * @param context Plugin context
 * @param message Info message
 */
void log_info(plugin_context_t* context, const char* message){
    fprintf(stdout, "[INFO][%s] - %s\n", context->name, message);
}

/**
 * Initialize the common plugin infrastructure with the specified queue size
 * @param process_function Plugin-specific processing function
 * @param name Plugin name
 * @param queue_size Maximum number of items that can be queued
 * @return NULL on success, error message on failure
 */
const char* common_plugin_init(const char* (*process_function)(const char*), const char* name, int queue_size){
    pthread_mutex_lock(&init_mutex);

    // if already initialized (error)
    if(g_plugin_context != NULL){
        pthread_mutex_unlock(&init_mutex);
        return "Plugin already initialized";
    }

    // allocate g_plugin_context with malloc()
    g_plugin_context = malloc(sizeof(plugin_context_t));
    if(g_plugin_context == NULL){
        pthread_mutex_unlock(&init_mutex);
        return "Failed to allocate plugin context";
    }

    // initialize all fields
    g_plugin_context->name = name;
    g_plugin_context->next_place_work = NULL;
    g_plugin_context->process_function = process_function;
    g_plugin_context->initialized = 0;
    g_plugin_context->finished = 0;

    // allocate and initialize the queue
    g_plugin_context->queue = consumer_producer_new(queue_size);
    if(g_plugin_context->queue == NULL){
        free(g_plugin_context);
        g_plugin_context = NULL;
        pthread_mutex_unlock(&init_mutex);
        return "Failed to create consumer-producer queue";
    }

    // start the consumer thread with pthread_create()
    int pthread_result = pthread_create(&g_plugin_context->consumer_thread, NULL, plugin_consumer_thread, g_plugin_context);
    if(pthread_result != 0){
        consumer_producer_free(g_plugin_context->queue);
        free(g_plugin_context);
        g_plugin_context = NULL;
        pthread_mutex_unlock(&init_mutex);
        return "Failed to create consumer thread";
    }

    // update init flag
    g_plugin_context->initialized = 1;

    // final unlock
    pthread_mutex_unlock(&init_mutex);

    // on success
    return NULL;
}

/**
 * Finalize the plugin - drain queue and terminate thread gracefully (i.e. pthread_join)
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_fini(void){
    // check if initiallized
    if(g_plugin_context == NULL || !g_plugin_context->initialized){
        return "Plugin isn't initiallized";
    }

    // send shutdown signal
    const char *place_result = plugin_place_work("<END>");
    if(place_result != NULL){
        return place_result;
    }

    // wait for plugin to finish processing (this should wait for the finished flag)
    const char *wait_result = plugin_wait_finished();
    if(wait_result != NULL){
        return wait_result;
    }

    // now join the thread since it should be finished
    int join_result = pthread_join(g_plugin_context->consumer_thread, NULL);
    if(join_result != 0){
        return "Failed to join consumer thread";
    }

    // clean up resources
    consumer_producer_free(g_plugin_context->queue);
    free(g_plugin_context);
    g_plugin_context= NULL;

    // on success
    return NULL;
}



/**
 * Place work (a string) into the plugin's queue
 * @param str The string to process (plugin takes ownership if it allocates new memory)
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_place_work(const char* str){
    // check if initialized
    if(g_plugin_context == NULL || !g_plugin_context->initialized){
        return "Plugin not initialized";
    }

    // error message
    if(str == NULL){
        return "Input string is NULL";
    }

    // use the queue's put function - it handles copying and blocking
    return consumer_producer_put(g_plugin_context->queue, str);
}



/**
 * Attach this plugin to the next plugin in the chain
 * @param next_place_work Function pointer to the next plugin's place_work function
 */
__attribute__((visibility("default"))) 
void plugin_attach(const char* (*next_place_work)(const char*)){
    if(g_plugin_context != NULL){
        g_plugin_context->next_place_work = next_place_work;
    }
}



/**
 * Wait until the plugin has finished processing all work and is ready to shutdown
 * This is a blocking function used for graceful shutdown coordination
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_wait_finished(void){
    // check if initiallized
    if(g_plugin_context == NULL || !g_plugin_context->initialized){
        return "Plugin not initialized";
    }

    // Use the queue's finished monitor - blocks until signaled
    if(consumer_producer_wait_finished(g_plugin_context->queue) != 0){
        return "Failed to wait for completion";
    }

    // on success
    return NULL;
}