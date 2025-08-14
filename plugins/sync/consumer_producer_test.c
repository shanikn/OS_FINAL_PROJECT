#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "consumer_producer.h"

// Test configuration
#define TEST_QUEUE_SIZE 5
#define NUM_PRODUCER_THREADS 3
#define NUM_CONSUMER_THREADS 2
#define ITEMS_PER_PRODUCER 10
#define TEST_TIMEOUT_SECONDS 5

// Global test data
consumer_producer_t test_queue;
int producer_count = 0;
int consumer_count = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;


// Test result tracking
int tests_passed = 0;
int tests_failed = 0;

void print_test_result(const char* test_name, int passed) {
    if (passed) {
        printf("[PASS] %s\n", test_name);
        tests_passed++;
    } else {
        printf("[FAIL] %s\n", test_name);
        tests_failed++;
    }
}

void print_test_header(const char* test_name) {
    printf("\n=== Testing: %s ===\n", test_name);
}

// Test 1: Basic initialization and cleanup
int test_init_destroy() {
    print_test_header("Queue Initialization and Destruction");
    
    consumer_producer_t queue;
    const char* result = consumer_producer_init(&queue, TEST_QUEUE_SIZE);
    
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    // Verify initial state
    int passed = (queue.capacity == TEST_QUEUE_SIZE && 
                  queue.count == 0 && 
                  queue.head == 0 && 
                  queue.tail == 0 &&
                  queue.items != NULL);
    
    consumer_producer_destroy(&queue);
    return passed;
}

// Test 2: Invalid initialization parameters
int test_invalid_init() {
    print_test_header("Invalid Initialization Parameters");
    
    consumer_producer_t queue;
    
    // Test NULL queue
    const char* result1 = consumer_producer_init(NULL, TEST_QUEUE_SIZE);
    
    // Test zero capacity
    const char* result2 = consumer_producer_init(&queue, 0);
    
    // Test negative capacity
    const char* result3 = consumer_producer_init(&queue, -1);
    
    return (result1 != NULL && result2 != NULL && result3 != NULL);
}

// Test 3: Basic put and get operations
int test_basic_put_get() {
    print_test_header("Basic Put and Get Operations");
    
    consumer_producer_t queue;
    const char* result = consumer_producer_init(&queue, TEST_QUEUE_SIZE);
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    // Test putting and getting a single item
    const char* test_string = "Hello, World!";
    result = consumer_producer_put(&queue, test_string);
    if (result != NULL) {
        printf("Put failed: %s\n", result);
        consumer_producer_destroy(&queue);
        return 0;
    }
    
    char* retrieved = consumer_producer_get(&queue);
    int passed = (retrieved != NULL && strcmp(retrieved, test_string) == 0);
    
    if (retrieved) {
        free(retrieved);
    }
    
    consumer_producer_destroy(&queue);
    return passed;
}

// Test 4: Queue capacity limits
int test_queue_capacity() {
    print_test_header("Queue Capacity Limits");
    
    consumer_producer_t queue;
    const char* result = consumer_producer_init(&queue, 2); // Small queue
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    // Fill the queue to capacity
    result = consumer_producer_put(&queue, "Item 1");
    if (result != NULL) {
        consumer_producer_destroy(&queue);
        return 0;
    }
    
    result = consumer_producer_put(&queue, "Item 2");
    if (result != NULL) {
        consumer_producer_destroy(&queue);
        return 0;
    }
    
    // Verify queue is full (count should equal capacity)
    int passed = (queue.count == queue.capacity);
    
    // Clean up
    char* item1 = consumer_producer_get(&queue);
    char* item2 = consumer_producer_get(&queue);
    
    if (item1) free(item1);
    if (item2) free(item2);
    
    consumer_producer_destroy(&queue);
    return passed;
}

