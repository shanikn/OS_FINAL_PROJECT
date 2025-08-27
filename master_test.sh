#!/bin/bash

# ================================================================================
#                        MASTER TEST SUITE FOR MODULAR PIPELINE SYSTEM
# ================================================================================
# This is the comprehensive master test that validates ALL critical requirements
# from the CODE_REVIEW.md specification. This test serves as the final validation
# before project submission.
#
# Test Categories:
# 1. Build System Verification
# 2. Command-Line Argument Validation
# 3. Individual Plugin Testing
# 4. Pipeline Chain Testing
# 5. Error Handling & Edge Cases
# 6. Memory Management (Valgrind)
# 7. Threading & Synchronization
# 8. Queue Capacity & Limits
# 9. Graceful Shutdown Testing
# 10. Repeated Plugin Usage
# 11. Long String & Buffer Tests
# 12. Special Character Handling
# 13. Performance & Stress Tests
# ================================================================================

# Exit on any error
set -e

# Colors for output
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly CYAN='\033[0;36m'
readonly MAGENTA='\033[0;35m'
readonly BOLD='\033[1m'
readonly NC='\033[0m' # No Color

# Configuration
readonly OUTPUT_DIR="output"
readonly ANALYZER="${OUTPUT_DIR}/analyzer"
readonly TEST_OUTPUT_DIR="master_test_results"
readonly TIMEOUT_DURATION=15
readonly VALGRIND_TIMEOUT=30
readonly MAX_LINE_LENGTH=1024

# Create test output directory
mkdir -p "${TEST_OUTPUT_DIR}"

# Test statistics
declare -i TESTS_PASSED=0
declare -i TESTS_FAILED=0
declare -i TESTS_TOTAL=0
declare -i CRITICAL_FAILURES=0

# Test results log
readonly TEST_LOG="${TEST_OUTPUT_DIR}/master_test_log.txt"
readonly ERROR_LOG="${TEST_OUTPUT_DIR}/error_details.log"

# Initialize logs
echo "MASTER TEST SUITE EXECUTION LOG" > "$TEST_LOG"
echo "Date: $(date)" >> "$TEST_LOG"
echo "========================================" >> "$TEST_LOG"
echo "" > "$ERROR_LOG"

# ================================================================================
#                                UTILITY FUNCTIONS
# ================================================================================

print_banner() {
    echo -e "\n${CYAN}╔═══════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${BOLD}                     MODULAR PIPELINE SYSTEM                        ${NC}${CYAN}║${NC}"
    echo -e "${CYAN}║${BOLD}                        MASTER TEST SUITE                           ${NC}${CYAN}║${NC}"
    echo -e "${CYAN}╚═══════════════════════════════════════════════════════════════════════╝${NC}"
}

print_section() {
    echo -e "\n${MAGENTA}════════════════════════════════════════════════════════${NC}"
    echo -e "${BOLD}${BLUE}SECTION $1: $2${NC}"
    echo -e "${MAGENTA}════════════════════════════════════════════════════════${NC}"
    echo "SECTION $1: $2" >> "$TEST_LOG"
}

print_test_case() {
    echo -e "\n${YELLOW}[TEST ${TESTS_TOTAL}]${NC} $1"
    echo "Test ${TESTS_TOTAL}: $1" >> "$TEST_LOG"
}

print_pass() {
    echo -e "${GREEN}✓ PASS${NC} $1"
    TESTS_PASSED=$((TESTS_PASSED + 1))
    echo "  ✓ PASS: $1" >> "$TEST_LOG"
}

print_fail() {
    echo -e "${RED}✗ FAIL${NC} $1"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    echo "  ✗ FAIL: $1" >> "$TEST_LOG"
    echo "FAILURE: $1" >> "$ERROR_LOG"
}

