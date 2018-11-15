cmake_minimum_required(VERSION 3.7.0)
project(test_croncpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/catch)

file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES "*.cpp" "include/*.cpp")

add_executable(test_croncpp ${SOURCES} ${headers})

file(GLOB files "test_*.cpp")

enable_testing()

foreach(file ${files})
    string(REGEX REPLACE "(^.*/|\\.[^.]*$)" "" file_without_ext ${file})
    add_executable(${file_without_ext} "main.cpp" ${file} ${headers})
    add_test(NAME ${file_without_ext} COMMAND ${file_without_ext} "-r compact")
    set_tests_properties(${file_without_ext}
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Passed all.*")
    set_tests_properties(${file_without_ext}
        PROPERTIES
        FAIL_REGULAR_EXPRESSION "Failed \\d+ test cases")
    set_tests_properties(${file_without_ext}
        PROPERTIES
        TIMEOUT 120)
endforeach()