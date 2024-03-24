cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O2 -pedantic-errors -Wextra -Wall")

project(JRNZ)

include_directories(src/)

# SDL needed for frontend
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Catch2 needed for unit testing
find_package(Catch2 3 REQUIRED)

add_library(jrnz_lib src/instructions.cpp src/storage_element.cpp src/register.cpp src/z80.cpp src/system.cpp src/debugger.cpp src/decoder.cpp src/ula.cpp src/options.cpp src/keyboard.cpp src/bus.cpp)

add_executable(run_jrnz src/main.cpp)
target_link_libraries(run_jrnz ${SDL2_LIBRARIES} jrnz_lib)

add_executable(run_tests tests/test_adc.cpp tests/test_sbc.cpp)
target_link_libraries(run_tests ${SDL2_LIBRARIES} Catch2::Catch2WithMain jrnz_lib)
