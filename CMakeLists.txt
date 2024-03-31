cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O2 -pedantic-errors -Wextra -Wall")

project(JRNZ)

include_directories(src/ src/z80/ include/)

# SDL needed for frontend
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Catch2 needed for unit testing
find_package(Catch2 3 REQUIRED)

add_library(z80_lib src/z80/instructions.cpp src/z80/storage_element.cpp src/z80/register.cpp src/z80/decoder.cpp src/z80/z80.cpp)
add_library(jrnz_lib src/system.cpp src/debugger.cpp src/ula.cpp src/options.cpp src/keyboard.cpp src/bus.cpp)

# Build emulator
add_executable(run_jrnz src/main.cpp)
target_link_libraries(run_jrnz ${SDL2_LIBRARIES} jrnz_lib z80_lib)

# Build unit tests
include(CTest)

add_executable(run_tests tests/test_adc.cpp tests/test_sbc.cpp tests/test_neg.cpp)
target_link_libraries(run_tests ${SDL2_LIBRARIES} Catch2::Catch2WithMain z80_lib jrnz_lib)

add_test(NAME run_tests COMMAND run_tests)
