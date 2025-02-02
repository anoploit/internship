cmake_minimum_required(VERSION 3.20.0)
set(ZEPHYR_BASE /home/sensible/actions-runner/_work/SEN-GS-1-TM-Firmware-CGM-Zephyr/zephyr) 

# Find the Zephyr package
find_package(Zephyr REQUIRED HINTS ${ZEPHYR_BASE})

project(app)

set(SOURCES
    src/main.cpp
)

# Include test directories
target_sources(app PRIVATE ${SOURCES})
