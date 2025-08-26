#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>
#include <pthread.h>

#include "plugins/plugin_sdk.h"


// Plugin structure to hold loaded plugin information
typedef struct {
    void* handle;                                    // dlopen handle
    const char* (*init)(int);                      // plugin_init function
    const char* (*place_work)(const char*);       // plugin_place_work function  
    void (*attach)(const char* (*)(const char*)); // plugin_attach function
    const char* (*wait_finished)(void);           // plugin_wait_finished function
    const char* (*fini)(void);                    // plugin_fini function
    const char* (*get_name)(void);                // plugin_get_name function
} plugin_t;

int main(int argc, char *argv[]){
    // Step 1: Parse command-line arguments
    if(argc < 2){
        fprintf(stderr, "Usage: %s <queue_size> [plugin1] [plugin2] ...\n", argv[0]);
        return 1;
    }
    
    int queue_size = atoi(argv[1]);
    if(queue_size <= 0){
        fprintf(stderr, "Queue size must be positive\n");
        return 1;
    }
    
    int num_plugins = argc - 2;
    if(num_plugins == 0){
        fprintf(stderr, "At least one plugin must be specified\n");
        return 1;
    }
    
    // Step 2: Load plugin shared objects
    plugin_t* plugins = malloc(num_plugins * sizeof(plugin_t));
    if(!plugins){
        fprintf(stderr, "Failed to allocate memory for plugins\n");
        return 1;
    }
    
    for(int i = 0; i < num_plugins; i++){
        char plugin_path[256];
        snprintf(plugin_path, sizeof(plugin_path), "output/%s.so", argv[i + 2]);
        
        plugins[i].handle = dlopen(plugin_path, RTLD_NOW | RTLD_LOCAL);
        if(!plugins[i].handle){
            fprintf(stderr, "Failed to load plugin %s: %s\n", argv[i + 2], dlerror());
            // Cleanup previously loaded plugins
            for(int j = 0; j < i; j++){
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
        
        // Load plugin functions
        plugins[i].init = dlsym(plugins[i].handle, "plugin_init");
        plugins[i].place_work = dlsym(plugins[i].handle, "plugin_place_work");
        plugins[i].attach = dlsym(plugins[i].handle, "plugin_attach");
        plugins[i].wait_finished = dlsym(plugins[i].handle, "plugin_wait_finished");
        plugins[i].fini = dlsym(plugins[i].handle, "plugin_fini");
        plugins[i].get_name = dlsym(plugins[i].handle, "plugin_get_name");
        
        if(!plugins[i].init || !plugins[i].place_work || !plugins[i].attach || 
           !plugins[i].wait_finished || !plugins[i].fini || !plugins[i].get_name){
            fprintf(stderr, "Plugin %s missing required functions\n", argv[i + 2]);
            // Cleanup
            for(int j = 0; j <= i; j++){
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 1;
        }
    }
    
    // Step 3: Initialize each plugin
    for(int i = 0; i < num_plugins; i++){
        const char* error = plugins[i].init(queue_size);
        if(error){
            fprintf(stderr, "Failed to initialize plugin %s: %s\n", argv[i + 2], error);
            // Cleanup initialized plugins
            for(int j = 0; j < i; j++){
                plugins[j].fini();
            }
            for(int j = 0; j < num_plugins; j++){
                dlclose(plugins[j].handle);
            }
            free(plugins);
            return 2;
        }
    }
    
    // Step 4: Attach plugins (chain them together)
    for(int i = 0; i < num_plugins - 1; i++){
        plugins[i].attach(plugins[i + 1].place_work);
    }
    
    // Step 5: Read STDIN lines and send to first plugin until <END>
    char line[1025]; // 1024 + 1 for null terminator
    while(fgets(line, sizeof(line), stdin)){
        // Remove newline if present
        line[strcspn(line, "\n")] = '\0';
        
        // Check for shutdown signal
        if(strcmp(line, "<END>") == 0){
            const char* error = plugins[0].place_work("<END>");
            if(error){
                fprintf(stderr, "Failed to send shutdown signal: %s\n", error);
            }
            break;
        }
        
        // Send line to first plugin
        const char* error = plugins[0].place_work(line);
        if(error){
            fprintf(stderr, "Failed to place work: %s\n", error);
        }
    }
    
    // Step 6: Wait for plugins to finish (in order)
    for(int i = 0; i < num_plugins; i++){
        const char* error = plugins[i].wait_finished();
        if(error){
            fprintf(stderr, "Plugin %s wait_finished failed: %s\n", argv[i + 2], error);
        }
    }
    
    // Step 7: Cleanup - plugin_fini, free memory, dlclose
    for(int i = 0; i < num_plugins; i++){
        const char* error = plugins[i].fini();
        if(error){
            fprintf(stderr, "Plugin %s cleanup failed: %s\n", argv[i + 2], error);
        }
        dlclose(plugins[i].handle);
    }
    free(plugins);
    
    // Step 8: Print success message and exit
    printf("Pipeline shutdown complete\n");
    return 0;
}