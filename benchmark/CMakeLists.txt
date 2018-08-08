cmake_minimum_required(VERSION 3.7.0)
project(bench_croncpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES "*.cpp" "include/*.cpp")

add_executable(bench_croncpp ${SOURCES} ${headers})

