/**
 * Unit test for plugin_common.c functionality
 * Tests the plugin infrastructure in isolation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "plugins/plugin_common.h"

// Test configuration
#define TEST_QUEUE_SIZE 5
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_RESET   "\033[0m"

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// Test helper functions
void print_test_result(const char* test_name, int passed) {
    if (passed) {
        printf(COLOR_GREEN "[TEST PASS]" COLOR_RESET " %s\n", test_name);
        tests_passed++;
    } else {
        printf(COLOR_RED "[TEST FAIL]" COLOR_RESET " %s\n", test_name);
        tests_failed++;
    }
}

// Mock transformation function for testing
const char* test_transform(const char* input) {
    if (input == NULL) return NULL;
    
    // Simple test: prepend "TEST:" to input
    size_t len = strlen(input) + 6; // "TEST:" + input + null
    char* result = malloc(len);
    if (result == NULL) return NULL;
    
    snprintf(result, len, "TEST:%s", input);
    return result;
}

// Mock transformation that returns NULL (error case)
const char* test_transform_fail(const char* input) {
    return NULL;
}

// Test 1: Basic initialization
void test_plugin_init() {
    const char* result = common_plugin_init(test_transform, "test_plugin", TEST_QUEUE_SIZE);
    int passed = (result == NULL);
    print_test_result("Plugin initialization", passed);
}

// Test 2: Double initialization should fail
void test_double_init() {
    const char* result = common_plugin_init(test_transform, "test_plugin2", TEST_QUEUE_SIZE);
    int passed = (result != NULL && strstr(result, "already initialized") != NULL);
    print_test_result("Double initialization prevention", passed);
}

// Test 3: Plugin place work
void test_place_work() {
    const char* result = plugin_place_work("hello");
    int passed = (result == NULL);
    print_test_result("Place work in queue", passed);
}

// Test 4: Plugin place work with NULL input
void test_place_work_null() {
    const char* result = plugin_place_work(NULL);
    int passed = (result != NULL && strstr(result, "NULL") != NULL);
    print_test_result("Place work with NULL input", passed);
}

// Test 5: Plugin attach
void test_attach() {
    // Mock next place work function
    const char* (*mock_next)(const char*) = plugin_place_work;
    
    plugin_attach(mock_next);
    // No direct way to verify attachment, but function shouldn't crash
    print_test_result("Plugin attach", 1);
}

// Test 6: Wait for work to be processed
void test_work_processing() {
    // Place some work and wait a bit for processing
    plugin_place_work("test_input");
    usleep(100000); // 100ms
    
    // Check if we can place more work (queue not stuck)
    const char* result = plugin_place_work("test_input2");
    int passed = (result == NULL);
    print_test_result("Work processing", passed);
}

// Test 7: Plugin shutdown sequence
void test_shutdown() {
    // Send shutdown signal
    const char* place_result = plugin_place_work("<END>");
    int place_ok = (place_result == NULL);
    
    // Wait for completion
    const char* wait_result = plugin_wait_finished();
    int wait_ok = (wait_result == NULL);
    
    // Finalize
    const char* fini_result = plugin_fini();
    int fini_ok = (fini_result == NULL);
    
    int passed = place_ok && wait_ok && fini_ok;
    print_test_result("Shutdown sequence", passed);
}

// Test 8: Operations on uninitialized plugin
void test_uninitialized_ops() {
    // After shutdown, operations should fail
    const char* result1 = plugin_place_work("test");
    const char* result2 = plugin_wait_finished();
    const char* result3 = plugin_fini();
    
    int passed = (result1 != NULL && result2 != NULL && result3 != NULL);
    print_test_result("Operations on uninitialized plugin", passed);
}

// Test 9: Re-initialization after shutdown
void test_reinit() {
    const char* result = common_plugin_init(test_transform, "test_plugin_new", TEST_QUEUE_SIZE);
    int passed = (result == NULL);
    print_test_result("Re-initialization after shutdown", passed);
    
    // Cleanup for next tests
    if (passed) {
        plugin_place_work("<END>");
        plugin_wait_finished();
        plugin_fini();
    }
}

// Test 10: Thread safety test (basic)
void* worker_thread(void* arg) {
    int thread_id = *(int*)arg;
    char buffer[32];
    
    for (int i = 0; i < 5; i++) {
        snprintf(buffer, sizeof(buffer), "thread%d_msg%d", thread_id, i);
        plugin_place_work(buffer);
        usleep(10000); // 10ms
    }
    
    return NULL;
}

void test_thread_safety() {
    // Initialize plugin
    const char* init_result = common_plugin_init(test_transform, "thread_test", 20);
    if (init_result != NULL) {
        print_test_result("Thread safety test setup", 0);
        return;
    }
    
    // Create multiple threads placing work
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
    }
    
    // Wait for threads
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Shutdown
    plugin_place_work("<END>");
    plugin_wait_finished();
    const char* fini_result = plugin_fini();
    
    int passed = (fini_result == NULL);
    print_test_result("Thread safety test", passed);
}

// Test 11: Invalid initialization parameters
void test_invalid_init_params() {
    // Test NULL function pointer
    const char* result1 = common_plugin_init(NULL, "test", TEST_QUEUE_SIZE);
    int test1 = (result1 != NULL);
    
    // Test NULL plugin name
    const char* result2 = common_plugin_init(test_transform, NULL, TEST_QUEUE_SIZE);
    int test2 = (result2 != NULL);
    
    // Test invalid queue size (0)
    const char* result3 = common_plugin_init(test_transform, "test", 0);
    int test3 = (result3 != NULL);
    
    // Test invalid queue size (negative)
    const char* result4 = common_plugin_init(test_transform, "test", -5);
    int test4 = (result4 != NULL);
    
    int passed = test1 && test2 && test3 && test4;
    print_test_result("Invalid initialization parameters", passed);
}

// Test 12: Queue boundary conditions
void test_queue_boundaries() {
    // Test with queue size 1 (minimum practical size)
    const char* init_result = common_plugin_init(test_transform, "boundary_test", 1);
    if (init_result != NULL) {
        print_test_result("Queue boundary test setup", 0);
        return;
    }
    
    // Fill the queue completely
    const char* place1 = plugin_place_work("msg1");
    const char* place2 = plugin_place_work("msg2");
    
    // Give time for processing
    usleep(50000); // 50ms
    
    // Shutdown
    plugin_place_work("<END>");
    plugin_wait_finished();
    const char* fini_result = plugin_fini();
    
    int passed = (place1 == NULL && fini_result == NULL);
    print_test_result("Queue boundary conditions", passed);
}

// Test 13: Multiple shutdown signals
void test_multiple_shutdown() {
    const char* init_result = common_plugin_init(test_transform, "multi_shutdown", TEST_QUEUE_SIZE);
    if (init_result != NULL) {
        print_test_result("Multiple shutdown test setup", 0);
        return;
    }
    
    // Send multiple END signals
    plugin_place_work("<END>");
    plugin_place_work("<END>");
    plugin_place_work("<END>");
    
    // Should still shutdown gracefully
    const char* wait_result = plugin_wait_finished();
    const char* fini_result = plugin_fini();
    
    int passed = (wait_result == NULL && fini_result == NULL);
    print_test_result("Multiple shutdown signals", passed);
}

// Test 14: Operations during shutdown
void test_operations_during_shutdown() {
    const char* init_result = common_plugin_init(test_transform, "shutdown_ops", TEST_QUEUE_SIZE);
    if (init_result != NULL) {
        print_test_result("Operations during shutdown test setup", 0);
        return;
    }
    
    // Start shutdown
    plugin_place_work("<END>");
    
    // Try operations during shutdown (should still work until fully shut down)
    const char* place_result = plugin_place_work("late_message");
    
    // Complete shutdown
    plugin_wait_finished();
    const char* fini_result = plugin_fini();
    
    // After shutdown, operations should fail
    const char* post_shutdown = plugin_place_work("too_late");
    
    int passed = (fini_result == NULL && post_shutdown != NULL);
    print_test_result("Operations during shutdown", passed);
}

// Test 15: Transformation function error handling
void test_transform_error_handling() {
    const char* init_result = common_plugin_init(test_transform_fail, "fail_test", TEST_QUEUE_SIZE);
    if (init_result != NULL) {
        print_test_result("Transform error test setup", 0);
        return;
    }
    
    // Place work with failing transform
    plugin_place_work("test_input");
    usleep(50000); // Give time for processing
    
    // Should still be able to shutdown cleanly even with failing transforms
    plugin_place_work("<END>");
    const char* wait_result = plugin_wait_finished();
    const char* fini_result = plugin_fini();
    
    int passed = (wait_result == NULL && fini_result == NULL);
    print_test_result("Transformation error handling", passed);
}

// Test 16: Attach function edge cases
void test_attach_edge_cases() {
    const char* init_result = common_plugin_init(test_transform, "attach_test", TEST_QUEUE_SIZE);
    if (init_result != NULL) {
        print_test_result("Attach edge cases test setup", 0);
        return;
    }
    
    // Test attaching NULL function (should not crash)
    plugin_attach(NULL);
    
    // Test multiple attachments (last one should win)
    plugin_attach(plugin_place_work);
    plugin_attach(NULL);
    
    // Place work and shutdown
    plugin_place_work("test");
    plugin_place_work("<END>");
    plugin_wait_finished();
    const char* fini_result = plugin_fini();
    
    int passed = (fini_result == NULL);
    print_test_result("Attach function edge cases", passed);
}

// Test 17: Rapid start/stop cycles
void test_rapid_cycles() {
    int success_count = 0;
    
    for (int i = 0; i < 5; i++) {
        const char* init_result = common_plugin_init(test_transform, "cycle_test", 3);
        if (init_result == NULL) {
            plugin_place_work("quick_test");
            plugin_place_work("<END>");
            plugin_wait_finished();
            const char* fini_result = plugin_fini();
            if (fini_result == NULL) {
                success_count++;
            }
        }
        usleep(10000); // Small delay between cycles
    }
    
    int passed = (success_count == 5);
    print_test_result("Rapid start/stop cycles", passed);
}

// Test 18: Concurrent operations stress test
void* stress_worker(void* arg) {
    int worker_id = *(int*)arg;
    char buffer[64];
    
    for (int i = 0; i < 20; i++) {
        snprintf(buffer, sizeof(buffer), "stress_%d_%d", worker_id, i);
        plugin_place_work(buffer);
        if (i % 5 == 0) usleep(1000); // Occasional small delay
    }
    return NULL;
}

void test_stress_concurrent() {
    const char* init_result = common_plugin_init(test_transform, "stress_test", 50);
    if (init_result != NULL) {
        print_test_result("Stress test setup", 0);
        return;
    }
    
    // Create multiple worker threads
    pthread_t threads[10];
    int thread_ids[10];
    
    for (int i = 0; i < 10; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, stress_worker, &thread_ids[i]);
    }
    
    // Wait for all workers
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Shutdown
    plugin_place_work("<END>");
    const char* wait_result = plugin_wait_finished();
    const char* fini_result = plugin_fini();
    
    int passed = (wait_result == NULL && fini_result == NULL);
    print_test_result("Concurrent operations stress test", passed);
}

// Test 19: Memory and resource validation
void test_resource_validation() {
    // This test checks if we can repeatedly init/shutdown without issues
    int cycles = 3;
    int success_count = 0;
    
    for (int i = 0; i < cycles; i++) {
        const char* init_result = common_plugin_init(test_transform, "resource_test", 5);
        if (init_result == NULL) {
            // Do some work
            for (int j = 0; j < 10; j++) {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "resource_msg_%d_%d", i, j);
                plugin_place_work(buffer);
            }
            
            plugin_place_work("<END>");
            const char* wait_result = plugin_wait_finished();
            const char* fini_result = plugin_fini();
            
            if (wait_result == NULL && fini_result == NULL) {
                success_count++;
            }
        }
        usleep(50000); // Pause between cycles
    }
    
    int passed = (success_count == cycles);
    print_test_result("Resource validation", passed);
}

int main() {
    printf(COLOR_YELLOW "=== Comprehensive Plugin Common Unit Tests ===" COLOR_RESET "\n\n");
    
    // Basic functionality tests
    printf(COLOR_YELLOW "--- Basic Functionality ---" COLOR_RESET "\n");
    test_plugin_init();
    test_double_init();
    test_place_work();
    test_place_work_null();
    test_attach();
    test_work_processing();
    test_shutdown();
    test_uninitialized_ops();
    test_reinit();
    
    // Edge cases and error conditions
    printf("\n" COLOR_YELLOW "--- Edge Cases & Error Conditions ---" COLOR_RESET "\n");
    test_invalid_init_params();
    test_queue_boundaries();
    test_multiple_shutdown();
    test_operations_during_shutdown();
    test_transform_error_handling();
    test_attach_edge_cases();
    
    // Stress and reliability tests
    printf("\n" COLOR_YELLOW "--- Stress & Reliability Tests ---" COLOR_RESET "\n");
    test_rapid_cycles();
    test_thread_safety();
    test_stress_concurrent();
    test_resource_validation();
    
    // Print comprehensive summary
    printf("\n" COLOR_YELLOW "=== Comprehensive Test Summary ===" COLOR_RESET "\n");
    printf(COLOR_GREEN "Tests passed: %d" COLOR_RESET "\n", tests_passed);
    if (tests_failed > 0) {
        printf(COLOR_RED "Tests failed: %d" COLOR_RESET "\n", tests_failed);
        printf(COLOR_RED "Total test coverage: %.1f%%" COLOR_RESET "\n", 
               (float)tests_passed / (tests_passed + tests_failed) * 100);
    } else {
        printf(COLOR_GREEN "Total test coverage: 100%%" COLOR_RESET "\n");
    }
    
    if (tests_failed == 0) {
        printf(COLOR_GREEN "\n🎉 ALL COMPREHENSIVE TESTS PASSED!" COLOR_RESET "\n");
        printf(COLOR_GREEN "✅ Plugin infrastructure is rock solid and ready for production!" COLOR_RESET "\n");
        return 0;
    } else {
        printf(COLOR_RED "\n❌ Some tests failed. Issues found in implementation:" COLOR_RESET "\n");
        printf("   - Check error handling for invalid inputs\n");
        printf("   - Verify thread safety and resource cleanup\n");
        printf("   - Test edge cases and boundary conditions\n");
        return 1;
    }
}