#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "plugin_sdk.h"
#include "plugin_common.h"

// uppercaser: Converts all alphabetic characters in the string to uppercase. 

/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char* plugin_get_name(void){
    return "uppercase";
}


// IMPLEMENT: plugin_transform unique for each plugin
// transformation function
const char* plugin_transform(const char* input){
    // allocate memory for the copy of input (so we can later on free the original input from memory)
    int len= strlen(input);
    int start=0;
    int end= len-1;
    char tmp;
            
    char* result= malloc(len+1);
    // error allocating memory: return NULL
    if(result == NULL){
        return NULL;
    }

    // copy
    strcpy(result, input);

    // reverse the copy
    while(start<end){
        tmp= result[start];
        result[start]=result[end];
        result[end]=tmp;

        start++;
        end--;
    }
           
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
    return common_plugin_init(plugin_transform, "uppercase", queue_size);
}
