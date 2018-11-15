cmake_minimum_required(VERSION 3.7.0)
project(croncpp)

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

message(status "** CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include(CTest)

add_subdirectory(benchmark)
add_subdirectory(test)