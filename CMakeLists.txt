cmake_minimum_required(VERSION 2.6)
project(dust-server)


################################
# Dependencies
################################
add_subdirectory(external_lib/dust)
add_subdirectory(external_lib/http_server)
if (NOT TARGET lua)
  add_subdirectory(external_lib/lua-5.2)
endif()


################################
# Include Paths
################################
include_directories("include/")
include_directories("external_lib/dust/include")
include_directories("external_lib/http_server")
include_directories("external_lib/luabridge/Source")
include_directories("external_lib/lua-5.2/src")


################################
# Static Dust Server Library
################################
file(GLOB src_files "src/*.cc")
add_library(dust-server STATIC ${src_files})
target_link_libraries(dust-server dust http_server lua)

if (MSVC)
  set(_def
    _VARIADIC_MAX=10
    _WIN32_WINNT=0x0501
  )
  set_target_properties(dust-server PROPERTIES COMPILE_DEFINITIONS ${_def})
elseif(MINGW)
  set(_def
    _WIN32_WINNT=0x0501
    WINVER=0x0501
    _WIN32_IE=0x0501
  )
  set_target_properties(dust-server PROPERTIES COMPILE_FLAGS "-std=c++11 -s")
  set_target_properties(dust-server PROPERTIES COMPILE_DEFINITIONS ${_def})
else()
  set_target_properties(dust-server PROPERTIES COMPILE_FLAGS "-Wall -Wextra -std=c++11")
endif()


################################
# Unit Tests
################################
if (NOT MSVC AND NOT MINGW)
  ### gtest
  if (NOT TARGET gtest)
    add_subdirectory(test/gtest-1.7.0)
    enable_testing()
  endif()
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

  ### Test Executable
  add_executable(dust-server-tests EXCLUDE_FROM_ALL
    test/script_test.cpp
    test/server_test.cpp
  )
  target_link_libraries(dust-server-tests gtest gtest_main dust-server lua)
  set_target_properties(dust-server-tests PROPERTIES COMPILE_FLAGS "-std=c++11")
endif()
