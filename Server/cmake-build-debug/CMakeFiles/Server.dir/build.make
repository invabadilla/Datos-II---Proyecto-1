# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/clion/149/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/149/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/Server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Server.dir/flags.make

CMakeFiles/Server.dir/main.cpp.o: CMakeFiles/Server.dir/flags.make
CMakeFiles/Server.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Server.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Server.dir/main.cpp.o -c "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/main.cpp"

CMakeFiles/Server.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Server.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/main.cpp" > CMakeFiles/Server.dir/main.cpp.i

CMakeFiles/Server.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Server.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/main.cpp" -o CMakeFiles/Server.dir/main.cpp.s

CMakeFiles/Server.dir/CMemoryPool.cpp.o: CMakeFiles/Server.dir/flags.make
CMakeFiles/Server.dir/CMemoryPool.cpp.o: ../CMemoryPool.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/Server.dir/CMemoryPool.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Server.dir/CMemoryPool.cpp.o -c "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/CMemoryPool.cpp"

CMakeFiles/Server.dir/CMemoryPool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Server.dir/CMemoryPool.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/CMemoryPool.cpp" > CMakeFiles/Server.dir/CMemoryPool.cpp.i

CMakeFiles/Server.dir/CMemoryPool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Server.dir/CMemoryPool.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/CMemoryPool.cpp" -o CMakeFiles/Server.dir/CMemoryPool.cpp.s

CMakeFiles/Server.dir/Variable.cpp.o: CMakeFiles/Server.dir/flags.make
CMakeFiles/Server.dir/Variable.cpp.o: ../Variable.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/Server.dir/Variable.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Server.dir/Variable.cpp.o -c "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/Variable.cpp"

CMakeFiles/Server.dir/Variable.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Server.dir/Variable.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/Variable.cpp" > CMakeFiles/Server.dir/Variable.cpp.i

CMakeFiles/Server.dir/Variable.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Server.dir/Variable.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/Variable.cpp" -o CMakeFiles/Server.dir/Variable.cpp.s

# Object files for target Server
Server_OBJECTS = \
"CMakeFiles/Server.dir/main.cpp.o" \
"CMakeFiles/Server.dir/CMemoryPool.cpp.o" \
"CMakeFiles/Server.dir/Variable.cpp.o"

# External object files for target Server
Server_EXTERNAL_OBJECTS =

Server: CMakeFiles/Server.dir/main.cpp.o
Server: CMakeFiles/Server.dir/CMemoryPool.cpp.o
Server: CMakeFiles/Server.dir/Variable.cpp.o
Server: CMakeFiles/Server.dir/build.make
Server: /usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5.12.8
Server: /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5.12.8
Server: /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.12.8
Server: CMakeFiles/Server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable Server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Server.dir/build: Server

.PHONY : CMakeFiles/Server.dir/build

CMakeFiles/Server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Server.dir/clean

CMakeFiles/Server.dir/depend:
	cd "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server" "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server" "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug" "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug" "/home/ingrid/Documents/Proyecto 1/Datos-II---Proyecto-1/Server/cmake-build-debug/CMakeFiles/Server.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/Server.dir/depend

