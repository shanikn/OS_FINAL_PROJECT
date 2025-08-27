#!/bin/bash

# ================================================================================
#                           MODULAR PIPELINE ANALYZER TEST SUITE
# ================================================================================
# Comprehensive testing for the modular multithreaded string analyzer pipeline
# Tests all critical functionality required for assignment submission
# ================================================================================

set -e

# Colors for clear output
readonly GREEN='\033[0;32m'
readonly RED='\033[0;31m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly BOLD='\033[1m'
readonly NC='\033[0m'

# Test configuration
readonly ANALYZER="./output/analyzer"
readonly TIMEOUT=10
declare -i passed=0
declare -i failed=0
declare -i total=0

# ================================================================================
#                                UTILITY FUNCTIONS  
# ================================================================================

print_header() {
    echo -e "\n${BLUE}${BOLD}=== $1 ===${NC}"
}

run_test() {
    local name="$1"
    local command="$2" 
    local expected="$3"
    local should_fail="${4:-false}"
    
    total=$((total + 1))
    echo -e "\n${YELLOW}TEST $total: $name${NC}"
    
    local output exit_code
    if output=$(timeout $TIMEOUT bash -c "$command" 2>&1); then
        exit_code=0
    else
        exit_code=$?
    fi
    
    local test_passed=false
    
    if [[ "$should_fail" == "true" ]]; then
        # Test should fail
        if [[ $exit_code -ne 0 ]]; then
            if [[ -z "$expected" ]] || [[ "$output" =~ $expected ]]; then
                test_passed=true
            fi
        fi
    else
        # Test should succeed
        if [[ $exit_code -eq 0 ]]; then
            if [[ -z "$expected" ]] || [[ "$output" =~ $expected ]]; then
                test_passed=true
            fi
        fi
    fi
    
    if [[ "$test_passed" == "true" ]]; then
        echo -e "${GREEN}✓ PASS${NC}"
        passed=$((passed + 1))
    else
        echo -e "${RED}✗ FAIL${NC}"
        echo -e "   Expected: $expected"
        echo -e "   Got: $output"
        echo -e "   Exit code: $exit_code"
        failed=$((failed + 1))
    fi
}

measure_time() {
    local start=$(date +%s%N)
    "$@"
    local end=$(date +%s%N)
    echo $(( (end - start) / 1000000 )) # milliseconds
}

# ================================================================================
#                                BUILD VERIFICATION
# ================================================================================

test_build_system() {
    print_header "BUILD SYSTEM VERIFICATION"
    
    # Ensure clean build
    run_test "Build system execution" "./build.sh" "All builds completed successfully"
    
    run_test "Analyzer executable exists" "test -x $ANALYZER" ""
    
    run_test "All plugin libraries exist" \
        "test -f output/logger.so && test -f output/uppercaser.so && test -f output/rotator.so && test -f output/flipper.so && test -f output/expander.so && test -f output/typewriter.so" \
        ""
}

# ================================================================================
#                            ARGUMENT VALIDATION TESTING
# ================================================================================

test_argument_validation() {
    print_header "COMMAND LINE ARGUMENT VALIDATION"
    
    run_test "No arguments shows usage" \
        "$ANALYZER" \
        "Usage: ./analyzer" \
        "true"
    
    run_test "Missing plugins argument" \
        "$ANALYZER 10" \
        "At least one plugin must be specified" \
        "true"
        
    run_test "Zero queue size rejected" \
        "echo 'test' | $ANALYZER 0 logger" \
        "Queue size must be positive" \
        "true"
        
    run_test "Negative queue size rejected" \
        "echo 'test' | $ANALYZER -5 logger" \
        "Queue size must be positive" \
        "true"
        
    run_test "Non-numeric queue size rejected" \
        "echo 'test' | $ANALYZER abc logger" \
        "Queue size must be positive" \
        "true"
        
    run_test "Non-existent plugin rejected" \
        "echo -e 'test\n<END>' | $ANALYZER 10 fakeplugin" \
        "Failed to load plugin" \
        "true"
}

# ================================================================================
#                            INDIVIDUAL PLUGIN TESTING
# ================================================================================

test_individual_plugins() {
    print_header "INDIVIDUAL PLUGIN FUNCTIONALITY"
    
    run_test "Logger plugin basic functionality" \
        "echo -e 'hello\n<END>' | $ANALYZER 10 logger" \
        "\\[logger\\] hello"
        
    run_test "Uppercaser converts to uppercase" \
        "echo -e 'Hello World123\n<END>' | $ANALYZER 10 uppercaser logger" \
        "\\[logger\\] HELLO WORLD123"
        
    run_test "Rotator shifts characters right" \
        "echo -e 'abcdef\n<END>' | $ANALYZER 10 rotator logger" \
        "\\[logger\\] fabcde"
        
    run_test "Flipper reverses string" \
        "echo -e 'hello\n<END>' | $ANALYZER 10 flipper logger" \
        "\\[logger\\] olleh"
        
    run_test "Expander adds spaces between chars" \
        "echo -e 'abc\n<END>' | $ANALYZER 10 expander logger" \
        "\\[logger\\] a b c"
        
    # Test typewriter with timing
    echo -e "\n${YELLOW}TEST $((total + 1)): Typewriter timing behavior${NC}"
    total=$((total + 1))
    local duration
    duration=$(measure_time bash -c "echo -e 'hi\n<END>' | $ANALYZER 10 typewriter >/dev/null")
    if [[ $duration -gt 150 ]]; then  # Should take at least 200ms for 2 chars
        echo -e "${GREEN}✓ PASS${NC} (${duration}ms)"
        passed=$((passed + 1))
    else
        echo -e "${RED}✗ FAIL${NC} - Too fast: ${duration}ms"
        failed=$((failed + 1))
    fi
}

# ================================================================================
#                            PIPELINE CHAIN TESTING
# ================================================================================

test_pipeline_chains() {
    print_header "PIPELINE CHAIN FUNCTIONALITY"
    
    run_test "Two-plugin chain" \
        "echo -e 'hello\n<END>' | $ANALYZER 15 uppercaser logger" \
        "\\[logger\\] HELLO"
        
    run_test "Three-plugin chain transformation" \
        "echo -e 'abcd\n<END>' | $ANALYZER 15 uppercaser rotator logger" \
        "\\[logger\\] DABC"
        
    run_test "Complex four-plugin chain" \
        "echo -e 'test\n<END>' | $ANALYZER 20 uppercaser flipper rotator logger" \
        "\\[logger\\] TSET"
        
    run_test "All plugins in sequence" \
        "echo -e 'hi\n<END>' | $ANALYZER 25 uppercaser rotator flipper expander logger" \
        "\\[logger\\]"
        
    run_test "Multiple input lines through pipeline" \
        "echo -e 'line1\nline2\n<END>' | $ANALYZER 10 uppercaser logger" \
        "\\[logger\\] LINE1.*\\[logger\\] LINE2"
}

# ================================================================================
#                            ERROR HANDLING & EDGE CASES
# ================================================================================

test_error_handling() {
    print_header "ERROR HANDLING & EDGE CASES"
    
    run_test "Empty string processing" \
        "echo -e '\n<END>' | $ANALYZER 10 logger" \
        "\\[logger\\]"
        
    run_test "Single character processing" \
        "echo -e 'a\n<END>' | $ANALYZER 10 uppercaser logger" \
        "\\[logger\\] A"
        
    run_test "Special characters preserved" \
        "echo -e '!@#\$%^&*()\n<END>' | $ANALYZER 10 logger" \
        "\\[logger\\] !@#\\\$%\\^&\\*\\(\\)"
        
    run_test "Numbers preserved in uppercaser" \
        "echo -e 'abc123def\n<END>' | $ANALYZER 10 uppercaser logger" \
        "\\[logger\\] ABC123DEF"
        
    run_test "Immediate shutdown signal" \
        "echo '<END>' | $ANALYZER 10 logger" \
        "Pipeline shutdown complete"
        
    run_test "Pipeline shutdown message present" \
        "echo -e 'test\n<END>' | $ANALYZER 10 logger" \
        "Pipeline shutdown complete"
}

# ================================================================================
#                            QUEUE & CAPACITY TESTING  
# ================================================================================

test_queue_capacity() {
    print_header "QUEUE CAPACITY & THREADING"
    
    run_test "Small queue size (1) handles processing" \
        "echo -e 'test\n<END>' | $ANALYZER 1 uppercaser logger" \
        "\\[logger\\] TEST"
        
    run_test "Large queue size (1000) works" \
        "echo -e 'large\n<END>' | $ANALYZER 1000 uppercaser logger" \
        "\\[logger\\] LARGE"
        
    run_test "Multiple items with small queue" \
        "echo -e 'item1\nitem2\nitem3\n<END>' | $ANALYZER 2 logger" \
        "\\[logger\\] item1.*\\[logger\\] item2.*\\[logger\\] item3"
        
    run_test "Repeated plugin usage" \
        "echo -e 'test\n<END>' | $ANALYZER 10 logger logger" \
        "\\[logger\\] test.*\\[logger\\] test"
}

# ================================================================================
#                            STRING LENGTH TESTING
# ================================================================================

test_string_lengths() {
    print_header "STRING LENGTH HANDLING"
    
    run_test "Medium length string" \
        "echo -e 'The quick brown fox jumps over the lazy dog multiple times\n<END>' | $ANALYZER 15 uppercaser logger" \
        "\\[logger\\] THE QUICK BROWN FOX"
        
    # Test long string (close to 1024 limit)
    local long_string
    long_string=$(printf 'A%.0s' {1..500})  # 500 A's
    run_test "Long string processing (500 chars)" \
        "echo -e '${long_string}\n<END>' | $ANALYZER 20 logger" \
        "\\[logger\\] ${long_string}"
}

# ================================================================================
#                            STRESS & INTEGRATION TESTING
# ================================================================================

test_stress_scenarios() {
    print_header "STRESS & INTEGRATION TESTING"
    
    run_test "Rapid multiple inputs" \
        "{ for i in {1..5}; do echo \"rapid\$i\"; done; echo '<END>'; } | $ANALYZER 3 logger" \
        "\\[logger\\] rapid1.*\\[logger\\] rapid5"
        
    run_test "Double transformation correctness (flipper twice)" \
        "echo -e 'hello\n<END>' | $ANALYZER 10 flipper flipper logger" \
        "\\[logger\\] hello"
        
    run_test "Rotator correctness (4 rotations = original)" \
        "echo -e 'abcd\n<END>' | $ANALYZER 15 rotator rotator rotator rotator logger" \
        "\\[logger\\] abcd"
        
    run_test "Mixed plugin types in long chain" \
        "echo -e 'mix\n<END>' | $ANALYZER 30 uppercaser rotator uppercaser flipper logger" \
        "\\[logger\\]"
}

# ================================================================================
#                            FINAL RESULTS & SUMMARY
# ================================================================================

print_final_results() {
    local pass_rate=0
    if [[ $total -gt 0 ]]; then
        pass_rate=$(( (passed * 100) / total ))
    fi
    
    echo -e "\n${BLUE}${BOLD}╔══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}${BOLD}║                    TEST RESULTS SUMMARY                 ║${NC}"
    echo -e "${BLUE}${BOLD}╠══════════════════════════════════════════════════════════╣${NC}"
    echo -e "${BLUE}${BOLD}║${NC} ${GREEN}Tests Passed: ${BOLD}$passed${NC}                                      ${BLUE}${BOLD}║${NC}"
    echo -e "${BLUE}${BOLD}║${NC} ${RED}Tests Failed: ${BOLD}$failed${NC}                                      ${BLUE}${BOLD}║${NC}"  
    echo -e "${BLUE}${BOLD}║${NC} Total Tests:  ${BOLD}$total${NC}                                      ${BLUE}${BOLD}║${NC}"
    echo -e "${BLUE}${BOLD}║${NC} Pass Rate:    ${BOLD}$pass_rate%${NC}                                     ${BLUE}${BOLD}║${NC}"
    echo -e "${BLUE}${BOLD}╠══════════════════════════════════════════════════════════╣${NC}"
    
    if [[ $failed -eq 0 ]]; then
        echo -e "${BLUE}${BOLD}║${NC} ${GREEN}${BOLD}RESULT: ✅ ALL TESTS PASSED - READY FOR SUBMISSION${NC}    ${BLUE}${BOLD}║${NC}"
        echo -e "${BLUE}${BOLD}║${NC} ${GREEN}Project meets all requirements!${NC}                        ${BLUE}${BOLD}║${NC}"
    elif [[ $pass_rate -ge 90 ]]; then
        echo -e "${BLUE}${BOLD}║${NC} ${YELLOW}${BOLD}RESULT: ⚠️  MOSTLY READY - MINOR ISSUES${NC}               ${BLUE}${BOLD}║${NC}"
        echo -e "${BLUE}${BOLD}║${NC} ${YELLOW}Consider addressing remaining failures${NC}                ${BLUE}${BOLD}║${NC}"
    else
        echo -e "${BLUE}${BOLD}║${NC} ${RED}${BOLD}RESULT: ❌ NEEDS WORK - MULTIPLE FAILURES${NC}            ${BLUE}${BOLD}║${NC}"
        echo -e "${BLUE}${BOLD}║${NC} ${RED}Significant issues need to be resolved${NC}               ${BLUE}${BOLD}║${NC}"
    fi
    
    echo -e "${BLUE}${BOLD}╚══════════════════════════════════════════════════════════╝${NC}"
    
    return $failed
}

# ================================================================================
#                                MAIN EXECUTION
# ================================================================================

main() {
    echo -e "${BLUE}${BOLD}"
    echo "╔══════════════════════════════════════════════════════════════════╗"
    echo "║              MODULAR PIPELINE ANALYZER TEST SUITE               ║" 
    echo "║                  Comprehensive Integration Testing               ║"
    echo "╚══════════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    echo -e "${YELLOW}Initializing test suite...${NC}"
    
    # Run all test categories
    test_build_system
    test_argument_validation  
    test_individual_plugins
    test_pipeline_chains
    test_error_handling
    test_queue_capacity
    test_string_lengths
    test_stress_scenarios
    
    # Print final results and exit with appropriate code
    print_final_results
}

# Execute main function with all arguments
main "$@"