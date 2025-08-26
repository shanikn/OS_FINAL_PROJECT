#!/bin/bash

# Exit on any error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[BUILD]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_status "Building unit tests..."

# Build monitor unit test
print_status "Building monitor unit test"
gcc monitor_test.c \
    plugins/sync/monitor.c \
    -lpthread -o monitor_test || {
    print_error "Failed to build monitor_test"
    exit 1
}

# Build consumer-producer unit test
print_status "Building consumer-producer unit test"
gcc consumer_producer_test.c \
    plugins/sync/consumer_producer.c \
    plugins/sync/monitor.c \
    -lpthread -o consumer_producer_test || {
    print_error "Failed to build consumer_producer_test"
    exit 1
}

print_status "Unit tests built successfully!"
print_status "Available unit tests:"
print_status "  - ./monitor_test"
print_status "  - ./consumer_producer_test"
print_status ""
print_status "To run tests:"
print_status "  ./monitor_test"
print_status "  ./consumer_producer_test"
