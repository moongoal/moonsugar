cmake_minimum_required(VERSION 4.1)

project(
  moonsugar
  VERSION 0.4.0
)

option(ENABLE_COMPRESS "Enable the compression module" ON)

include(CPack)
include(scripts/target.cmake)
include(scripts/test.cmake)
include(scripts/install.cmake)
include(scripts/depcopy.cmake)

find_package(moondance CONFIG)
find_package(ZLIB REQUIRED)

add_library(
  moonsugar
  src/coroutine.c
  src/path.c
  src/memory/interface.c
  src/uri.c
  src/config.c
  src/platform/sys/sys.c
  src/log/log.c
  src/log/console.c
  src/log/file.c
  src/memory/heap.c
  src/memory/stack.c
  src/memory/freelist.c
  src/memory/arena.c
  $<$<BOOL:${ENABLE_COMPRESS}>:src/compress.c>
)

target_include_directories(
  moonsugar
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_compile_definitions(moonsugar PRIVATE MSLIB)

if(ENABLE_COMPRESS)
  target_link_libraries(
    moonsugar
    PRIVATE 
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
    include/moonsugar/assert.h
    include/moonsugar/log.h
    include/moonsugar/platform.h
    include/moonsugar/util.h
    include/moonsugar/coroutine.h
    include/moonsugar/path.h
    include/moonsugar/memory.h
    include/moonsugar/uri.h
    include/moonsugar/config.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/moonsugar/version.h
    $<$<BOOL:${ENABLE_COMPRESS}>:include/moonsugar/compress.h>
)

if(WIN32)
  target_sources(
    moonsugar
    PRIVATE 
      src/platform/memory/os.win32.c
      src/platform/sys/sys.win32.c
  )
elseif(UNIX)
  target_sources(
    moonsugar
    PRIVATE 
      src/platform/memory/os.unix.c
  )
endif()

if(APPLE)
  target_sources(
    moonsugar
    PRIVATE
      src/platform/memory/os.osx.c
  )
endif()

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  target_sources(
    moonsugar
    PRIVATE
      src/platform/sys/sys.x86.c
  )
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
  target_sources(
    moonsugar
    PRIVATE
      src/platform/sys/sys.arm.c
  )
endif()

ms_configure_target(moonsugar)

if(NOT ${moondance_FOUND})
  message(WARNING "Moondance not found. Tests will be disabled.")
else()
  enable_testing()
  ms_add_test(test-coroutine test/coroutine.c)
  ms_add_test(test-path test/path.c)
  ms_add_test(test-uri test/uri.c)
  ms_add_test(test-config test/config.c)
  ms_add_test(test-log test/log.c)
  ms_add_test(test-util test/util.c)

  ms_add_test(test-memory-os test/memory/os.c)
  ms_add_test(test-memory-heap test/memory/heap.c)

  if(ENABLE_COMPRESS)
    ms_add_test(test-compress test/compress.c)
  endif()
endif()

ms_install()
ms_copy_dependencies()
