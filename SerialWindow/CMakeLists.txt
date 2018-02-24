cmake_minimum_required(VERSION 3.5)
project(ConsoleSerialWindow)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(ConsoleSerialWindow SerialClass.h SerialClass.cpp SerialConnect.cpp)
set_target_properties(ConsoleSerialWindow PROPERTIES OUTPUT_NAME ConsoleSerialWindow.exe)