cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O2 -pedantic-errors -Wextra -Wall")

project(JRNZ)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(Boost CONFIG COMPONENTS system filesystem unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_library(jrnz_lib src/instructions.cpp src/storage_element.cpp src/register.cpp src/z80.cpp src/system.cpp src/debugger.cpp src/decoder.cpp src/ula.cpp src/options.cpp src/keyboard.cpp src/bus.cpp)

add_executable(run_jrnz src/main.cpp)
target_link_libraries(run_jrnz ${SDL2_LIBRARIES} jrnz_lib)

add_executable(run_tests src/test_main.cpp src/test_adc.cpp src/test_sbc.cpp)
target_link_libraries(run_tests ${SDL2_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} jrnz_lib)
