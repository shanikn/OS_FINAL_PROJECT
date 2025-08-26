#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "plugin_sdk.h"
#include "plugin_common.h"

// logger: Logs all strings that pass through to standard output. 
// prints the string to STDOUT 


/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char* plugin_get_name(void){
    return "logger";
}


// transformation function
const char* plugin_transform(const char* input){
    fprintf(stdout, "[logger] %s\n", input);
    
    // allocate memory for the copy of input (so we can later on free the original input from memory)
    char* result= malloc(strlen(input)+1);
    // error allocating memory: return NULL
    if(result == NULL){
        return NULL;
    }
    // copy
    strcpy(result, input);
    return result;
}


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

