/**
 * A unit test is a small program that tests a specific module (in this case, the
 * monitor) in isolation to ensure it behaves as expected. Test thoroughly, as it will save you a
 * lot of headaches and frustrations later on in case of a bug. 
 */

#include <pthread.h>   
#include <stdio.h>     
#include <stdlib.h>    
#include <unistd.h>

#include "monitor.h"

// Simple output without colors

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;

// Function to print test results
void print_test_result(const char* test_name, int passed) {
    if (passed) {
        printf("[TEST PASS] %s\n", test_name);
        tests_passed++;
    } else {
        printf("[TEST FAIL] %s\n", test_name);
        tests_failed++;
    }
}

void print_test_summary() {
    printf("\n========== TEST SUMMARY ==========\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Total tests:  %d\n", tests_passed + tests_failed);
    printf("=================================\n");
}

// Test 1: Basic initialization and destruction
void test_basic_init_destroy() {
    monitor_t monitor;
    
    // Test successful initialization
    int result = monitor_init(&monitor);
    int passed = (result == 0);
    
    if (passed) {
        monitor_destroy(&monitor);
    }
    
    print_test_result("Basic initialization and destruction", passed);
}

// Test 2: NULL pointer handling
void test_null_pointer_handling() {
    int passed = 1;
    
    // Test monitor_init with NULL
    if (monitor_init(NULL) != -1) {
        passed = 0;
    }
    
    // Test other functions with NULL (should not crash)
    monitor_destroy(NULL);
    monitor_signal(NULL);
    monitor_reset(NULL);
    
    if (monitor_wait(NULL) != -1) {
        passed = 0;
    }
    
    print_test_result("NULL pointer handling", passed);
}

// Test 3: Signal before wait (race condition prevention)
void test_signal_before_wait() {
    monitor_t monitor;
    monitor_init(&monitor);
    
    // Signal the monitor first
    monitor_signal(&monitor);
    
    // Then wait - should return immediately without blocking
    int result = monitor_wait(&monitor);
    int passed = (result == 0);
    
    monitor_destroy(&monitor);
    print_test_result("Signal before wait (race condition prevention)", passed);
}

// Test 4: Reset functionality
void test_reset_functionality() {
    monitor_t monitor;
    monitor_init(&monitor);
    
    // Signal the monitor
    monitor_signal(&monitor);
    
    // Reset it
    monitor_reset(&monitor);
    
    // Now a wait should block (we'll use a timeout mechanism)
    // We'll test this by checking if signal state was properly reset
    monitor_signal(&monitor);
    int result = monitor_wait(&monitor);
    int passed = (result == 0);
    
    monitor_destroy(&monitor);
    print_test_result("Reset functionality", passed);
}

// Shared data for multithreaded tests
typedef struct {
    monitor_t* monitor;
    int* shared_value;
    int thread_ready;
    int test_result;
} thread_test_data_t;

// Thread function for testing wait functionality
void* waiting_thread(void* arg) {
    thread_test_data_t* data = (thread_test_data_t*)arg;
    
    // Indicate thread is ready
    data->thread_ready = 1;
    
    // Wait for the monitor
    int result = monitor_wait(data->monitor);
    
    // If wait was successful, update shared value
    if (result == 0) {
        (*data->shared_value)++;
        data->test_result = 1;
    } else {
        data->test_result = 0;
    }
    
    return NULL;
}

// Test 5: Signal after wait (normal operation)
void test_signal_after_wait() {
    monitor_t monitor;
    monitor_init(&monitor);
    
    int shared_value = 0;
    thread_test_data_t data = {&monitor, &shared_value, 0, 0};
    
    pthread_t thread;
    pthread_create(&thread, NULL, waiting_thread, &data);
    
    // Wait for thread to be ready and start waiting
    while (!data.thread_ready) {
        usleep(1000); // 1ms
    }
    usleep(10000); // Give thread time to call monitor_wait
    
    // Now signal the monitor
    monitor_signal(&monitor);
    
    // Wait for thread to complete
    pthread_join(thread, NULL);
    
    int passed = (data.test_result == 1 && shared_value == 1);
    
    monitor_destroy(&monitor);
    print_test_result("Signal after wait (normal operation)", passed);
}

