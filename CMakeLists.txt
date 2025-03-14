# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Find Zephyr base, required for Zephyr projects
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Define the project name
project(HW3)

# Gather all .c files from the src directory
file(GLOB SOURCES "src/*.c")

# Gather all .h files from the inc directory (not directly added to the target, just for inclusion)
file(GLOB HEADERS "inc/*.h")

# Specify the source files for the 'app' target
target_sources(app PRIVATE ${SOURCES})

# Include the headers directory for the 'app' target
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/inc)

