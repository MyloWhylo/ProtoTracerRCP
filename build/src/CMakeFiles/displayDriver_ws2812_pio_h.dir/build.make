# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mylowhylo/PiPico/pico/displayDriver

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mylowhylo/PiPico/pico/displayDriver/build

# Utility rule file for displayDriver_ws2812_pio_h.

# Include the progress variables for this target.
include src/CMakeFiles/displayDriver_ws2812_pio_h.dir/progress.make

src/CMakeFiles/displayDriver_ws2812_pio_h: ../src/ws2812.pio.h


../src/ws2812.pio.h: ../src/ws2812.pio
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/mylowhylo/PiPico/pico/displayDriver/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating ../../src/ws2812.pio.h"
	cd /home/mylowhylo/PiPico/pico/displayDriver/build/src && ../pioasm/pioasm -o c-sdk /home/mylowhylo/PiPico/pico/displayDriver/src/ws2812.pio /home/mylowhylo/PiPico/pico/displayDriver/src/ws2812.pio.h

displayDriver_ws2812_pio_h: src/CMakeFiles/displayDriver_ws2812_pio_h
displayDriver_ws2812_pio_h: ../src/ws2812.pio.h
displayDriver_ws2812_pio_h: src/CMakeFiles/displayDriver_ws2812_pio_h.dir/build.make

.PHONY : displayDriver_ws2812_pio_h

# Rule to build all files generated by this target.
src/CMakeFiles/displayDriver_ws2812_pio_h.dir/build: displayDriver_ws2812_pio_h

.PHONY : src/CMakeFiles/displayDriver_ws2812_pio_h.dir/build

src/CMakeFiles/displayDriver_ws2812_pio_h.dir/clean:
	cd /home/mylowhylo/PiPico/pico/displayDriver/build/src && $(CMAKE_COMMAND) -P CMakeFiles/displayDriver_ws2812_pio_h.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/displayDriver_ws2812_pio_h.dir/clean

src/CMakeFiles/displayDriver_ws2812_pio_h.dir/depend:
	cd /home/mylowhylo/PiPico/pico/displayDriver/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mylowhylo/PiPico/pico/displayDriver /home/mylowhylo/PiPico/pico/displayDriver/src /home/mylowhylo/PiPico/pico/displayDriver/build /home/mylowhylo/PiPico/pico/displayDriver/build/src /home/mylowhylo/PiPico/pico/displayDriver/build/src/CMakeFiles/displayDriver_ws2812_pio_h.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/displayDriver_ws2812_pio_h.dir/depend
