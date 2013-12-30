cmake_minimum_required(VERSION 2.6)
project(http_server)

include_directories(".")
file(GLOB HTTP_SERVER_FILES "*.cpp")
add_library(http_server STATIC ${HTTP_SERVER_FILES})

if(MINGW)
  set(_def
      _WIN32_WINNT=0x0501
      WINVER=0x0501
      _WIN32_IE=0x0501
     )
  set_target_properties(http_server PROPERTIES COMPILE_FLAGS "-s")
  set_target_properties(http_server PROPERTIES COMPILE_DEFINITIONS ${_def})
endif()
