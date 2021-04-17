cmake_minimum_required(VERSION 3.17)
project(Server)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread")

add_executable(Server main.cpp CMemoryPool.cpp CMemoryPool.h IMemoryBlock.h SMemoryChunk.h Variable.cpp Variable.h Split_getline.h)
