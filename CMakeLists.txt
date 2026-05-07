cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 20)
# Code relies on casting char to int extends the sign. This is probably the most
# portable way of doing this and probably should be investigated further
set(CMAKE_CXX_FLAGS_RELEASE_INIT
    "-O2 -pedantic-errors -Wextra -Wall -fsigned-char")

project(JRNZ)

include_directories(src/ src/z80/ include/)

# SDL needed for frontend
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Catch2 needed for unit testing
option(USE_VENDORED_CATCH2 "Download Catch2 from GitHub" ON)
if(USE_VENDORED_CATCH2)
  message(STATUS "Using vendored Catch2")
  include(FetchContent)

  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0 # or a later release
  )

  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
else()
  message(STATUS "Using Catch2 library")
  find_package(Catch2 3 REQUIRED)
  list(APPEND CMAKE_MODULE_PATH ${Catch2_DIR})
endif()

add_library(z80_lib src/z80/instructions.cpp src/z80/storage_element.cpp
                    src/z80/register.cpp src/z80/decoder.cpp src/z80/z80.cpp)
add_library(
  jrnz_lib
  src/system.cpp
  src/debugger.cpp
  src/ula.cpp
  src/options.cpp
  src/keyboard.cpp
  src/bus.cpp
  src/formats/format_sna.cpp
  src/formats/format_z80.cpp)

# Build emulator
add_executable(run_jrnz src/main.cpp)
target_link_libraries(run_jrnz ${SDL2_LIBRARIES} jrnz_lib z80_lib)

# Build unit tests
include(CTest)
include(Catch)

add_executable(
  run_tests
  tests/test_adc.cpp
  tests/test_sbc.cpp
  tests/test_neg.cpp
  tests/test_snapshot_sna.cpp
  tests/test_snapshot_z80.cpp
  tests/test_decoder.cpp
  tests/test_control_flow.cpp
  tests/test_flags_misc.cpp
  tests/test_flags_matrix.cpp
  tests/test_execution_control.cpp
  tests/test_interrupt_matrix.cpp
  tests/test_execution_control_matrix.cpp
  tests/test_execution_alu.cpp
  tests/test_execution_alu_source_matrix.cpp
  tests/test_execution_arithmetic16.cpp
  tests/test_execution_loads.cpp
  tests/test_execution_extended.cpp
  tests/test_execution_blocks_ports.cpp
  tests/test_execution_block_io.cpp
  tests/test_execution_rotate_shift.cpp
  tests/test_execution_nibbles.cpp
  tests/test_execution_ports_matrix.cpp
  tests/test_execution_misc_documented.cpp
  tests/test_execution_family_matrix.cpp
  tests/test_execution_16bit_indexed_matrix.cpp
  tests/test_execution_index_pairs_misc.cpp
  tests/test_execution_indexed_cb_matrix.cpp
  tests/test_execution_bitsetres.cpp
  tests/test_execution_exchange_stack.cpp
  tests/test_execution_index_halves.cpp
  tests/test_execution_index_halves_matrix.cpp)
target_link_libraries(
  run_tests
  z80_lib
  jrnz_lib
  ${SDL2_LIBRARIES}
  Catch2::Catch2WithMain
  z80_lib
  jrnz_lib)

catch_discover_tests(run_tests)

add_executable(run_tests_undocumented tests/test_undocumented_opcodes.cpp
                                      tests/test_undocumented_matrix.cpp)
target_link_libraries(
  run_tests_undocumented
  z80_lib
  jrnz_lib
  ${SDL2_LIBRARIES}
  Catch2::Catch2WithMain
  z80_lib
  jrnz_lib)

catch_discover_tests(run_tests_undocumented TEST_PREFIX "undocumented::")

add_executable(
  run_tests_undocumented_flags tests/test_undocumented_flags.cpp
                               tests/test_undocumented_flags_matrix.cpp)
target_link_libraries(
  run_tests_undocumented_flags
  z80_lib
  jrnz_lib
  ${SDL2_LIBRARIES}
  Catch2::Catch2WithMain
  z80_lib
  jrnz_lib)

catch_discover_tests(run_tests_undocumented_flags TEST_PREFIX
                     "undocumented-flags::")
