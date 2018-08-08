cmake_minimum_required(VERSION 3.7.0)
project(test_croncpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/catch)

file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES "*.cpp" "include/*.cpp")

add_executable(test_croncpp ${SOURCES} ${headers})
