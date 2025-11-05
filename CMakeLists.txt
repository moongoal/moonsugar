cmake_minimum_required(VERSION 4.1)

project(
  moonsugar
  VERSION 0.1.0
)

include(CPack)
include(scripts/target.cmake)
include(scripts/test.cmake)
include(scripts/install.cmake)

find_package(moondance CONFIG)

add_library(
  moonsugar
  src/coroutine.c
  src/path.c
)

target_include_directories(
  moonsugar
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

configure_file(
  include/moonsugar/version.h.in
  include/moonsugar/version.h
  NEWLINE_STYLE UNIX
)

target_sources(
  moonsugar
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS include ${CMAKE_CURRENT_BINARY_DIR}/include
  FILES
    include/moonsugar/api.h
    include/moonsugar/coroutine.h
    include/moonsugar/path.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/moonsugar/version.h
)

ms_configure_target(moonsugar)

if(NOT ${moondance_FOUND})
  message(WARNING "Moondance not found. Tests will be disabled.")
else()
  enable_testing()
  ms_add_test(test-coroutine test/coroutine.c)
  ms_add_test(test-path test/path.c)
endif()

ms_install()
