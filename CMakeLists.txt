cmake_minimum_required(VERSION 3.10)
project(dust-server)

include(cmake/pkg.cmake)


################################
# Static Dust Server Library
################################
file(GLOB src_files "src/*.cc")
add_library(dust-server STATIC ${src_files})
target_link_libraries(dust-server dust http_server lua luabridge)
target_include_directories(dust-server PUBLIC include)

if (MSVC)
  set(_def "_VARIADIC_MAX=10 _WIN32_WINNT=0x0501")
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
add_executable(dust-server-tests EXCLUDE_FROM_ALL
  test/script_test.cpp
  test/server_test.cpp
)
target_link_libraries(dust-server-tests gtest gtest_main dust-server lua)
set_target_properties(dust-server-tests PROPERTIES COMPILE_FLAGS "-std=c++11")