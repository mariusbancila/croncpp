cmake_minimum_required(VERSION 3.7.0)
project(croncpp)

if(WIN32)
   message(status "Setting MSVC flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")
   add_definitions(-D_SCL_SECURE_NO_WARNINGS)
elseif(APPLE)
   message(status "Setting Clang flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fexceptions -g -Wall")
else()
   message(status "Setting GCC flags")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fexceptions -g -Wall")
   # enable profiling
   if(ENABLE_PROFILING)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -g -O0 -fprofile-arcs -ftest-coverage") 
   endif()
endif()

message(status "** CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

if(BUILD_TESTS)
    include(CTest)
endif()

add_subdirectory(benchmark)
add_subdirectory(test)