# Final Submission Quick-Check (OS Pipeline Project)

## 1. File & Directory Structure
- All required source files present (see checklist)
- No `output/` directory or binaries (.so, executables) in zip
- README file present and correctly formatted

## 2. Build & Run
- `build.sh` works on Ubuntu 24.04 + gcc13, exits non-zero on failure
- All plugins build as `.so` files
- `test.sh` runs and all tests pass (no manual intervention needed)

## 3. Main Application (main.c)
- Accepts: `<queue_size> <plugin1> <plugin2> ...`
- Prints usage and exits code 1 on invalid args
- Loads plugins with `dlopen(RTLD_NOW | RTLD_LOCAL)`
- Handles all errors gracefully (no crash)
- Prints "Pipeline shutdown complete" on success

## 4. Plugin System
- All 6 plugins implement required SDK functions
- Each plugin runs in its own thread
- Plugins can be chained in any order, including repeated usage
- No busy waiting; uses monitors for sync

## 5. Synchronization & Threading
- Monitor and consumer-producer queue implemented as specified
- Proper blocking on full/empty queue
- Graceful shutdown on `<END>`

## 6. Error Handling
- All errors print to stderr
- Exit codes: 0 (success), 1 (args/load), 2 (init)
- No debug prints in final output

## 7. Output
- Only plugin output and final shutdown message on STDOUT
- Errors on STDERR only

## 8. Submission Format
- Zip file named `[firstname]_[lastname]_[id].zip`, no root folder
- README format: `[First Name], [Last Name], [ID]`
- Only `dl` and `pthread` libraries used

## 9. Manual Pre-Submission
- Download zip, unzip, verify files
- Run build and test scripts
- Check README and output format

---
**If all above checks pass, your project is ready for submission!**
