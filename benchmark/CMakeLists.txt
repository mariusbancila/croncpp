set(SOURCES main.cpp)

add_executable(bench_croncpp ${SOURCES})

target_compile_features(bench_croncpp PUBLIC ${CXX_STD_VER})
set_target_properties(bench_croncpp PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(bench_croncpp PRIVATE croncpp croncpp_options)
target_include_directories(bench_croncpp PRIVATE ${CMAKE_SOURCE_DIR}/catch)
