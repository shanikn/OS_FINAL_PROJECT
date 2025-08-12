#!/bin/bash

#compiles the main application into the output directory

set -e


# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
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

print_status "Monitor test built successfully"
