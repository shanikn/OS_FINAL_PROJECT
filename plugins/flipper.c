#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plugin_sdk.h"

const char* plugin_get_name(void);

const char* plugin_init(int queue_size);

const char* plugin_fini(void);

const char* plugin_place_work(const char* str);

void plugin_attach(const char* (*next_place_work)(const char*));

const char* plugin_wait_finished(void);
