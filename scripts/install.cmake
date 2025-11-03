include_guard()

include(CMakePackageConfigHelpers)

function(ms_install)
  install(
    TARGETS moonsugar
    EXPORT moonsugar
    FILE_SET HEADERS
  )

  install(
    EXPORT moonsugar
    DESTINATION lib/cmake/moonsugar
    NAMESPACE moonsugar::
    FILE MoonsugarTargets.cmake
  )

  configure_package_config_file(
    scripts/moonsugar-config.cmake.in
    scripts/moonsugar-config.cmake
    INSTALL_DESTINATION lib/cmake/moonsugar
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/scripts/moonsugar-config.cmake
    DESTINATION lib/cmake/moonsugar
  )
endfunction()

