cmake_minimum_required(VERSION 3.5.0)

project(Backtester CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python_modules)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g")

set(LD_LIBRARY_PATH /usr/local/lib)

include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  # Specify the version/tag/branch you want to use:
 # GIT_TAG v2.13.6
)
FetchContent_MakeAvailable(pybind11)

FetchContent_Declare(
  clickhouse
  GIT_REPOSITORY https://github.com/ClickHouse/clickhouse-cpp.git
  GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(clickhouse)

add_executable(PnDQuoter src/examples/pnd_quoter.cpp)
target_link_libraries(PnDQuoter PUBLIC gtest clickhouse-cpp-lib)
set_property(TARGET PnDQuoter PROPERTY CXX_STANDARD 20)

find_package(GTest REQUIRED)
add_executable(Tests tests/tests.cpp)
target_link_libraries(Tests PUBLIC gtest clickhouse-cpp-lib)
set_property(TARGET Tests PROPERTY CXX_STANDARD 20)

#find_library(PAPI_LIBRARY NAMES papi)
#add_executable(market_making src/examples/market_making.cpp)
#target_link_libraries(market_making PUBLIC papi)
#set_property(TARGET market_making PROPERTY CXX_STANDARD 20)



pybind11_add_module(python_simulator src/convenience/python_simulator.cpp)