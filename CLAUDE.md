# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Assignment Overview
**Due:** August 31, 2025 at 23:59 (NO LATE SUBMISSIONS)  
**Individual Assignment** - Modular Multithreaded String Analyzer Pipeline in C  
**Environment:** Ubuntu 24.04, gcc 13, NO external libraries except `dl` and `pthread`

## Build Commands

### Main Build
```bash
./build.sh
```
Builds all plugins as shared libraries (.so files) and the main analyzer executable:
```bash
# Plugin build template (page 19 in PDF)
gcc -fPIC -shared -o output/${plugin_name}.so \
 plugins/${plugin_name}.c \
 plugins/plugin_common.c \
 plugins/sync/monitor.c \
 plugins/sync/consumer_producer.c \
 -ldl -lpthread
```

### Unit Test Build
```bash
./build_unit.sh
```
Builds unit test executables for synchronization components:
- `monitor_test` - Tests the monitor synchronization primitive
- `consumer_producer_test` - Tests the thread-safe queue implementation

### Integration Tests
```bash
./test.sh
```
Comprehensive automated testing script that must test both positive and negative cases.

## Usage Examples
```bash
# Basic pipeline with queue size 20
./analyzer 20 uppercaser rotator logger flipper typewriter

# Input: "hello" → Output: "[logger] OHELL", "[typewriter] LLEHO"
echo "hello\n<END>" | ./analyzer 20 uppercaser logger
```

## Architecture Overview

This is a **multithreaded plugin-based text processing pipeline system** implementing core OS concepts:
- **Multithreading & Synchronization**: Producer-consumer pattern with bounded queues
- **Dynamic Loading**: Runtime plugin loading via `dlopen`/`dlsym` 
- **Shared Object Management**: Plugin system using .so files
- **Modular Design**: Plugins can be chained in any order and reused

### System Behavior
1. Main application dynamically loads plugins specified in command-line arguments
2. Reads input line-by-line from STDIN (max 1024 chars per line)
3. Each plugin runs in own thread, communicates via thread-safe bounded queues
4. `<END>` input triggers graceful shutdown - queues drained, threads terminated cleanly
5. **NO BUSY WAITING** - proper synchronization mechanisms required

### 1. Synchronization Layer (`plugins/sync/`)
- **Monitor** (`monitor.c/h`): Stateful condition variable that "remembers" signals to prevent race conditions where signals sent before waiting are lost
- **Consumer-Producer Queue** (`consumer_producer.c/h`): Thread-safe circular buffer using monitors for blocking put/get operations when full/empty

### 2. Plugin Infrastructure (`plugins/`)
- **Plugin SDK** (`plugin_sdk.h`): Public interface exported by all plugins
- **Plugin Common** (`plugin_common.c/h`): Shared implementation framework providing generic consumer thread, queue management, chain forwarding, graceful shutdown
- **Plugin Context**: Each plugin runs in dedicated thread with isolated queue

### 3. Required Plugins (6 total)
- **logger**: Logs all strings that pass through to standard output
- **typewriter**: Simulates typewriter effect by printing each character with 100ms delay (usleep)
- **uppercaser**: Converts all alphabetic characters to uppercase  
- **rotator**: Moves every character one position right, last wraps to front
- **flipper**: Reverses the order of characters in the string
- **expander**: Inserts single white space between each character

### 4. Main Application (`main.c`)
**8-Step Process:**
1. Parse command-line arguments (queue_size + plugin names)
2. Load plugin shared objects using `dlopen(RTLD_NOW | RTLD_LOCAL)` and `dlsym`
3. Initialize each plugin with `plugin_init(queue_size)`
4. Attach plugins: `plugin[i].attach(plugin[i+1].place_work)`
5. Read STDIN lines with `fgets()`, send to first plugin until `<END>`
6. Wait for plugins to finish: `plugin_wait_finished()` in order
7. Cleanup: `plugin_fini()`, free memory, `dlclose()`
8. Print "Pipeline shutdown complete", exit(0)

## Critical Implementation Requirements

### Error Handling & Exit Codes
- **Code 0**: Success
- **Code 1**: Command-line argument errors, plugin loading failures  
- **Code 2**: Plugin initialization failures
- **Must handle all errors** - application cannot crash!
- Remove all debug/internal logs from final submission - STDOUT only for pipeline output

### Plugin Development Pattern
1. Include `plugin_common.h`
2. Implement transformation function: `const char* plugin_transform(const char* input)`
3. Call `common_plugin_init(plugin_transform, "plugin_name", queue_size)` from `plugin_init()`
4. Export required SDK functions with `__attribute__((visibility("default")))`

### Memory Management
- Plugins must free all allocated memory except string passed to next plugin
- Last plugin in chain must ensure final string is freed
- No memory leaks allowed!

### Current Status  
**Branch:** `main` (95% complete - READY FOR SUBMISSION!)
- ✅ Monitor and Consumer-Producer sync components fully implemented and tested (19/19 unit tests passing)
- ✅ Plugin common infrastructure COMPLETE - all functions implemented and tested
- ✅ All 6 individual plugins implemented and working
- ✅ Main application pipeline orchestration COMPLETE - full 8-step process implemented  
- ✅ Build system working - all plugins compile successfully
- ✅ Comprehensive documentation created (`main_implementation_explanation.md`)
- 🔨 Integration testing (`test.sh`) - FINAL REMAINING TASK

### Testing Commands
```bash
# Unit tests (all passing)
./build_unit.sh && ./output/monitor_test
./build_unit.sh && ./output/consumer_producer_test  
./build_plugin_test.sh && ./output/plugin_common_test

# System build
./build.sh

# Manual integration test examples  
echo -e "hello\n<END>" | ./output/analyzer 10 logger
echo -e "hello\n<END>" | ./output/analyzer 20 uppercaser rotator flipper logger
```

### Submission Requirements
- **Zip structure:** `[firstname]_[lastname]_[id].zip`
- **README:** `[First Name], [Last Name], [ID]`
- **No output directory or binaries in submission**
- **Must work on Ubuntu 24.04 with gcc 13**
- **Comprehensive `test.sh` with positive/negative test cases** - IN PROGRESS

**READY FOR FINAL TESTING AND SUBMISSION!** 🚀