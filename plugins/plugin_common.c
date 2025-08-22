#include <stdio.h>     // logging output
#include <stdlib.h>    // malloc, free
#include <string.h>    // strcpy, strlen
#include <pthread.h>   // threads
#include "sync/consumer_producer.h"
#include "plugin_sdk.h"

#include "plugin_common.h"
/**
 * What plugin_common.c does:
 * It's the "plugin engine" that handles ALL the boring stuff:
 *  ✅ Queue management
 *  ✅ Thread creation and lifecycle
 *  ✅ Receiving work via plugin_place_work()
 *  ✅ Passing results to the next plugin
 *  ✅ Graceful shutdown
 *  ✅ Error handling
 */

    
// static global to hold the plugin state
static plugin_context_t* g_plugin_context = NULL;



// IMPLEMENT: plugin_consumer_thread
/**
 * Generic consumer thread function
 * This function runs in a separate thread and processes items from the queue
 * @param arg Pointer to plugin_context_t
 * @return NULL
 */
void* plugin_consumer_thread(void* arg){
    // we set the global plugin state variable with the input *arg (typecasting into a pointer to a plugin_contex_t structure)
    plugin_context_t* context= (plugin_context_t*)arg;
    
    // run forever untill we get the shutdown signal
    while(1){
        // get next item from the queue
        char* item = consumer_producer_get(context->queue);

        // if the string item is "<END>", meaning the shutdown signal, we shut down gracfully
        if(strcmp(item, "<END>") == 0){
            // foward shutdown signal to next plugin in the chain- to its function next_place_work (if there is one)
            if(context->next_place_work){
                context->next_place_work("<END>");
            }

            // and free it too in case there is no next plugin
            free(item);
            break;
        }

        // we get here in case the item isn't the shutdown signal
        // we need to proccess the item using the plugins transofrmation function:
        // TODO: finish this after earlier simpler functions it's based on
        const char* result= context->
    }

}


/**
 * Print error message in the format [ERROR][Plugin Name] - message
 * @param context Plugin context
 * @param message Error message
 */
void log_error(plugin_context_t* context, const char* message){
    fprintf(stderr, "[ERROR][%s] - %s\n", context->name, message);
}


// IMPLEMENT: log_info
/**
 * Print info message in the format [INFO][Plugin Name] - message
 * @param context Plugin context
 * @param message Info message
 */
void log_info(plugin_context_t* context, const char* message);


// IMPLEMENT: plugin_get_name
/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char* plugin_get_name(void);


// IMPLEMENT: common_plugin_init
/**
 * Initialize the common plugin infrastructure with the specified queue size
 * @param process_function Plugin-specific processing function
 * @param name Plugin name
 * @param queue_size Maximum number of items that can be queued
 * @return NULL on success, error message on failure
 */
const char* common_plugin_init(const char* (*process_function)(const char*), const char* name, int queue_size){
    // process_function; // Plugin-specific processing function
    

}


// IMPLEMENT: plugin_init
/**
 * Initialize the plugin with the specified queue size - calls common_plugin_init
 * This function should be implemented by each plugin
 * @param queue_size Maximum number of items that can be queued
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_init(int queue_size);


// IMPLEMENT: plugin_fini
/**
 * Finalize the plugin - drain queue and terminate thread gracefully (i.e. pthread_join)
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_fini(void);


// IMPLEMENT: plugin_place_work
/**
 * Place work (a string) into the plugin's queue
 * @param str The string to process (plugin takes ownership if it allocates new memory)
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_place_work(const char* str);


// IMPLEMENT: plugin_attach
/**
 * Attach this plugin to the next plugin in the chain
 * @param next_place_work Function pointer to the next plugin's place_work function
 */
__attribute__((visibility("default")))
void plugin_attach(const char* (*next_place_work)(const char*));


// IMPLEMENT: plugin_wait_finished
/**
 * Wait until the plugin has finished processing all work and is ready to shutdown
 * This is a blocking function used for graceful shutdown coordination
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_wait_finished(void);