// Test 5: FIFO order verification
int test_fifo_order() {
    print_test_header("FIFO Order Verification");
    
    consumer_producer_t queue;
    const char* result = consumer_producer_init(&queue, TEST_QUEUE_SIZE);
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    // Put multiple items
    const char* items[] = {"First", "Second", "Third", "Fourth"};
    int num_items = sizeof(items) / sizeof(items[0]);
    
    for (int i = 0; i < num_items; i++) {
        result = consumer_producer_put(&queue, items[i]);
        if (result != NULL) {
            printf("Put failed for item %d: %s\n", i, result);
            consumer_producer_destroy(&queue);
            return 0;
        }
    }
    
    // Get items and verify order
    int passed = 1;
    for (int i = 0; i < num_items; i++) {
        char* retrieved = consumer_producer_get(&queue);
        if (!retrieved || strcmp(retrieved, items[i]) != 0) {
            printf("FIFO violation: expected '%s', got '%s'\n", 
                   items[i], retrieved ? retrieved : "NULL");
            passed = 0;
        }
        if (retrieved) free(retrieved);
    }
    
    consumer_producer_destroy(&queue);
    return passed;
}

// Producer thread function for concurrent tests
void* producer_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        char* item = malloc(64);
        snprintf(item, 64, "Producer%d-Item%d", thread_id, i);
        
        const char* result = consumer_producer_put(&test_queue, item);
        if (result != NULL) {
            printf("Producer %d failed to put item %d: %s\n", thread_id, i, result);
            free(item);
            return NULL;
        }
        
        pthread_mutex_lock(&count_mutex);
        producer_count++;
        pthread_mutex_unlock(&count_mutex);
        
        // Small delay to increase chance of race conditions
        usleep(1000);
    }
    
    return NULL;
}

// Consumer thread function for concurrent tests
void* consumer_thread(void* arg) {
    int thread_id = *(int*)arg;
    
    while (1) {
        char* item = consumer_producer_get(&test_queue);
        if (!item) {
            break;
        }
        
        // Check if this is the end signal
        if (strcmp(item, "<END>") == 0) {
            free(item);
            break;
        }
        
        pthread_mutex_lock(&count_mutex);
        consumer_count++;
        pthread_mutex_unlock(&count_mutex);
        
        printf("Consumer %d got: %s\n", thread_id, item);
        free(item);
        
        // Small delay to increase chance of race conditions
        usleep(1500);
    }
    
    return NULL;
}

