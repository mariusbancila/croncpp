set(SOURCES main.cpp)

add_executable(bench_croncpp ${SOURCES})
target_link_libraries(bench_croncpp PRIVATE croncpp croncpp_options)
target_include_directories(bench_croncpp PRIVATE ${CMAKE_SOURCE_DIR}/catch)
