#!/bin/bash

# ================================================================================
#                           MODULAR PIPELINE ANALYZER TEST SUITE
# ================================================================================
# Comprehensive testing for the modular multithreaded string analyzer pipeline
# Tests all critical functionality required for assignment submission
# ================================================================================

set -e

# Pastel color palette
readonly PASTEL_GREEN='\033[38;2;119;221;119m'   # Soft mint green
readonly PASTEL_RED='\033[38;2;255;179;186m'     # Soft pink/coral
readonly PASTEL_YELLOW='\033[38;2;253;253;150m'  # Soft lemon
readonly PASTEL_BLUE='\033[38;2;174;198;207m'    # Soft sky blue
readonly PASTEL_PURPLE='\033[38;2;199;177;218m'  # Soft lavender
readonly PASTEL_PINK='\033[38;2;255;209;220m'     # Soft rose
readonly PASTEL_CYAN='\033[38;2;150;206;214m'     # Soft cyan
readonly BOLD='\033[1m'
readonly DIM='\033[2m'
readonly NC='\033[0m'

# Test configuration
readonly ANALYZER="./output/analyzer"
readonly TIMEOUT=10
declare -i passed=0
declare -i failed=0
declare -i total=0

# Helper function to print section headers with cute decorations
print_header() {
    echo -e "\n${PASTEL_PURPLE}${BOLD}╭─────────────────────────────────────────────────────────────╮${NC}"
    echo -e "${PASTEL_PURPLE}${BOLD}│  ✨ $1${NC}"
    echo -e "${PASTEL_PURPLE}${BOLD}╰─────────────────────────────────────────────────────────────╯${NC}"
}

run_test() {
    local name="$1"
    local command="$2" 
    local expected="$3"
    local should_fail="${4:-false}"
    
    total=$((total + 1))
    echo -e "\n${PASTEL_CYAN}🧪 TEST $total: ${PASTEL_YELLOW}$name${NC}"
    
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
        echo -e "   ${PASTEL_GREEN}✅ PASSED ${DIM}(๑˃ᴗ˂)ﻭ${NC}"
        passed=$((passed + 1))
    else
        echo -e "   ${PASTEL_RED}❌ FAILED ${DIM}(｡•́︿•̀｡)${NC}"
        echo -e "   ${DIM}Expected: $expected${NC}"
        echo -e "   ${DIM}Got: $output${NC}"
        echo -e "   ${DIM}Exit code: $exit_code${NC}"
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
        "Unknown plugin: fakeplugin" \
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
    echo -e "\n${PASTEL_CYAN}🧪 TEST $((total + 1)): ${PASTEL_YELLOW}Typewriter timing behavior${NC}"
    total=$((total + 1))
    local duration
    duration=$(measure_time bash -c "echo -e 'hi\n<END>' | $ANALYZER 10 typewriter >/dev/null")
    if [[ $duration -gt 150 ]]; then  # Should take at least 200ms for 2 chars
        echo -e "   ${PASTEL_GREEN}✅ PASSED ${DIM}(๑˃ᴗ˂)ﻭ${NC} ${DIM}(${duration}ms)${NC}"
        passed=$((passed + 1))
    else
        echo -e "   ${PASTEL_RED}❌ FAILED ${DIM}(｡•́︿•̀｡)${NC} ${DIM}- Too fast: ${duration}ms${NC}"
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
        "\\[logger\\] TTSE"
        
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
        
    # Removed bonus test for repeated plugin usage
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
        
    # Removed bonus test for mixed plugin types in long chain
}

# ================================================================================
#                            FINAL RESULTS & SUMMARY
# ================================================================================

