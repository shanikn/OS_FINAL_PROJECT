#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "plugin_sdk.h"
#include "plugin_common.h"

// expander: Inserts a single white space between each character in the string. 

/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char* plugin_get_name(void){
    return "expander";
}


// transformation function
const char* plugin_transform(const char* input){
    int len= strlen(input);
    
    // allocate *twice* the memory for the copy of input (so we can later on free the original input from memory)
    char* result= malloc((len*2)+1);
    // error allocating memory: return NULL
    if(result == NULL){
        return NULL;
    }
    
    // add a single space after each char from input (except for the last char) 
    for(int i=0; i<len-1; i++){
        result[2*i]=input[i];
        result[(2*i)+1]=' ';
    }

    // add the last char from input and then the null terminator
    result[2*(len-1)]=input[len-1];
    result[2*(len-1)+1]='\0';

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
    return common_plugin_init(plugin_transform, "expander", queue_size);
}
