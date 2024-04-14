cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 20)
# Code relies on casting char to int extends the sign. This is probably the most
# portable way of doing this and probably should be investigated further
set(CMAKE_CXX_FLAGS "-O2 -pedantic-errors -Wextra -Wall -fsigned-char")

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
else()
  message(STATUS "Using Catch2 library")
  find_package(Catch2 3 REQUIRED)
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

add_executable(run_tests tests/test_adc.cpp tests/test_sbc.cpp
                         tests/test_neg.cpp)
target_link_libraries(
  run_tests
  z80_lib
  jrnz_lib
  ${SDL2_LIBRARIES}
  Catch2::Catch2WithMain
  z80_lib
  jrnz_lib)

add_test(NAME run_tests COMMAND run_tests)
