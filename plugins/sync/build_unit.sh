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

# Create output directory if it doesn't exist
mkdir -p output

# Build monitor unit test
print_status "Building monitor unit test"
gcc -o ../../output/monitor_test monitor_test.c \
    monitor.c \
    -lpthread || {
    print_error "Failed to build monitor_test"
    exit 1
}

# Build consumer-producer unit test
print_status "Building consumer-producer unit test"
gcc -o ../../output/consumer_producer_test consumer_producer_test.c \
    consumer_producer.c \
    monitor.c \
    -lpthread || {
    print_error "Failed to build consumer_producer_test"
    exit 1
}

print_status "Unit tests built successfully!"
print_status "Available unit tests:"
print_status "  - output/monitor_test"
print_status "  - output/consumer_producer_test"