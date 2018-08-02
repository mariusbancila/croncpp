cmake_minimum_required(VERSION 3.7.0)
project(test_croncpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/catch)

file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES "test/*.cpp" "include/*.cpp")

if(WIN32)
	message(status "Setting MSVC flags")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS)
elseif(APPLE)
	message(status "Setting Clang flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -fexceptions -g -Wall")
else()
	include_directories(${LIBUUID_INCLUDE_DIR})
   message(status "Setting GCC flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -fexceptions -g -Wall")
endif()

add_executable(test_croncpp ${SOURCES} ${headers})

message(status "** CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
