#!/bin/bash
set -e

echo "Building plugin_common unit test..."

# Create output directory if it doesn't exist
mkdir -p output

# Build the test executable
gcc -o output/plugin_common_test \
    plugin_common_test.c \
    plugins/plugin_common.c \
    plugins/sync/monitor.c \
    plugins/sync/consumer_producer.c \
    -ldl -lpthread

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "Run with: ./output/plugin_common_test"
else
    echo "❌ Build failed!"
    exit 1
fi