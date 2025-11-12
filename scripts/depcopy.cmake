include_guard()

function(ms_copy_dependencies)
  if(${ZLIB_USE_STATIC_LIBS})
    return()
  endif()

  if(DEFINED CACHE{MS_ZLIB_PATH})
    return()
  endif()

  find_file(
    MS_ZLIB_PATH
    NAMES zlib.dll zlib1.dll zlib.so zlib1.so
    HINTS ${CMAKE_LIBRARY_PATH} ${ZLIB_ROOT}
    PATH_SUFFIXES bin lib
    REQUIRED
  )

  message(STATUS "Copying ZLIB dependency: ${MS_ZLIB_PATH}")

  file(
    COPY ${MS_ZLIB_PATH}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
  )
endfunction()
