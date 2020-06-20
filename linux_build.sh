#!/bin/sh

# For Unix (OSX and Linux)

# Create folder and generate Eclipse project
mkdir -p build
cd build
cmake .. -G "Unix Makefiles"

# Build project
make -j 4
