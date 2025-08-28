## 2025-08-28

### main.c
- Changed error message for missing plugin argument to match test suite: now prints "At least one plugin must be specified".
- Removed plugin name validation (is_valid_plugin) so plugin loading fails naturally and prints "Failed to load plugin ..." for unknown plugins, matching test expectations.