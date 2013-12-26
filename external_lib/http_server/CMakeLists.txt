cmake_minimum_required(VERSION 2.6)
project(http_server)

include_directories(".")
file(GLOB HTTP_SERVER_FILES "*.cpp")
add_library(http_server STATIC ${HTTP_SERVER_FILES})