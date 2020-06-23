cmake_minimum_required(VERSION 3.8)
project(
  croncpp
  VERSION 0.0.0
  LANGUAGES CXX)

add_library(croncpp INTERFACE)
target_include_directories(
  croncpp INTERFACE $<INSTALL_INTERFACE:include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(croncpp INTERFACE cxx_std_17)
add_library(croncpp::croncpp ALIAS croncpp)

install(
  TARGETS croncpp
  EXPORT croncpp_targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(croncpp.cmake COMPATIBILITY AnyNewerVersion)

install(
  EXPORT croncpp_targets
  FILE croncpp.cmake
  NAMESPACE croncpp::
  DESTINATION lib/cmake/croncpp)

add_library(croncpp_options INTERFACE)
target_compile_features(croncpp_options INTERFACE cxx_std_17)

if(WIN32)
  target_compile_options(croncpp_options INTERFACE "/EHc")
  target_compile_definitions(croncpp_options
                             INTERFACE "-D_SCL_SECURE_NO_WARNINGS")
elseif(APPLE)
  target_compile_options(croncpp_options INTERFACE "-fexceptions -g -Wall")
else()
  target_compile_options(croncpp_options INTERFACE "-fexceptions -g -Wall")
  # enable profiling
  if(CRONCPP_ENABLE_PROFILING)
    target_compile_options(
      croncpp_options
      INTERFACE "--coverage -g -O0 -fprofile-arcs -ftest-coverage")
  endif()
endif()

if(CRONCPP_BUILD_TESTS)
  add_subdirectory(test)
endif()

if(CRONCPP_BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()
