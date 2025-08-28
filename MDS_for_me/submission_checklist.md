# 🎯 Final Submission Checklist - OS Assignment

## 📁 FILE STRUCTURE (CRITICAL - -15 points if wrong)
```
[zip root]
├── main.c
├── README                    ⚠️ REQUIRED - 0 points without it
├── build.sh                 ⚠️ MUST WORK - auto 0 if fails
├── test.sh                  📋 Integration tests
├── [any other required source files]
└── plugins/
    ├── plugin_common.c
    ├── plugin_common.h
    ├── plugin_sdk.h
    ├── logger.c
    ├── typewriter.c
    ├── uppercaser.c
    ├── rotator.c
    ├── flipper.c
    ├── expander.c
    └── sync/
        ├── consumer_producer.c
        ├── consumer_producer.h
        ├── monitor.c
        └── monitor.h
```

**DO NOT INCLUDE:**
- `output/` directory
- Any compiled binaries (.so files, executables)

## 🔧 BUILD REQUIREMENTS

### build.sh MUST:
- [x] Start with `set -e` (exit on any gcc failure)
- [x] Compile main.c with `-ldl` flag
- [x] Build all 6 plugins as .so files with:
  ```bash
  gcc -fPIC -shared -o output/${plugin_name}.so \
      plugins/${plugin_name}.c \
      plugins/plugin_common.c \
      plugins/sync/monitor.c \
      plugins/sync/consumer_producer.c \
      -ldl -lpthread
  ```
- [x] Create `output/` directory if not exists
- [x] Exit with non-zero code on any failure
- [x] Work on Ubuntu 24.04 with gcc13

### Required Plugins (.so files):
- [x] logger.so
- [x] typewriter.so  
- [x] uppercaser.so
- [x] rotator.so
- [x] flipper.so
- [x] expander.so

## 🎯 MAIN APPLICATION (main.c)

### Command Line Parsing:
- [x] First arg: queue_size (positive integer)
- [x] Remaining args: plugin names (without .so)
- [x] Print usage on invalid args + exit code 1
- [x] Required usage format:
```
Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>
Arguments:
 queue_size Maximum number of items in each plugin's queue
 plugin1..N Names of plugins to load (without .so extension)
Available plugins:
 logger - Logs all strings that pass through
 typewriter - Simulates typewriter effect with delays
 uppercaser - Converts strings to uppercase
 rotator - Move every character to the right. Last character moves to the beginning.
 flipper - Reverses the order of characters
 expander - Expands each character with spaces
Example:
 ./analyzer 20 uppercaser rotator logger
 echo 'hello' | ./analyzer 20 uppercaser rotator logger
 echo '<END>' | ./analyzer 20 uppercaser rotator logger
```

### Plugin Loading (dlopen/dlsym):
- [x] Use `dlopen` with `RTLD_NOW | RTLD_LOCAL`
- [x] Load functions: init, fini, place_work, attach, wait_finished
- [x] Use `dlerror()` for error messages
- [x] Exit code 1 on loading failure

### Plugin Management:
- [x] Initialize all plugins with queue_size
- [x] Exit code 2 on init failure
- [x] Attach plugins in chain (don't attach last plugin)
- [x] Call wait_finished() in ascending order
- [x] Call fini() on all plugins
- [x] dlclose() all plugins

### Input Processing:
- [x] Use `fgets()` for lines up to 1024 chars
- [x] Remove trailing `\n` before sending to plugins
- [x] Send strings to first plugin via place_work()
- [x] Break loop when exactly `<END>` received
- [x] Assume pure ASCII input

### Final Output:
- [x] Print: "Pipeline shutdown complete"
- [x] Exit code 0 on success

## 🔄 PLUGIN INTERFACE (plugin_sdk.h)

### Required Functions (all plugins MUST export):
- [x] `const char* plugin_get_name(void)`
- [x] `const char* plugin_init(int queue_size)`
- [x] `const char* plugin_fini(void)`
- [x] `const char* plugin_place_work(const char* str)`
- [x] `void plugin_attach(const char* (*next_place_work)(const char*))`
- [x] `const char* plugin_wait_finished(void)`

### Plugin Behaviors:
- [x] **logger**: Print `[logger] <string>` to stdout
- [x] **typewriter**: Print each char with 100ms delay (usleep)
- [x] **uppercaser**: Convert to uppercase
- [x] **rotator**: Move each char right by 1, last wraps to front
- [x] **flipper**: Reverse string order
- [x] **expander**: Insert space between each character

## 🔒 SYNCHRONIZATION REQUIREMENTS

### Monitor (monitor.c/h):
- [x] Wrap mutex + condition variable + signaled flag
- [x] Functions: init, destroy, signal, reset, wait
- [x] Solve race condition where signal sent before wait
- [x] Manual reset only (not auto-reset)

### Consumer-Producer Queue:
- [x] Bounded thread-safe queue with circular buffer
- [x] Block on full queue (producer)
- [x] Block on empty queue (consumer) 
- [x] Use monitors for synchronization
- [x] NO BUSY WAITING allowed
- [x] Support finished signaling

### Threading:
- [x] Each plugin runs in own thread
- [x] Use pthread library
- [x] Graceful shutdown on `<END>`

## 📝 ERROR HANDLING (CRITICAL)

### Must Handle:
- [x] Invalid command line arguments
- [x] Plugin loading failures (dlopen/dlsym)
- [x] Plugin initialization failures
- [x] Memory allocation failures
- [x] Thread creation failures
- [x] Queue operations failures

### Error Reporting:
- [x] Error messages to stderr
- [x] Proper exit codes (0=success, 1=args/loading, 2=init)
- [x] Application MUST NOT CRASH on any input

## 🧪 TESTING (test.sh)

### Required Tests:
- [x] Build project using build.sh
- [x] Test correct plugin chains
- [x] Test invalid arguments
- [x] Test missing plugins
- [x] Test edge cases (empty strings, long strings)
- [x] Clear pass/fail messages
- [x] Exit non-zero on any test failure
- [x] No manual intervention required

## 📋 OUTPUT REQUIREMENTS

### STDOUT Rules:
- [x] ONLY plugin output allowed in STDOUT
- [x] NO DEBUG PRINTS in final submission
- [x] Errors go to stderr only
- [x] Final message: "Pipeline shutdown complete"

### Example Output:
```bash
$ echo -e "hello\n<END>" | ./analyzer 20 uppercaser logger
[logger] HELLO
Pipeline shutdown complete
```

## 📦 SUBMISSION FORMAT

### Zip File:
- [x] Name: `[first_name]_[last_name]_[id].zip`
- [x] NO root directory in zip
- [x] Test on Ubuntu 24.04 + gcc13 before submission

### README File:
- [x] Format: `[First Name], [Last Name], [ID]`
- [x] Example: "John, Doe, 1234567890"

### Libraries:
- [x] ONLY allowed: `dl`, `pthread`
- [x] NO external libraries beyond these

## ⚠️ PENALTY CHECKLIST

Avoid these automatic penalties:
- [ ] Wrong zip structure: -15 points
- [ ] Missing README: 0 points (resubmission needed)
- [ ] build.sh fails: automatic 0 points
- [ ] Code change appeals: -15 points each

## 🎯 PRE-SUBMISSION VERIFICATION

Before submitting:
1. [ ] Download your zip from Moodle
2. [ ] Unzip and verify all files present
3. [x] Run `./build.sh` - must succeed
4. [x] Run `./test.sh` - all tests pass  
5. [x] Test basic functionality manually
6. [x] Verify no debug prints in output
7. [x]Check README format is exact