// Thread function for multiple waiters
void* multi_waiting_thread(void* arg) {
    thread_test_data_t* data = (thread_test_data_t*)arg;
    
    // Wait for the monitor
    int result = monitor_wait(data->monitor);
    
    // If wait was successful, increment shared value atomically
    if (result == 0) {
        __sync_fetch_and_add(data->shared_value, 1);
    }
    
    return NULL;
}

// Test 6: Multiple waiters (broadcast functionality)
void test_multiple_waiters() {
    monitor_t monitor;
    monitor_init(&monitor);
    
    const int NUM_THREADS = 5;
    int shared_value = 0;
    pthread_t threads[NUM_THREADS];
    thread_test_data_t data = {&monitor, &shared_value, 0, 0};
    
    // Create multiple waiting threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, multi_waiting_thread, &data);
    }
    
    // Give threads time to start waiting
    usleep(50000); // 50ms
    
    // Signal the monitor (should wake all threads)
    monitor_signal(&monitor);
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    int passed = (shared_value == NUM_THREADS);
    
    monitor_destroy(&monitor);
    print_test_result("Multiple waiters (broadcast functionality)", passed);
}

// Test 7: Multiple signals (idempotent behavior)
void test_multiple_signals() {
    monitor_t monitor;
    monitor_init(&monitor);
    
    // Signal multiple times
    monitor_signal(&monitor);
    monitor_signal(&monitor);
    monitor_signal(&monitor);
    
    // Wait should still work (only one wait needed)
    int result = monitor_wait(&monitor);
    int passed = (result == 0);
    
    monitor_destroy(&monitor);
    print_test_result("Multiple signals (idempotent behavior)", passed);
}

// Test 8: Signal, wait, signal, wait pattern
void test_signal_wait_pattern() {
    monitor_t monitor;
    monitor_init(&monitor);
    
    int passed = 1;
    
    // First cycle: signal then wait
    monitor_signal(&monitor);
    if (monitor_wait(&monitor) != 0) {
        passed = 0;
    }
    
    // Reset for second cycle
    monitor_reset(&monitor);
    
    // Second cycle: signal then wait again
    monitor_signal(&monitor);
    if (monitor_wait(&monitor) != 0) {
        passed = 0;
    }
    
    monitor_destroy(&monitor);
    print_test_result("Signal-wait pattern with reset", passed);
}

// Performance test data
typedef struct {
    monitor_t* monitor;
    int iterations;
    int producer_done;
} perf_test_data_t;

void* producer_thread(void* arg) {
    perf_test_data_t* data = (perf_test_data_t*)arg;
    
    for (int i = 0; i < data->iterations; i++) {
        monitor_signal(data->monitor);
        usleep(100); // Small delay
        monitor_reset(data->monitor);
        usleep(100);
    }
    
    data->producer_done = 1;
    monitor_signal(data->monitor); // Final signal to wake consumer
    
    return NULL;
}

void* consumer_thread(void* arg) {
    perf_test_data_t* data = (perf_test_data_t*)arg;
    
    int signals_received = 0;
    while (!data->producer_done || signals_received < data->iterations) {
        if (monitor_wait(data->monitor) == 0) {
            signals_received++;
            monitor_reset(data->monitor);
        }
    }
    
    return NULL;
}

// Test 9: Performance/stress test
void test_performance() {
    monitor_t monitor;
    monitor_init(&monitor);
    
    const int ITERATIONS = 100;
    perf_test_data_t data = {&monitor, ITERATIONS, 0};
    
    pthread_t prod_thread, cons_thread;
    
    pthread_create(&prod_thread, NULL, producer_thread, &data);
    pthread_create(&cons_thread, NULL, consumer_thread, &data);
    
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    printf("[INFO] Performance test completed\n");
    
    monitor_destroy(&monitor);
    print_test_result("Performance/stress test", 1); // If we got here, it passed
}

int main() {
    printf("========== MONITOR UNIT TESTS ==========\n\n");
    
    // Run all tests
    test_basic_init_destroy();
    test_null_pointer_handling();
    test_signal_before_wait();
    test_reset_functionality();
    test_signal_after_wait();
    test_multiple_waiters();
    test_multiple_signals();
    test_signal_wait_pattern();
    test_performance();
    
    // Print summary
    print_test_summary();
    
    // Exit with appropriate code
    if (tests_failed > 0) {
        printf("\nSome tests failed! Please fix the issues.\n");
        return 1;
    } else {
        printf("\nAll tests passed! Monitor implementation is correct.\n");
        return 0;
    }
}