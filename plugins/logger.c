#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "plugin_sdk.h"
#include "plugin_common.h"

// logger: Logs all strings that pass through to standard output. 

/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char* plugin_get_name(void){
    return "logger";
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
    return common_plugin_init(plugin_transform, "logger", queue_size);
}


// TODO: Export the required plugin interface as described in the plugin_sdk.h section

// TODO: figure out how to export, what does that even mean?