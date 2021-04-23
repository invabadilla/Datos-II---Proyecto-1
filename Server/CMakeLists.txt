cmake_minimum_required(VERSION 3.17)
project(Server)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread")
find_package(Qt5 COMPONENTS Widgets REQUIRED)
add_executable(Server mServer.cpp CMemoryPool.cpp CMemoryPool.h IMemoryBlock.h SMemoryChunk.h Variable.cpp Variable.h Split_getline.h json.hpp Variable_struct.cpp Variable_struct.h)
target_link_libraries(Server PRIVATE Qt5::Widgets)
