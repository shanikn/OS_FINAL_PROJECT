#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// <ctype.h> is a standard library in C
#include <ctype.h> // for uppercase


#include "plugin_sdk.h"
#include "plugin_common.h"

// uppercaser: Converts all alphabetic characters in the string to uppercase. 

/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char* plugin_get_name(void){
    return "uppercaser";
}


// transformation function
const char* plugin_transform(const char* input){


    // allocate memory for the copy of input (so we can later on free the original input from memory)
    int len= strlen(input);

    char* result= malloc(len+1);
    // error allocating memory: return NULL
    if(result == NULL){
        return NULL;
    }

    // copy
    strcpy(result, input);
    
    // uppercase each char
    for(int i=0; i<len; i++){
        result[i]=toupper(result[i]);
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
    return common_plugin_init(plugin_transform, "uppercaser", queue_size);
}
