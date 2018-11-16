cmake_minimum_required(VERSION 3.7.0)
project(test_croncpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/catch)

file(GLOB headers ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB SOURCES "*.cpp" "include/*.cpp")

add_executable(test_croncpp ${SOURCES} ${headers})

if(BUILD_TESTS)
    enable_testing()

    add_test(NAME "test_croncpp" COMMAND "test_croncpp" "-r compact")
    set_tests_properties("test_croncpp"
        PROPERTIES
        PASS_REGULAR_EXPRESSION "Passed all.*")
    set_tests_properties("test_croncpp"
        PROPERTIES
        FAIL_REGULAR_EXPRESSION "Failed \\d+ test cases")
    set_tests_properties("test_croncpp"
        PROPERTIES
        TIMEOUT 120)
endif()
