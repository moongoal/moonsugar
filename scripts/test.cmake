include_guard()

include(scripts/target.cmake)

function(ms_add_test name source)
  add_executable(${name} ${source})
  target_compile_definitions(${name} PRIVATE MS_TEST_SUITE)
  target_link_libraries(${name} PRIVATE moondance::moondance moonsugar)
  ms_configure_target(${name})
  
  add_test(NAME ${name} COMMAND ${name})
endfunction()

function(ms_build_test_plugin)
  add_library(test-plugin SHARED test/plugin/test-plugin.c)
  target_link_libraries(test-plugin PRIVATE moonsugar)
  ms_configure_target(test-plugin)
endfunction()
