# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

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
CMAKE_COMMAND = /home/utnso/Documentos/cmake-3.15.3/bin/cmake

# The command to remove a file.
RM = /home/utnso/Documentos/cmake-3.15.3/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/cliente.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/cliente.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cliente.dir/flags.make

CMakeFiles/cliente.dir/cliente.c.o: CMakeFiles/cliente.dir/flags.make
CMakeFiles/cliente.dir/cliente.c.o: ../cliente.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/cliente.dir/cliente.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cliente.dir/cliente.c.o   -c "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cliente.c"

CMakeFiles/cliente.dir/cliente.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cliente.dir/cliente.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cliente.c" > CMakeFiles/cliente.dir/cliente.c.i

CMakeFiles/cliente.dir/cliente.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cliente.dir/cliente.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cliente.c" -o CMakeFiles/cliente.dir/cliente.c.s

# Object files for target cliente
cliente_OBJECTS = \
"CMakeFiles/cliente.dir/cliente.c.o"

# External object files for target cliente
cliente_EXTERNAL_OBJECTS =

cliente: CMakeFiles/cliente.dir/cliente.c.o
cliente: CMakeFiles/cliente.dir/build.make
cliente: /usr/lib/libaltaLibreria.so
cliente: /usr/lib/libcommons.so
cliente: CMakeFiles/cliente.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable cliente"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cliente.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cliente.dir/build: cliente

.PHONY : CMakeFiles/cliente.dir/build

CMakeFiles/cliente.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cliente.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cliente.dir/clean

CMakeFiles/cliente.dir/depend:
	cd "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente" "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente" "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cmake-build-debug" "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cmake-build-debug" "/home/utnso/Documentos/tp-2019-2c-418-status-code/File System/src/SAC-cli/cliente/cmake-build-debug/CMakeFiles/cliente.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/cliente.dir/depend
