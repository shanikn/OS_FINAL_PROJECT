# 🚨 Critical Issues & Fixes for OS Assignment

## 1. **main.c - Missing Complete Usage Message**
**Problem:** Simple error message instead of required detailed usage from PDF  
**Fix:** Replace `fprintf(stderr, "Usage: %s <queue_size>...")` with complete format from PDF pages 7-8:
```c
printf("Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>\n");
printf("Arguments:\n");
printf(" queue_size Maximum number of items in each plugin's queue\n");
printf(" plugin1..N Names of plugins to load (without .so extension)\n");
printf("Available plugins:\n");
printf(" logger - Logs all strings that pass through\n");
printf(" typewriter - Simulates typewriter effect with delays\n");
printf(" uppercaser - Converts strings to uppercase\n");
printf(" rotator - Move every character to the right. Last character moves to the beginning.\n");
printf(" flipper - Reverses the order of characters\n");
printf(" expander - Expands each character with spaces\n");
printf("Example:\n");
printf(" ./analyzer 20 uppercaser rotator logger\n");
printf(" echo 'hello' | ./analyzer 20 uppercaser rotator logger\n");
printf(" echo '<END>' | ./analyzer 20 uppercaser rotator logger\n");
```

## 2. **test.sh - Empty File (AUTO-FAIL RISK)**
**Problem:** Only comments, no implementation  
**Fix:** Create comprehensive test script:
- Start with `#!/bin/bash` and `set -e`
- Call `./build.sh` first
- Test valid cases: `echo -e "hello\n<END>" | ./output/analyzer 10 logger`
- Test invalid args, missing plugins
- Verify outputs match expected results
- Print PASS/FAIL for each test
- `exit 1` on any failure

## 3. **Error Handling**
**Problem:** Usage printed to stderr, should be stdout per PDF  
**Fix:** Print usage to `stdout` on invalid args, errors to `stderr`

## 4. **Plugin Output Verification**
**Problem:** Need to verify plugin outputs match PDF examples  
**Fix:** Test each plugin behavior matches specification exactly

---
**Priority:** Fix #1 and #2 immediately - they cause automatic failure