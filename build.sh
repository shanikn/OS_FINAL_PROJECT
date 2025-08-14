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

# Create output directory
print_status "Creating output directory"
mkdir -p output

# Build main application
print_status "Building main application: analyzer"
gcc -o output/analyzer main.c -ldl -lpthread || {
    print_error "Failed to build main application"
    exit 1
}

# TODO: Build plugins when they are implemented
print_warning "Plugins not yet implemented - will be added as they are created"

print_status "Build completed successfully!"
print_status "Output files:"
print_status "  - output/analyzer (main executable)"