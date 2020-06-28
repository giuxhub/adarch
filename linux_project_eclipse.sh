#!/bin/sh

# For Unix (OSX and Linux) Eclipse

# Create folder and generate Eclipse project
mkdir -p eclipse
cd eclipse
cmake .. -G "Eclipse CDT4 - Unix Makefiles"

# Build project
make -j 4
