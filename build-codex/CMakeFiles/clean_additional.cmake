# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles\\appPipeSightConsole_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appPipeSightConsole_autogen.dir\\ParseCache.txt"
  "appPipeSightConsole_autogen"
  )
endif()
