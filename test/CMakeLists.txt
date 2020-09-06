set(SOURCES main.cpp test_oracle.cpp test_standard.cpp)

add_executable(test_croncpp ${SOURCES})

target_compile_features(test_croncpp PUBLIC ${CXX_STD_VER})
set_target_properties(test_croncpp PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(test_croncpp PRIVATE croncpp croncpp_options)
target_include_directories(test_croncpp PRIVATE ${CMAKE_SOURCE_DIR}/catch)

add_test(NAME "test_croncpp" COMMAND "test_croncpp" "-r compact")
set_tests_properties("test_croncpp" PROPERTIES PASS_REGULAR_EXPRESSION
                                               "Passed all.*")
set_tests_properties("test_croncpp" PROPERTIES FAIL_REGULAR_EXPRESSION
                                               "Failed \\d+ test cases")
set_tests_properties("test_croncpp" PROPERTIES TIMEOUT 120)
