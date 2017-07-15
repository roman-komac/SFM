# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

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
CMAKE_SOURCE_DIR = /home/roman/Diplomska/SFM

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/roman/Diplomska/SFM

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running interactive CMake command-line interface..."
	/usr/bin/cmake -i .
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/roman/Diplomska/SFM/CMakeFiles /home/roman/Diplomska/SFM/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/roman/Diplomska/SFM/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named StructureFromMotion

# Build rule for target.
StructureFromMotion: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 StructureFromMotion
.PHONY : StructureFromMotion

# fast build rule for target.
StructureFromMotion/fast:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/build
.PHONY : StructureFromMotion/fast

SFM.o: SFM.cpp.o
.PHONY : SFM.o

# target to build an object file
SFM.cpp.o:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM.cpp.o
.PHONY : SFM.cpp.o

SFM.i: SFM.cpp.i
.PHONY : SFM.i

# target to preprocess a source file
SFM.cpp.i:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM.cpp.i
.PHONY : SFM.cpp.i

SFM.s: SFM.cpp.s
.PHONY : SFM.s

# target to generate assembly for a file
SFM.cpp.s:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM.cpp.s
.PHONY : SFM.cpp.s

SFM/Image.o: SFM/Image.cpp.o
.PHONY : SFM/Image.o

# target to build an object file
SFM/Image.cpp.o:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM/Image.cpp.o
.PHONY : SFM/Image.cpp.o

SFM/Image.i: SFM/Image.cpp.i
.PHONY : SFM/Image.i

# target to preprocess a source file
SFM/Image.cpp.i:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM/Image.cpp.i
.PHONY : SFM/Image.cpp.i

SFM/Image.s: SFM/Image.cpp.s
.PHONY : SFM/Image.s

# target to generate assembly for a file
SFM/Image.cpp.s:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM/Image.cpp.s
.PHONY : SFM/Image.cpp.s

SFM/RANSAC/ransac.o: SFM/RANSAC/ransac.cpp.o
.PHONY : SFM/RANSAC/ransac.o

# target to build an object file
SFM/RANSAC/ransac.cpp.o:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM/RANSAC/ransac.cpp.o
.PHONY : SFM/RANSAC/ransac.cpp.o

SFM/RANSAC/ransac.i: SFM/RANSAC/ransac.cpp.i
.PHONY : SFM/RANSAC/ransac.i

# target to preprocess a source file
SFM/RANSAC/ransac.cpp.i:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM/RANSAC/ransac.cpp.i
.PHONY : SFM/RANSAC/ransac.cpp.i

SFM/RANSAC/ransac.s: SFM/RANSAC/ransac.cpp.s
.PHONY : SFM/RANSAC/ransac.s

# target to generate assembly for a file
SFM/RANSAC/ransac.cpp.s:
	$(MAKE) -f CMakeFiles/StructureFromMotion.dir/build.make CMakeFiles/StructureFromMotion.dir/SFM/RANSAC/ransac.cpp.s
.PHONY : SFM/RANSAC/ransac.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... StructureFromMotion"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... SFM.o"
	@echo "... SFM.i"
	@echo "... SFM.s"
	@echo "... SFM/Image.o"
	@echo "... SFM/Image.i"
	@echo "... SFM/Image.s"
	@echo "... SFM/RANSAC/ransac.o"
	@echo "... SFM/RANSAC/ransac.i"
	@echo "... SFM/RANSAC/ransac.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

