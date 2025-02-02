cmake_minimum_required(VERSION 3.20.0)
# To do: Fix this issue
# set(ZEPHYR_BASE /home/sensible/actions-runner/_work/SEN-GS-1-TM-Firmware-CGM-Zephyr/zephyr)

# Find the Zephyr package
find_package(Zephyr REQUIRED HINTS ${ZEPHYR_BASE})

# Enable Google Test for unit testing
project(app)

set(SOURCES
    src/test.cpp
)

# Include test directories
target_sources(app PRIVATE ${SOURCES})

