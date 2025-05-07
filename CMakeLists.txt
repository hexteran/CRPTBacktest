cmake_minimum_required(VERSION 3.5.0)

project(Backtester CXX)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build example apps" OFF)
option(BUILD_PYSTRATEGY "Build the Python strategy bridge" OFF)
option(BUILD_ALL "Turn on tests, examples, and pystrategy" OFF)

if (NOT BUILD_TESTS AND 
    NOT BUILD_EXAMPLES AND
    NOT BUILD_PYSTRATEGY AND
    NOT BUILD_ALL)
  set (BUILD_ALL ON)
endif()

if (BUILD_ALL)
  set(BUILD_TESTS ON CACHE BOOL "" FORCE)
  set(BUILD_EXAMPLES ON CACHE BOOL "" FORCE)
  set(BUILD_PYSTRATEGY ON CACHE BOOL "" FORCE)
endif()

message(STATUS "BUILD_TESTS is ${BUILD_TESTS}")
message(STATUS "BUILD_EXAMPLES is ${BUILD_EXAMPLES}")
message(STATUS "BUILD_PYSTRATEGY is ${BUILD_PYSTRATEGY}")
message(STATUS "BUILD_ALL is ${BUILD_ALL}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(LD_LIBRARY_PATH /usr/local/lib)

include(FetchContent)

if (BUILD_EXAMPLES OR BUILD_TESTS)
  FetchContent_Declare(
    clickhouse
    GIT_REPOSITORY https://github.com/ClickHouse/clickhouse-cpp.git
    GIT_SHALLOW TRUE)

  FetchContent_MakeAvailable(clickhouse)
endif()

if (BUILD_EXAMPLES)
  add_executable(PnDQuoter src/examples/pnd_quoter.cpp)
  target_link_libraries(PnDQuoter PUBLIC clickhouse-cpp-lib)

  #find_library(PAPI_LIBRARY NAMES papi)
  #add_executable(market_making src/examples/market_making.cpp)
  #target_link_libraries(market_making PUBLIC papi)
  #set_property(TARGET market_making PROPERTY CXX_STANDARD 20)
endif()

if (BUILD_TESTS)
find_package(GTest REQUIRED)
  add_executable(Tests tests/tests.cpp)
  target_link_libraries(Tests PUBLIC gtest clickhouse-cpp-lib)
endif()

if (BUILD_PYSTRATEGY)
  FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
  )
  FetchContent_MakeAvailable(pybind11)
  pybind11_add_module(python_simulator src/convenience/python_simulator.cpp)
endif()