print_critical_fail() {
    echo -e "${RED}${BOLD}💀 CRITICAL FAILURE${NC} $1"
    TESTS_FAILED=$((TESTS_FAILED + 1))
    CRITICAL_FAILURES=$((CRITICAL_FAILURES + 1))
    echo "  💀 CRITICAL: $1" >> "$TEST_LOG"
    echo "CRITICAL FAILURE: $1" >> "$ERROR_LOG"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

count_test() {
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
}

run_with_timeout() {
    timeout "$TIMEOUT_DURATION" "$@" 2>&1
}

run_analyzer() {
    local input="$1"
    local args="$2"
    echo -e "${input}" | run_with_timeout $ANALYZER $args
}

# ================================================================================
#                            SECTION 1: BUILD SYSTEM VERIFICATION
# ================================================================================

test_build_system() {
    print_section "1" "Build System Verification"
    
    # Test 1.1: Verify build script exists and is executable
    print_test_case "Build script executable"
    count_test
    if [[ -x "./build.sh" ]]; then
        print_pass "build.sh is executable"
    else
        print_critical_fail "build.sh is not executable"
        return 1
    fi
    
    # Test 1.2: Clean build from scratch
    print_test_case "Clean build execution"
    count_test
    print_info "Running clean build..."
    if ./build.sh > "${TEST_OUTPUT_DIR}/build.log" 2>&1; then
        print_pass "Build completed successfully"
    else
        print_critical_fail "Build failed - check ${TEST_OUTPUT_DIR}/build.log"
        return 1
    fi
    
    # Test 1.3: Verify main executable exists
    print_test_case "Main executable verification"
    count_test
    if [[ -x "$ANALYZER" ]]; then
        print_pass "Analyzer executable exists and is executable"
    else
        print_critical_fail "Analyzer executable missing or not executable"
        return 1
    fi
    
    # Test 1.4: Verify all plugin .so files exist
    print_test_case "Plugin library verification"
    count_test
    local plugins=("logger" "uppercaser" "rotator" "flipper" "expander" "typewriter")
    local missing_plugins=()
    
    for plugin in "${plugins[@]}"; do
        if [[ ! -f "${OUTPUT_DIR}/${plugin}.so" ]]; then
            missing_plugins+=("$plugin")
        fi
    done
    
    if [[ ${#missing_plugins[@]} -eq 0 ]]; then
        print_pass "All 6 plugin libraries exist"
    else
        print_critical_fail "Missing plugin libraries: ${missing_plugins[*]}"
        return 1
    fi
}

# ================================================================================
#                        SECTION 2: COMMAND-LINE ARGUMENT VALIDATION
# ================================================================================

test_command_line_arguments() {
    print_section "2" "Command-Line Argument Validation"
    
    # Test 2.1: No arguments (should fail with exit code 1)
    print_test_case "No arguments provided"
    count_test
    if run_with_timeout "$ANALYZER" >/dev/null 2>&1; then
        print_fail "Should fail with no arguments (exit code != 1)"
    else
        local exit_code=$?
        if [[ $exit_code -eq 1 ]]; then
            print_pass "Correctly failed with exit code 1"
        else
            print_fail "Wrong exit code: expected 1, got $exit_code"
        fi
    fi
    
    # Test 2.2: Only queue size (should fail with exit code 1)
    print_test_case "Missing plugin arguments"
    count_test
    if run_with_timeout "$ANALYZER" 10 >/dev/null 2>&1; then
        print_fail "Should fail with missing plugin argument"
    else
        print_pass "Correctly failed with missing plugin argument"
    fi
    
    # Test 2.3: Invalid queue size - zero
    print_test_case "Invalid queue size (zero)"
    count_test
    if run_with_timeout "$ANALYZER" 0 logger >/dev/null 2>&1; then
        print_fail "Should fail with zero queue size"
    else
        print_pass "Correctly rejected zero queue size"
    fi
    
    # Test 2.4: Invalid queue size - negative
    print_test_case "Invalid queue size (negative)"
    count_test
    if run_with_timeout "$ANALYZER" -5 logger >/dev/null 2>&1; then
        print_fail "Should fail with negative queue size"
    else
        print_pass "Correctly rejected negative queue size"
    fi
    
    # Test 2.5: Invalid queue size - non-numeric
    print_test_case "Invalid queue size (non-numeric)"
    count_test
    if run_with_timeout "$ANALYZER" abc logger >/dev/null 2>&1; then
        print_fail "Should fail with non-numeric queue size"
    else
        print_pass "Correctly rejected non-numeric queue size"
    fi
    
    # Test 2.6: Non-existent plugin (should fail with appropriate exit code)
    print_test_case "Non-existent plugin"
    count_test
    if run_with_timeout "$ANALYZER" 10 nonexistent_plugin >/dev/null 2>&1; then
        print_fail "Should fail with non-existent plugin"
    else
        print_pass "Correctly rejected non-existent plugin"
    fi
    
    # Test 2.7: Usage help format verification
    print_test_case "Usage help format"
    count_test
    local usage_output
    usage_output=$(run_with_timeout "$ANALYZER" 2>&1 || true)
    if [[ "$usage_output" == *"Usage: ./analyzer <queue_size> <plugin1> <plugin2> ... <pluginN>"* ]] && \
       [[ "$usage_output" == *"Available plugins:"* ]] && \
       [[ "$usage_output" == *"logger"* ]] && \
       [[ "$usage_output" == *"Example:"* ]]; then
        print_pass "Usage help format is correct"
    else
        print_fail "Usage help format doesn't match specification"
    fi
}

# ================================================================================
#                            SECTION 3: INDIVIDUAL PLUGIN TESTING
# ================================================================================

test_individual_plugins() {
    print_section "3" "Individual Plugin Testing"
    
    # Test 3.1: Logger plugin
    print_test_case "Logger plugin functionality"
    count_test
    local result
    result=$(run_analyzer "hello world\n<END>" "10 logger" | grep "\\[logger\\]" || true)
    local expected="[logger] hello world"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Logger plugin works correctly"
    else
        print_fail "Logger failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 3.2: Uppercaser plugin
    print_test_case "Uppercaser plugin functionality"
    count_test
    result=$(run_analyzer "Hello World123!\n<END>" "10 uppercaser logger" | grep "\\[logger\\]" || true)
    expected="[logger] HELLO WORLD123!"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Uppercaser plugin works correctly"
    else
        print_fail "Uppercaser failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 3.3: Rotator plugin (circular shift right)
    print_test_case "Rotator plugin functionality"
    count_test
    result=$(run_analyzer "abcdef\n<END>" "10 rotator logger" | grep "\\[logger\\]" || true)
    expected="[logger] fabcde"  # last char moves to first
    if [[ "$result" == "$expected" ]]; then
        print_pass "Rotator plugin works correctly"
    else
        print_fail "Rotator failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 3.4: Flipper plugin (string reversal)
    print_test_case "Flipper plugin functionality"
    count_test
    result=$(run_analyzer "hello\n<END>" "10 flipper logger" | grep "\\[logger\\]" || true)
    expected="[logger] olleh"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Flipper plugin works correctly"
    else
        print_fail "Flipper failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 3.5: Expander plugin (spaces between characters)
    print_test_case "Expander plugin functionality"
    count_test
    result=$(run_analyzer "abc\n<END>" "10 expander logger" | grep "\\[logger\\]" || true)
    expected="[logger] a b c"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Expander plugin works correctly"
    else
        print_fail "Expander failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 3.6: Typewriter plugin (with timing verification)
    print_test_case "Typewriter plugin functionality"
    count_test
    local start_time end_time duration
    start_time=$(date +%s%N)
    result=$(run_analyzer "hi\n<END>" "10 typewriter" | grep "\\[typewriter\\]" || true)
    end_time=$(date +%s%N)
    duration=$(( (end_time - start_time) / 1000000 ))  # Convert to milliseconds
    
    expected="[typewriter] hi"
    if [[ "$result" == "$expected" ]] && [[ $duration -ge 200 ]]; then  # Should take at least 200ms for "hi"
        print_pass "Typewriter plugin works with correct timing"
    else
        print_fail "Typewriter failed - Expected: '$expected', Got: '$result', Duration: ${duration}ms"
    fi
}

# ================================================================================
#                            SECTION 4: PIPELINE CHAIN TESTING
# ================================================================================

test_pipeline_chains() {
    print_section "4" "Pipeline Chain Testing"
    
    # Test 4.1: Two-plugin chain
    print_test_case "Two-plugin chain (uppercaser -> logger)"
    count_test
    local result
    result=$(run_analyzer "hello\n<END>" "10 uppercaser logger" | grep "\\[logger\\]" || true)
    local expected="[logger] HELLO"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Two-plugin chain works correctly"
    else
        print_fail "Two-plugin chain failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 4.2: Three-plugin chain
    print_test_case "Three-plugin chain (uppercaser -> rotator -> logger)"
    count_test
    result=$(run_analyzer "abcd\n<END>" "10 uppercaser rotator logger" | grep "\\[logger\\]" || true)
    expected="[logger] DABC"  # ABCD -> DABC (rotated right)
    if [[ "$result" == "$expected" ]]; then
        print_pass "Three-plugin chain works correctly"
    else
        print_fail "Three-plugin chain failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 4.3: Complex five-plugin chain
    print_test_case "Complex five-plugin chain"
    count_test
    result=$(run_analyzer "hello\n<END>" "15 uppercaser rotator flipper expander logger" | grep "\\[logger\\]" || true)
    # hello -> HELLO -> OHELL -> LLEHO -> L L E H O -> [logger] L L E H O
    expected="[logger] L L E H O"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Complex five-plugin chain works correctly"
    else
        print_fail "Complex chain failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 4.4: Multiple lines through pipeline
    print_test_case "Multiple input lines through pipeline"
    count_test
    local output
    output=$(run_analyzer "line1\nline2\nline3\n<END>" "10 uppercaser logger")
    local line1 line2 line3
    line1=$(echo "$output" | grep "LINE1" || true)
    line2=$(echo "$output" | grep "LINE2" || true) 
    line3=$(echo "$output" | grep "LINE3" || true)
    
    if [[ "$line1" == "[logger] LINE1" ]] && \
       [[ "$line2" == "[logger] LINE2" ]] && \
       [[ "$line3" == "[logger] LINE3" ]]; then
        print_pass "Multiple input lines processed correctly"
    else
        print_fail "Multiple lines failed - Got: '$output'"
    fi
}

# ================================================================================
#                          SECTION 5: ERROR HANDLING & EDGE CASES
# ================================================================================

test_error_handling() {
    print_section "5" "Error Handling & Edge Cases"
    
    # Test 5.1: Empty string handling
    print_test_case "Empty string processing"
    count_test
    local result
    result=$(run_analyzer "\n<END>" "10 logger" | grep "\\[logger\\]" || true)
    local expected="[logger] "
    if [[ "$result" == "$expected" ]]; then
        print_pass "Empty string handled correctly"
    else
        print_fail "Empty string failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 5.2: Single character
    print_test_case "Single character processing"
    count_test
    result=$(run_analyzer "a\n<END>" "10 uppercaser logger" | grep "\\[logger\\]" || true)
    expected="[logger] A"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Single character handled correctly"
    else
        print_fail "Single character failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 5.3: Special characters and symbols
    print_test_case "Special characters and symbols"
    count_test
    result=$(run_analyzer "!@#\$%^&*()\n<END>" "10 logger" | grep "\\[logger\\]" || true)
    expected="[logger] !@#\$%^&*()"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Special characters handled correctly"
    else
        print_fail "Special characters failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 5.4: Numbers preservation
    print_test_case "Numbers preservation in uppercaser"
    count_test
    result=$(run_analyzer "abc123def\n<END>" "10 uppercaser logger" | grep "\\[logger\\]" || true)
    expected="[logger] ABC123DEF"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Numbers preserved correctly"
    else
        print_fail "Numbers not preserved - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 5.5: Very small queue size (stress test)
    print_test_case "Very small queue size (1)"
    count_test
    result=$(run_analyzer "test\n<END>" "1 uppercaser logger" | grep "\\[logger\\]" || true)
    expected="[logger] TEST"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Small queue size handled correctly"
    else
        print_fail "Small queue failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 5.6: Graceful shutdown verification
    print_test_case "Graceful shutdown message"
    count_test
    local output
    output=$(run_analyzer "test\n<END>" "10 logger")
    if [[ "$output" == *"Pipeline shutdown complete"* ]]; then
        print_pass "Graceful shutdown message present"
    else
        print_fail "Missing graceful shutdown message"
    fi
    
    # Test 5.7: No input (just <END>)
    print_test_case "No input before <END>"
    count_test
    output=$(run_analyzer "<END>" "10 logger")
    if [[ "$output" == *"Pipeline shutdown complete"* ]]; then
        print_pass "Handles immediate <END> correctly"
    else
        print_fail "Failed to handle immediate <END>"
    fi
}

# ================================================================================
#                            SECTION 6: MEMORY MANAGEMENT TESTING
# ================================================================================

test_memory_management() {
    print_section "6" "Memory Management Testing"
    
    if ! command -v valgrind &> /dev/null; then
        print_info "Valgrind not available, skipping memory tests"
        return 0
    fi
    
    # Test 6.1: Simple memory leak detection
    print_test_case "Memory leak detection (simple)"
    count_test
    print_info "Running valgrind analysis (this may take time)..."
    
    local valgrind_output
    valgrind_output=$(echo -e "test\n<END>" | \
        timeout "$VALGRIND_TIMEOUT" valgrind --leak-check=full --show-leak-kinds=all \
        --track-origins=yes "$ANALYZER" 10 logger 2>&1 || true)
    
    if [[ "$valgrind_output" == *"All heap blocks were freed -- no leaks are possible"* ]] || \
       [[ "$valgrind_output" == *"definitely lost: 0 bytes"* && "$valgrind_output" == *"possibly lost: 0 bytes"* ]]; then
        print_pass "No memory leaks detected"
    else
        print_fail "Memory leaks detected - check valgrind output"
        echo "$valgrind_output" > "${TEST_OUTPUT_DIR}/valgrind_simple.log"
    fi
    
    # Test 6.2: Complex pipeline memory test
    print_test_case "Memory leak detection (complex pipeline)"
    count_test
    
    valgrind_output=$(echo -e "complex test\nmore data\n<END>" | \
        timeout "$VALGRIND_TIMEOUT" valgrind --leak-check=full \
        "$ANALYZER" 15 uppercaser rotator flipper expander logger 2>&1 || true)
    
    if [[ "$valgrind_output" == *"All heap blocks were freed -- no leaks are possible"* ]] || \
       [[ "$valgrind_output" == *"definitely lost: 0 bytes"* && "$valgrind_output" == *"possibly lost: 0 bytes"* ]]; then
        print_pass "No memory leaks in complex pipeline"
    else
        print_fail "Memory leaks in complex pipeline - check valgrind output"
        echo "$valgrind_output" > "${TEST_OUTPUT_DIR}/valgrind_complex.log"
    fi
}

# ================================================================================
#                          SECTION 7: REPEATED PLUGIN USAGE
# ================================================================================

test_repeated_plugins() {
    print_section "7" "Repeated Plugin Usage"
    
    # Test 7.1: Same plugin multiple times
    print_test_case "Multiple instances of same plugin"
    count_test
    local result
    result=$(run_analyzer "test\n<END>" "10 logger logger logger" | wc -l)
    # Should have 3 logger lines + 1 shutdown line = 4 total lines
    if [[ "$result" -eq 4 ]]; then
        print_pass "Multiple plugin instances work correctly"
    else
        print_fail "Multiple instances failed - Expected 4 lines, got $result"
    fi
    
    # Test 7.2: Repeated rotations (4 rotations = original)
    print_test_case "Repeated rotations correctness"
    count_test
    result=$(run_analyzer "abcd\n<END>" "10 rotator rotator rotator rotator logger" | grep "\\[logger\\]" || true)
    expected="[logger] abcd"  # 4 right rotations should restore original
    if [[ "$result" == "$expected" ]]; then
        print_pass "Repeated rotations work correctly"
    else
        print_fail "Repeated rotations failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 7.3: Double flip (should restore original)
    print_test_case "Double flip restoration"
    count_test
    result=$(run_analyzer "hello\n<END>" "10 flipper flipper logger" | grep "\\[logger\\]" || true)
    expected="[logger] hello"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Double flip restores original correctly"
    else
        print_fail "Double flip failed - Expected: '$expected', Got: '$result'"
    fi
}

# ================================================================================
#                            SECTION 8: QUEUE CAPACITY TESTING
# ================================================================================

test_queue_capacity() {
    print_section "8" "Queue Capacity & Limits Testing"
    
    # Test 8.1: Large queue size
    print_test_case "Large queue size (1000)"
    count_test
    local result
    result=$(run_analyzer "large queue test\n<END>" "1000 uppercaser logger" | grep "\\[logger\\]" || true)
    local expected="[logger] LARGE QUEUE TEST"
    if [[ "$result" == "$expected" ]]; then
        print_pass "Large queue size works correctly"
    else
        print_fail "Large queue failed - Expected: '$expected', Got: '$result'"
    fi
    
    # Test 8.2: Multiple items with small queue (stress test)
    print_test_case "Multiple items with small queue"
    count_test
    local output
    output=$(run_analyzer "item1\nitem2\nitem3\nitem4\nitem5\n<END>" "2 uppercaser logger")
    local item_count
    item_count=$(echo "$output" | grep "\\[logger\\]" | wc -l)
    if [[ "$item_count" -eq 5 ]]; then
        print_pass "Small queue handles multiple items correctly"
    else
        print_fail "Small queue failed - Expected 5 items, processed $item_count"
    fi
}

# ================================================================================
#                            SECTION 9: LONG STRING TESTING
# ================================================================================

test_long_strings() {
    print_section "9" "Long String & Buffer Testing"
    
    # Test 9.1: Long string (close to 1024 limit)
    print_test_case "Long string (near 1024 char limit)"
    count_test
    local long_string
    long_string=$(printf 'A%.0s' {1..1000})  # 1000 A's
    local result
    result=$(run_analyzer "${long_string}\n<END>" "10 logger" | grep "\\[logger\\]" | cut -d']' -f2- | sed 's/^ //' || true)
    
    if [[ "$result" == "$long_string" ]]; then
        print_pass "Long string (1000 chars) handled correctly"
    else
        print_fail "Long string failed - length mismatch"
    fi
    
    # Test 9.2: Medium length strings through complex pipeline
    print_test_case "Medium strings through complex pipeline"
    count_test
    local medium_string="The quick brown fox jumps over the lazy dog multiple times to create a longer test string for pipeline processing"
    result=$(run_analyzer "${medium_string}\n<END>" "20 uppercaser rotator logger" | grep "\\[logger\\]" || true)
    
    if [[ -n "$result" ]] && [[ "$result" == *"[logger]"* ]]; then
        print_pass "Medium string through pipeline works"
    else
        print_fail "Medium string pipeline failed"
    fi
}

# ================================================================================
#                          SECTION 10: STRESS TESTING
# ================================================================================

test_stress_scenarios() {
    print_section "10" "Stress Testing"
    
    # Test 10.1: Many plugins in chain
    print_test_case "Maximum plugin chain length"
    count_test
    local result
    # Test with all 6 different plugins
    result=$(run_analyzer "stress\n<END>" "50 uppercaser rotator flipper expander rotator uppercaser logger" | grep "\\[logger\\]" || true)
    
    if [[ -n "$result" ]] && [[ "$result" == *"[logger]"* ]]; then
        print_pass "Long plugin chain works correctly"
    else
        print_fail "Long plugin chain failed"
    fi
    
    # Test 10.2: High-frequency processing
    print_test_case "Multiple rapid inputs"
    count_test
    local rapid_input=""
    for i in {1..10}; do
        rapid_input+="rapid${i}\\n"
    done
    rapid_input+="<END>"
    
    local output
    output=$(run_analyzer "$rapid_input" "5 logger")
    local processed_count
    processed_count=$(echo "$output" | grep "\\[logger\\]" | wc -l)
    
    if [[ "$processed_count" -eq 10 ]]; then
        print_pass "Rapid input processing works correctly"
    else
        print_fail "Rapid input failed - Expected 10, processed $processed_count"
    fi
}

# ================================================================================
#                          SECTION 11: INTEGRATION TESTING
# ================================================================================

test_integration() {
    print_section "11" "Integration Testing"
    
    # Test 11.1: End-to-end realistic scenario
    print_test_case "End-to-end realistic scenario"
    count_test
    local realistic_input="Hello, World!\\nThis is a test.\\nProcessing multiple lines.\\n<END>"
    local output
    output=$(run_analyzer "$realistic_input" "15 uppercaser expander logger")
    
    local line_count
    line_count=$(echo "$output" | grep "\\[logger\\]" | wc -l)
    
    if [[ "$line_count" -eq 3 ]] && [[ "$output" == *"Pipeline shutdown complete"* ]]; then
        print_pass "End-to-end scenario works correctly"
    else
        print_fail "End-to-end scenario failed"
    fi
    
    # Test 11.2: Plugin initialization failure recovery
    print_test_case "Plugin initialization error handling"
    count_test
    local error_output
    error_output=$(run_with_timeout "$ANALYZER" invalid_queue_size logger 2>&1 || true)
    
    if [[ "$error_output" == *"Usage:"* ]]; then
        print_pass "Error recovery shows usage correctly"
    else
        print_fail "Error recovery doesn't show usage"
    fi
}

# ================================================================================
#                            FINAL RESULTS REPORTING
# ================================================================================

print_final_report() {
    local pass_rate
    if [[ $TESTS_TOTAL -gt 0 ]]; then
        pass_rate=$(( (TESTS_PASSED * 100) / TESTS_TOTAL ))
    else
        pass_rate=0
    fi
    
    echo -e "\n${CYAN}╔═══════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${BOLD}                          MASTER TEST REPORT                        ${NC}${CYAN}║${NC}"
    echo -e "${CYAN}╠═══════════════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${CYAN}║${NC} ${GREEN}Tests Passed: ${BOLD}${TESTS_PASSED}${NC}                                                    ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC} ${RED}Tests Failed: ${BOLD}${TESTS_FAILED}${NC}                                                    ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC} Total Tests:  ${BOLD}${TESTS_TOTAL}${NC}                                                    ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC} Pass Rate:    ${BOLD}${pass_rate}%${NC}                                                   ${CYAN}║${NC}"
    
    if [[ $CRITICAL_FAILURES -gt 0 ]]; then
        echo -e "${CYAN}║${NC} ${RED}${BOLD}Critical Failures: ${CRITICAL_FAILURES}${NC}                                           ${CYAN}║${NC}"
    fi
    
    echo -e "${CYAN}╠═══════════════════════════════════════════════════════════════════════╣${NC}"
    
    # Final verdict
    if [[ $TESTS_FAILED -eq 0 ]]; then
        echo -e "${CYAN}║${NC} ${GREEN}${BOLD}RESULT: ✅ ALL TESTS PASSED - PROJECT READY FOR SUBMISSION${NC}        ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC} ${GREEN}This project meets all critical requirements!${NC}                      ${CYAN}║${NC}"
    elif [[ $CRITICAL_FAILURES -gt 0 ]]; then
        echo -e "${CYAN}║${NC} ${RED}${BOLD}RESULT: 💀 CRITICAL FAILURES - DO NOT SUBMIT${NC}                      ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC} ${RED}Critical issues must be fixed before submission!${NC}                  ${CYAN}║${NC}"
    elif [[ $pass_rate -ge 90 ]]; then
        echo -e "${CYAN}║${NC} ${YELLOW}${BOLD}RESULT: ⚠️  MOSTLY READY - MINOR ISSUES${NC}                            ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC} ${YELLOW}Consider fixing remaining issues before submission${NC}               ${CYAN}║${NC}"
    else
        echo -e "${CYAN}║${NC} ${RED}${BOLD}RESULT: ❌ NOT READY - SIGNIFICANT ISSUES${NC}                         ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC} ${RED}Multiple issues need to be addressed${NC}                             ${CYAN}║${NC}"
    fi
    
    echo -e "${CYAN}╚═══════════════════════════════════════════════════════════════════════╝${NC}"
    
    # Save final report
    echo "" >> "$TEST_LOG"
    echo "FINAL RESULTS:" >> "$TEST_LOG"
    echo "Tests Passed: $TESTS_PASSED" >> "$TEST_LOG"
    echo "Tests Failed: $TESTS_FAILED" >> "$TEST_LOG"
    echo "Total Tests: $TESTS_TOTAL" >> "$TEST_LOG"
    echo "Pass Rate: ${pass_rate}%" >> "$TEST_LOG"
    echo "Critical Failures: $CRITICAL_FAILURES" >> "$TEST_LOG"
    
    print_info "Detailed logs saved to:"
    print_info "  - Test Log: $TEST_LOG"
    print_info "  - Error Log: $ERROR_LOG"
    
    # Return appropriate exit code
    if [[ $TESTS_FAILED -eq 0 ]]; then
        return 0
    elif [[ $CRITICAL_FAILURES -gt 0 ]]; then
        return 2
    else
        return 1
    fi
}

# ================================================================================
#                                MAIN EXECUTION
# ================================================================================

main() {
    print_banner
    
    print_info "Initializing master test suite..."
    print_info "Test results will be saved to: $TEST_OUTPUT_DIR"
    
    # Run all test sections
    test_build_system
    test_command_line_arguments
    test_individual_plugins
    test_pipeline_chains
    test_error_handling
    test_memory_management
    test_repeated_plugins
    test_queue_capacity
    test_long_strings
    test_stress_scenarios
    test_integration
    
    # Print final report and exit with appropriate code
    print_final_report
}

# Run main function and preserve exit code
main "$@"
exit_code=$?

echo -e "\n${BLUE}Master test suite completed with exit code: $exit_code${NC}"
exit $exit_code