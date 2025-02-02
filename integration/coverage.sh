#!/usr/bin/bash

west twister --coverage -p native_posix -T tests --jobs 1
lcov --extract twister-out/coverage.info "*/SEN-GS-1-TM-Firmware-CGM-Zephyr/include/*" "*/SEN-GS-1-TM-Firmware-CGM-Zephyr/drivers/*" "*/SEN-GS-1-TM-Firmware-CGM-Zephyr/lib/*" --output-file twister-out/filtered_coverage.info
genhtml twister-out/filtered_coverage.info --output-directory twister-out/filtered_coverage

# To open the coverage, use either of the following commands:

# python3 -m htpp.server 80 (or any other port)

# Or search for the index.html inside the twister-out/filtered_coverage and then double click it and choose the 
# "reveal in file explorer". Then double click the file and it will open in google chrome or explorer, your choice

dir1="./include" 
dir2="./drivers"
dir3="./lib"

# File to store uncovered files
output_file="twister-out/uncovered_files.txt"

# Clear the output file if it exists
true> "$output_file"

# Get the list of actual files in the directories
actual_files=($(find "$dir1" "$dir2" "$dir3" \( -name "*.cpp" -o -name "*.h" -o -name "*.c" \)))

# Iterate over the actual files
for actual_file in "${actual_files[@]}"; do
  actual_file_name=$(basename "$actual_file")

  # Check if the actual file is NOT covered by searching in the coverage file
  if ! grep -q "SF:.*${actual_file_name}" "./twister-out/filtered_coverage.info"; then
    echo "$actual_file_name is NOT covered" >> "$output_file"
  fi
done

#To find the files that arent covered go to the twister-out dir and search for uncovered_files.txt
