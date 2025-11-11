cmake_minimum_required(VERSION 4.1)

project(
  moonsugar
  VERSION 0.2.0
)

option(ENABLE_COMPRESS "Enable the compression module" OFF)

include(CPack)
include(scripts/target.cmake)
include(scripts/test.cmake)
include(scripts/install.cmake)

find_package(moondance CONFIG)
find_package(ZLIB REQUIRED)

add_library(
  moonsugar
  src/coroutine.c
  src/path.c
  src/memory/interface.c
  $<$<BOOL:${ENABLE_COMPRESS}>:src/compress.c>
)

target_include_directories(
  moonsugar
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if(ENABLE_COMPRESS)
  target_link_libraries(
    moonsugar
    PUBLIC
    ZLIB::ZLIB
  )
endif()

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
    include/moonsugar/memory.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/moonsugar/version.h
    $<$<BOOL:${ENABLE_COMPRESS}>:include/moonsugar/compress.h>
)

ms_configure_target(moonsugar)

if(NOT ${moondance_FOUND})
  message(WARNING "Moondance not found. Tests will be disabled.")
else()
  enable_testing()
  ms_add_test(test-coroutine test/coroutine.c)
  ms_add_test(test-path test/path.c)

  if(ENABLE_COMPRESS)
    ms_add_test(test-compress test/compress.c)
  endif()
endif()

ms_install()