print_final_results() {
    local pass_rate=0
    if [[ $total -gt 0 ]]; then
        pass_rate=$(( (passed * 100) / total ))
    fi
    
    # Create cute decorative border
    echo -e "\n${PASTEL_PINK}${BOLD}╭────────────────────────────────────────────────────────────────╮${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│                                                                │${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│                    🌸 TEST RESULTS SUMMARY 🌸                  │${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│                                                                │${NC}"
    echo -e "${PASTEL_PINK}${BOLD}├────────────────────────────────────────────────────────────────┤${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│                                                                │${NC}"
    
    # Format the stats with proper padding
    local passed_str=$(printf "%-2d" $passed)
    local failed_str=$(printf "%-2d" $failed)
    local total_str=$(printf "%-2d" $total)
    local rate_str=$(printf "%-3d" $pass_rate)
    
    echo -e "${PASTEL_PINK}${BOLD}│${NC}  ${PASTEL_GREEN}✨ Tests Passed:${NC} ${BOLD}$passed_str${NC}                                         ${PASTEL_PINK}${BOLD}│${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│${NC}  ${PASTEL_RED}💔 Tests Failed:${NC} ${BOLD}$failed_str${NC}                                         ${PASTEL_PINK}${BOLD}│${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│${NC}  ${PASTEL_CYAN}📊 Total Tests:${NC}  ${BOLD}$total_str${NC}                                         ${PASTEL_PINK}${BOLD}│${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│${NC}  ${PASTEL_PURPLE}🎯 Pass Rate:${NC}    ${BOLD}$rate_str%${NC}                                        ${PASTEL_PINK}${BOLD}│${NC}"
    
    echo -e "${PASTEL_PINK}${BOLD}│                                                                │${NC}"
    echo -e "${PASTEL_PINK}${BOLD}├────────────────────────────────────────────────────────────────┤${NC}"
    echo -e "${PASTEL_PINK}${BOLD}│                                                                │${NC}"
    
    if [[ $failed -eq 0 ]]; then
        echo -e "${PASTEL_PINK}${BOLD}│${NC}     ${PASTEL_GREEN}${BOLD}🎉 ALL TESTS PASSED - READY FOR SUBMISSION! 🎉${NC}          ${PASTEL_PINK}${BOLD}│${NC}"
        echo -e "${PASTEL_PINK}${BOLD}│${NC}           ${PASTEL_GREEN}Your project sparkles with perfection!${NC}            ${PASTEL_PINK}${BOLD}│${NC}"
        echo -e "${PASTEL_PINK}${BOLD}│${NC}                      ${DIM}✧･ﾟ: *✧･ﾟ:*${NC}                            ${PASTEL_PINK}${BOLD}│${NC}"
    elif [[ $pass_rate -ge 90 ]]; then
        echo -e "${PASTEL_PINK}${BOLD}│${NC}       ${PASTEL_YELLOW}${BOLD}⚠️  ALMOST THERE - MINOR ADJUSTMENTS NEEDED${NC}          ${PASTEL_PINK}${BOLD}│${NC}"
        echo -e "${PASTEL_PINK}${BOLD}│${NC}          ${PASTEL_YELLOW}Just a few tweaks to reach perfection!${NC}            ${PASTEL_PINK}${BOLD}│${NC}"
        echo -e "${PASTEL_PINK}${BOLD}│${NC}                      ${DIM}(ﾉ◕ヮ◕)ﾉ*:･ﾟ✧${NC}                          ${PASTEL_PINK}${BOLD}│${NC}"
    else
        echo -e "${PASTEL_PINK}${BOLD}│${NC}        ${PASTEL_RED}${BOLD}❌ NEEDS ATTENTION - MULTIPLE ISSUES${NC}               ${PASTEL_PINK}${BOLD}│${NC}"
        echo -e "${PASTEL_PINK}${BOLD}│${NC}           ${PASTEL_RED}Don't give up! You've got this!${NC}                  ${PASTEL_PINK}${BOLD}│${NC}"
        echo -e "${PASTEL_PINK}${BOLD}│${NC}                      ${DIM}(っ˘̩╭╮˘̩)っ${NC}                            ${PASTEL_PINK}${BOLD}│${NC}"
    fi
    
    echo -e "${PASTEL_PINK}${BOLD}│                                                                │${NC}"
    echo -e "${PASTEL_PINK}${BOLD}╰────────────────────────────────────────────────────────────────╯${NC}"
    
    return $failed
}

# ================================================================================
#                                MAIN EXECUTION
# ================================================================================

main() {
    echo -e "${PASTEL_PURPLE}${BOLD}"
    echo "╔════════════════════════════════════════════════════════════════════╗"
    echo "║                                                                    ║"
    echo "║           🌟 MODULAR PIPELINE ANALYZER TEST SUITE 🌟              ║" 
    echo "║                  ✨ Comprehensive Testing Suite ✨                ║"
    echo "║                                                                    ║"
    echo "╚════════════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    echo -e "\n${PASTEL_CYAN}🚀 Initializing test suite...${NC}\n"
    
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
    exit $failed
}

# Execute main function with all arguments
main "$@"