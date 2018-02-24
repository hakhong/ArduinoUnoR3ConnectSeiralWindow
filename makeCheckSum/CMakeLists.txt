cmake_minimum_required(VERSION 3.5)
project(makeCheckSum)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(makeCheckSum CheckSum.cpp)
set_target_properties(makeCheckSum PROPERTIES OUTPUT_NAME makeChecksum.exe)