# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/robot/Documents/DifferentialDrive

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/robot/Documents/DifferentialDrive/build

# Include any dependencies generated for this target.
include CMakeFiles/controller.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/controller.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/controller.dir/flags.make

CMakeFiles/controller.dir/src/controllers.cpp.o: CMakeFiles/controller.dir/flags.make
CMakeFiles/controller.dir/src/controllers.cpp.o: ../src/controllers.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/robot/Documents/DifferentialDrive/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/controller.dir/src/controllers.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/controller.dir/src/controllers.cpp.o -c /home/robot/Documents/DifferentialDrive/src/controllers.cpp

CMakeFiles/controller.dir/src/controllers.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/controller.dir/src/controllers.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/robot/Documents/DifferentialDrive/src/controllers.cpp > CMakeFiles/controller.dir/src/controllers.cpp.i

CMakeFiles/controller.dir/src/controllers.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/controller.dir/src/controllers.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/robot/Documents/DifferentialDrive/src/controllers.cpp -o CMakeFiles/controller.dir/src/controllers.cpp.s

CMakeFiles/controller.dir/src/controllers.cpp.o.requires:

.PHONY : CMakeFiles/controller.dir/src/controllers.cpp.o.requires

CMakeFiles/controller.dir/src/controllers.cpp.o.provides: CMakeFiles/controller.dir/src/controllers.cpp.o.requires
	$(MAKE) -f CMakeFiles/controller.dir/build.make CMakeFiles/controller.dir/src/controllers.cpp.o.provides.build
.PHONY : CMakeFiles/controller.dir/src/controllers.cpp.o.provides

CMakeFiles/controller.dir/src/controllers.cpp.o.provides.build: CMakeFiles/controller.dir/src/controllers.cpp.o


# Object files for target controller
controller_OBJECTS = \
"CMakeFiles/controller.dir/src/controllers.cpp.o"

# External object files for target controller
controller_EXTERNAL_OBJECTS =

lib/libcontroller.so: CMakeFiles/controller.dir/src/controllers.cpp.o
lib/libcontroller.so: CMakeFiles/controller.dir/build.make
lib/libcontroller.so: CMakeFiles/controller.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/robot/Documents/DifferentialDrive/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library lib/libcontroller.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/controller.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/controller.dir/build: lib/libcontroller.so

.PHONY : CMakeFiles/controller.dir/build

CMakeFiles/controller.dir/requires: CMakeFiles/controller.dir/src/controllers.cpp.o.requires

.PHONY : CMakeFiles/controller.dir/requires

CMakeFiles/controller.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/controller.dir/cmake_clean.cmake
.PHONY : CMakeFiles/controller.dir/clean

CMakeFiles/controller.dir/depend:
	cd /home/robot/Documents/DifferentialDrive/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/robot/Documents/DifferentialDrive /home/robot/Documents/DifferentialDrive /home/robot/Documents/DifferentialDrive/build /home/robot/Documents/DifferentialDrive/build /home/robot/Documents/DifferentialDrive/build/CMakeFiles/controller.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/controller.dir/depend

