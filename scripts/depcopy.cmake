include_guard()

function(ms_copy_dependencies)
  if(NOT ENABLE_COMPRESS)
    return()
  endif()

  if(ZLIB_USE_STATIC_LIBS)
    return()
  endif()

  if(DEFINED CACHE{MS_ZLIB_PATH})
    return()
  endif()

  find_file(
    MS_ZLIB_PATH
    NAMES zlib.dll zlib1.dll zlib.so zlib1.so libz.1.dylib
    PATH_SUFFIXES bin lib
    REQUIRED
  )

  message(STATUS "Copying ZLIB dependency: ${MS_ZLIB_PATH}")

  file(
    COPY ${MS_ZLIB_PATH}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
  )
endfunction()
