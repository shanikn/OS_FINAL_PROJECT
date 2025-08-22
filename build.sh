#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status(){
    echo -e "${GREEN}[BUILD]${NC} $1"
}

print_warning(){
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error(){
    echo -e "${RED}[ERROR]${NC} $1"
}

# Create output directory
mkdir -p output

# Build monitor test
print_status "Building monitor test..."
gcc -o output/monitor_test \
    plugins/sync/monitor_test.c \
    plugins/sync/monitor.c \
    -lpthread || {
    print_error "Failed to build monitor_test"
    exit 1
}

# Build consumer_producer test
print_status "Building consumer_producer test..."
gcc -o output/consumer_producer_test \
    plugins/sync/consumer_producer_test.c \
    plugins/sync/consumer_producer.c \
    plugins/sync/monitor.c \
    -lpthread || {
    print_error "Failed to build consumer_producer_test"
    exit 1
}

# Build logger plugin
print_status "Building logger plugin..."
gcc -fPIC -shared -o output/logger.so \
    plugins/logger.c \
    plugins/plugin_common.c \
    plugins/sync/monitor.c \
    plugins/sync/consumer_producer.c \
    -ldl -lpthread || {
    print_error "Failed to build logger plugin"
    exit 1
}

print_status "All builds completed successfully"
