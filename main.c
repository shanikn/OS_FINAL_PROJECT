#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>
#include <pthread.h>

#include "plugins/plugin_sdk.h"

// Helper function to check if a plugin name is valid (one of the 6 allowed)
int is_valid_plugin(const char* name) {
    const char* valid_plugins[] = {"logger", "typewriter", "uppercaser", "rotator", "flipper", "expander"};
    for(int i = 0; i < 6; i++) {
        if(strcmp(name, valid_plugins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function pointer typedefs as specified in PDF
typedef const char* (*plugin_init_func_t)(int);
typedef const char* (*plugin_fini_func_t)(void);
typedef const char* (*plugin_place_work_func_t)(const char*);
typedef void (*plugin_attach_func_t)(const char* (*)(const char*));
typedef const char* (*plugin_wait_finished_func_t)(void);

// Plugin structure exactly as specified in PDF page 8
typedef struct {
    plugin_init_func_t init;
    plugin_fini_func_t fini;
    plugin_place_work_func_t place_work;
    plugin_attach_func_t attach;
    plugin_wait_finished_func_t wait_finished;
    char* name;
    void* handle;
} plugin_handle_t;



// help message as a string variable
const char* help_message =
"Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>\n\n"
"Arguments:\n"
" queue_size\t Maximum number of items in each plugin's queue\n"
" plugin1..N\t Names of plugins to load (without .so extension)\n\n"
"Available plugins:\n"
" logger\t\t - Logs all strings that pass through\n"
" typewriter\t - Simulates typewriter effect with delays\n"
" uppercaser\t - Converts strings to uppercase\n"
" rotator\t - Move every character to the right. Last character moves to the beginning.\n"
" flipper\t - Reverses the order of characters\n"
" expander\t - Expands each character with spaces\n\n"
"Example:\n"
" ./analyzer 20 uppercaser rotator logger\n"
" echo 'hello' | ./analyzer 20 uppercaser rotator logger\n"
" echo '<END>' | ./analyzer 20 uppercaser rotator logger\n";


// helper function: usage print to stdout
void print_usage(){
    fprintf(stdout, "%s\n", help_message);
}



// void print_usage() {
//     printf("Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>\n");
//     printf("Arguments:\n");
//     printf(" queue_size Maximum number of items in each plugin's queue\n");
//     printf(" plugin1..N Names of plugins to load (without .so extension)\n");
//     printf("Available plugins:\n");
//     printf(" logger - Logs all strings that pass through\n");
//     printf(" typewriter - Simulates typewriter effect with delays\n");
//     printf(" uppercaser - Converts strings to uppercase\n");
//     printf(" rotator - Move every character to the right. Last character moves to the beginning.\n");
//     printf(" flipper - Reverses the order of characters\n");
//     printf(" expander - Expands each character with spaces\n");
//     printf("Example:\n");
//     printf(" ./analyzer 20 uppercaser rotator logger\n");
//     printf(" echo 'hello' | ./analyzer 20 uppercaser rotator logger\n");
//     printf(" echo '<END>' | ./analyzer 20 uppercaser rotator logger\n");
// }

int main(int argc, char *argv[]){
    // Step 1: Parse and validate command-line arguments
    if(argc < 3) {
        fprintf(stderr, "At least one plugin must be specified\n");
        print_usage();
        return 1;
    }
    
    // Parse queue_size robustly and ensure positive integer
    char* endptr = NULL;
    long qs = strtol(argv[1], &endptr, 10);
    if(endptr == argv[1] || *endptr != '\0' || qs <= 0) {
        fprintf(stderr, "Queue size must be positive\n");
        print_usage();
        return 1;
    }
    int queue_size = (int)qs;
    
    // Validate plugin names
    for(int i = 2; i < argc; i++) {
        if(!is_valid_plugin(argv[i])) {
            fprintf(stderr, "Unknown plugin: %s\n", argv[i]);
            print_usage();
            return 1;
        }
    }
    
    int num_plugins = argc - 2;
    
    // Step 2: Load plugin shared objects
    plugin_handle_t* plugins = malloc(num_plugins * sizeof(plugin_handle_t));
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
        plugins[i].fini = dlsym(plugins[i].handle, "plugin_fini");
        plugins[i].place_work = dlsym(plugins[i].handle, "plugin_place_work");
        plugins[i].attach = dlsym(plugins[i].handle, "plugin_attach");
        plugins[i].wait_finished = dlsym(plugins[i].handle, "plugin_wait_finished");
        
        // Get plugin name and store it
        const char* (*get_name)(void) = dlsym(plugins[i].handle, "plugin_get_name");
        if(get_name) {
            const char* plugin_name = get_name();
            plugins[i].name = malloc(strlen(plugin_name) + 1);
            if(plugins[i].name) {
                strcpy(plugins[i].name, plugin_name);
            }
        } else {
            plugins[i].name = malloc(strlen(argv[i + 2]) + 1);
            if(plugins[i].name) {
                strcpy(plugins[i].name, argv[i + 2]);
            }
        }
        
        if(!plugins[i].init || !plugins[i].place_work || !plugins[i].attach || 
           !plugins[i].wait_finished || !plugins[i].fini){
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
            fprintf(stderr, "Plugin %s cleanup failed: %s\n", plugins[i].name ? plugins[i].name : argv[i + 2], error);
        }
        if(plugins[i].name) {
            free(plugins[i].name);
        }
        dlclose(plugins[i].handle);
    }
    free(plugins);
    
    // Step 8: Print success message and exit
    printf("Pipeline shutdown complete\n");
    return 0;
}