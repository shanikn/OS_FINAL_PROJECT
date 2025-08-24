#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "plugin_sdk.h"
#include "plugin_common.h"

// rotator: Moves every character in the string one position to the right. 
// The last character wraps around to the front. 


/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
const char* plugin_get_name(void){
    return "rotator";
}


// IMPLEMENT: plugin_transform unique for each plugin
// transformation function
const char* plugin_transform(const char* input);


/**
 * Initialize the plugin with the specified queue size - calls common_plugin_init
 * This function should be implemented by each plugin
 * @param queue_size Maximum number of items that can be queued
 * @return NULL on success, error message on failure
 */
__attribute__((visibility("default")))
const char* plugin_init(int queue_size){
    return common_plugin_init(plugin_transform, "rotator", queue_size);
}


// TODO: Export the required plugin interface as described in the plugin_sdk.h section