// Test 6: Concurrent producers and consumers
int test_concurrent_operations() {
    print_test_header("Concurrent Producers and Consumers");
    
    const char* result = consumer_producer_init(&test_queue, TEST_QUEUE_SIZE);
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    // Reset counters
    producer_count = 0;
    consumer_count = 0;
    
    // Create producer threads
    pthread_t producers[NUM_PRODUCER_THREADS];
    int producer_ids[NUM_PRODUCER_THREADS];
    
    for (int i = 0; i < NUM_PRODUCER_THREADS; i++) {
        producer_ids[i] = i;
        if (pthread_create(&producers[i], NULL, producer_thread, &producer_ids[i]) != 0) {
            printf("Failed to create producer thread %d\n", i);
            return 0;
        }
    }
    
    // Create consumer threads
    pthread_t consumers[NUM_CONSUMER_THREADS];
    int consumer_ids[NUM_CONSUMER_THREADS];
    
    for (int i = 0; i < NUM_CONSUMER_THREADS; i++) {
        consumer_ids[i] = i;
        if (pthread_create(&consumers[i], NULL, consumer_thread, &consumer_ids[i]) != 0) {
            printf("Failed to create consumer thread %d\n", i);
            return 0;
        }
    }
    
    // Wait for all producers to finish
    for (int i = 0; i < NUM_PRODUCER_THREADS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Send end signals to consumers
    for (int i = 0; i < NUM_CONSUMER_THREADS; i++) {
        consumer_producer_put(&test_queue, "<END>");
    }
    
    // Wait for all consumers to finish
    for (int i = 0; i < NUM_CONSUMER_THREADS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    // Verify all items were produced and consumed
    int expected_items = NUM_PRODUCER_THREADS * ITEMS_PER_PRODUCER;
    int passed = (producer_count == expected_items && consumer_count == expected_items);
    
    if (!passed) {
        printf("Expected %d items produced and consumed, got %d produced, %d consumed\n",
               expected_items, producer_count, consumer_count);
    }
    
    consumer_producer_destroy(&test_queue);
    return passed;
}

// Test 7: Finished signal functionality
int test_finished_signal() {
    print_test_header("Finished Signal Functionality");
    
    consumer_producer_t queue;
    const char* result = consumer_producer_init(&queue, TEST_QUEUE_SIZE);
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    // Signal finished
    consumer_producer_signal_finished(&queue);
    
    // Wait for finished (should return immediately since it was just signaled)
    int wait_result = consumer_producer_wait_finished(&queue);
    
    consumer_producer_destroy(&queue);
    return (wait_result == 0);
}

// Test 8: Memory stress test
int test_memory_stress() {
    print_test_header("Memory Stress Test");
    
    consumer_producer_t queue;
    const char* result = consumer_producer_init(&queue, 100); // Larger queue
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    const int STRESS_ITEMS = 1000;
    
    // Put and immediately get many items
    for (int i = 0; i < STRESS_ITEMS; i++) {
        char* item = malloc(64);
        snprintf(item, 64, "StressItem%d", i);
        
        result = consumer_producer_put(&queue, item);
        if (result != NULL) {
            printf("Stress test put failed at item %d: %s\n", i, result);
            free(item);
            consumer_producer_destroy(&queue);
            return 0;
        }
        
        char* retrieved = consumer_producer_get(&queue);
        if (!retrieved || strcmp(retrieved, item) != 0) {
            printf("Stress test get failed at item %d\n", i);
            if (retrieved) free(retrieved);
            consumer_producer_destroy(&queue);
            return 0;
        }
        
        free(retrieved);
    }
    
    consumer_producer_destroy(&queue);
    return 1;
}

// Test 9: Edge cases
int test_edge_cases() {
    print_test_header("Edge Cases");
    
    consumer_producer_t queue;
    const char* result = consumer_producer_init(&queue, TEST_QUEUE_SIZE);
    if (result != NULL) {
        printf("Init failed: %s\n", result);
        return 0;
    }
    
    int passed = 1;
    
    // Test empty string
    result = consumer_producer_put(&queue, "");
    if (result != NULL) {
        printf("Failed to put empty string: %s\n", result);
        passed = 0;
    } else {
        char* retrieved = consumer_producer_get(&queue);
        if (!retrieved || strcmp(retrieved, "") != 0) {
            printf("Failed to get empty string correctly\n");
            passed = 0;
        }
        if (retrieved) free(retrieved);
    }
    
    // Test very long string
    char* long_string = malloc(2048);
    memset(long_string, 'A', 2047);
    long_string[2047] = '\0';
    
    result = consumer_producer_put(&queue, long_string);
    if (result != NULL) {
        printf("Failed to put long string: %s\n", result);
        passed = 0;
    } else {
        char* retrieved = consumer_producer_get(&queue);
        if (!retrieved || strcmp(retrieved, long_string) != 0) {
            printf("Failed to get long string correctly\n");
            passed = 0;
        }
        if (retrieved) free(retrieved);
    }
    
    free(long_string);
    consumer_producer_destroy(&queue);
    return passed;
}

int main() {
    printf("=== Consumer-Producer Queue Unit Tests ===\n");
    printf("Testing comprehensive functionality of the queue implementation...\n");
    
    // Run all tests
    print_test_result("Queue Initialization and Destruction", test_init_destroy());
    print_test_result("Invalid Initialization Parameters", test_invalid_init());
    print_test_result("Basic Put and Get Operations", test_basic_put_get());
    print_test_result("Queue Capacity Limits", test_queue_capacity());
    print_test_result("FIFO Order Verification", test_fifo_order());
    //print_test_result("Concurrent Producers and Consumers", test_concurrent_operations());
    print_test_result("Finished Signal Functionality", test_finished_signal());
    print_test_result("Memory Stress Test", test_memory_stress());
    print_test_result("Edge Cases", test_edge_cases());
    
    // Print summary
    printf("\n=== Test Summary ===\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("Total tests: %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\nAll tests passed! Your consumer-producer queue is working correctly.\n");
        return 0;
    } else {
        printf("\nSome tests failed. Please review your implementation.\n");
        return 1;
    }
}