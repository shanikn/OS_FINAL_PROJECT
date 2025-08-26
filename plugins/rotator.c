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
__attribute__((visibility("default")))
const char* plugin_get_name(void){
    return "rotator";
}


// transformation function
const char* plugin_transform(const char* input){
    int len= strlen(input);
    
    // allocate the memory for the copy of input (so we can later on free the original input from memory)
    char* result= malloc(len+1);
    // error allocating memory: return NULL
    if(result == NULL){
        return NULL;
    }


    // puts the last char at front
    result[0]=input[len-1];

    // copies every char from the input 1 index after the original
    for(int i=0; i<len-1; i++){
        result[i+1]=input[i];
    }

    // add the null terminator
    result[len]='\0';
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
    return common_plugin_init(plugin_transform, "rotator", queue_size);
}
