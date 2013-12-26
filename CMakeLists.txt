cmake_minimum_required(VERSION 2.6)
project(dust-server)

################################
# Compile flags
################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")


################################
# Dependencies
################################
add_subdirectory(external_lib/dust)
add_subdirectory(external_lib/http_server)
add_subdirectory(external_lib/lua-5.2.0)


################################
# Include Paths
################################
include_directories("include/")
include_directories("external_lib/dust/include")
include_directories("external_lib/http_server")
include_directories("external_lib/luabridge/Source")
include_directories("external_lib/lua-5.2.0/src")

file(GLOB src_files "src/*.cc")
add_library(dust-server STATIC ${src_files})
target_link_libraries(dust-server dust http_server lua ${Boost_LIBRARIES})


################################
# Unit Tests
################################
if (NOT TARGET gtest)
  add_subdirectory(test/gtest-1.7.0)
  enable_testing()
endif()

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(dust-server-tests EXCLUDE_FROM_ALL
  test/script_test.cpp
#  test/server_test.cpp
)

target_link_libraries(dust-server-tests gtest gtest_main dust-server lua)
