#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> // for usleep

#include "plugin_sdk.h"
#include "plugin_common.h"

// typewriter: Simulates a typewriter effect by printing each character with a 100ms delay (you can use the usleep function). 
// Notice, this can cause a “traffic jam”. 
//TODO: make sure there isn't a traffic jam, the implementation is ok


/**
 * Get the plugin's name
 * @return The plugin's name (should not be modified or freed)
 */
__attribute__((visibility("default")))
const char* plugin_get_name(void){
    return "typewriter";
}


// transformation function
const char* plugin_transform(const char* input){
    int len= strlen(input);

    // allocate memory for the copy of input (so we can later on free the original input from memory)
    char* result= malloc(len+1);
    // error allocating memory: return NULL
    if(result == NULL){
        return NULL;
    }
    
    // copy 
    strcpy(result, input);
    
    // printing the plugin name before the loop
    fprintf(stdout, "[typewriter] ");


    // printing each character with a 100ms delay
    for(int i=0; i<len; i++){
        fprintf(stdout,"%c", result[i]);
        usleep(100000); //100 ms
    }

    // don't forget to enter a line at the end
    fprintf(stdout,"\n");
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
    return common_plugin_init(plugin_transform, "typewriter", queue_size);
}
