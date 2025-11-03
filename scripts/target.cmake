include_guard()

function(ms_configure_target target)
  set_target_properties(
    ${target}
    PROPERTIES
      C_STANDARD 99
      C_STANDARD_REQUIRED ON
  )
  
  target_compile_options(
    ${target}
    PRIVATE
      "-Wall"
      "-Werror"
      "-Wextra"
      "-pedantic"
  )

  if(WIN32)
    target_compile_definitions(
      ${target}
      PRIVATE
      _CRT_SECURE_NO_WARNINGS
    )
  endif()
endfunction()
