cmake_minimum_required(VERSION 3.16)
project(BootScope VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bootscope src/main.cpp)
target_compile_options(bootscope PRIVATE -Wall -Wextra -Wpedantic)
install(TARGETS bootscope RUNTIME DESTINATION bin)
