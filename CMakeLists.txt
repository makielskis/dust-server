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


################################
# Boost
################################
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS
  python
REQUIRED)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR} "/usr/include/python2.7")
else(Boost_FOUND)
	message("No Boost found.")
endif(Boost_FOUND)
link_directories(${CMAKE_LIBRARY_PATH})


################################
# Include Paths
################################
include_directories("include/")
include_directories("external_lib/dust/include")

file(GLOB src_files "src/*.cc")
add_library(dust-server STATIC ${src_files})
target_link_libraries(dust-server dust ${Boost_LIBRARIES})


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
)

target_link_libraries(dust-server-tests gtest gtest_main dust dust-server python2.7)
