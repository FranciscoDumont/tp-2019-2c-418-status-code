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
CMAKE_SOURCE_DIR = /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor

# Include any dependencies generated for this target.
include CMakeFiles/servidor.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/servidor.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/servidor.dir/flags.make

CMakeFiles/servidor.dir/servidor.c.o: CMakeFiles/servidor.dir/flags.make
CMakeFiles/servidor.dir/servidor.c.o: servidor.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/servidor.dir/servidor.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/servidor.dir/servidor.c.o   -c /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor/servidor.c

CMakeFiles/servidor.dir/servidor.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/servidor.dir/servidor.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor/servidor.c > CMakeFiles/servidor.dir/servidor.c.i

CMakeFiles/servidor.dir/servidor.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/servidor.dir/servidor.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor/servidor.c -o CMakeFiles/servidor.dir/servidor.c.s

# Object files for target servidor
servidor_OBJECTS = \
"CMakeFiles/servidor.dir/servidor.c.o"

# External object files for target servidor
servidor_EXTERNAL_OBJECTS =

servidor: CMakeFiles/servidor.dir/servidor.c.o
servidor: CMakeFiles/servidor.dir/build.make
servidor: /usr/lib/libaltaLibreria.so
servidor: /usr/lib/libcommons.so
servidor: CMakeFiles/servidor.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable servidor"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/servidor.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/servidor.dir/build: servidor

.PHONY : CMakeFiles/servidor.dir/build

CMakeFiles/servidor.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/servidor.dir/cmake_clean.cmake
.PHONY : CMakeFiles/servidor.dir/clean

CMakeFiles/servidor.dir/depend:
	cd /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor /home/utnso/Documentos/tp-2019-2c-418-status-code/Ejemplos/servidor/CMakeFiles/servidor.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/servidor.dir/depend

