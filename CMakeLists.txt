cmake_minimum_required(VERSION 3.5.0)

project(Backtester CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python_modules)
set(CMAKE_CXX_FLAGS_DEBUG "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(LD_LIBRARY_PATH /usr/local/lib)

include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  # Specify the version/tag/branch you want to use:
 # GIT_TAG v2.13.6
)

FetchContent_MakeAvailable(pybind11)

#add_executable(Strategy_SMA src)
#target_link_libraries(Strategy_SMA PUBLIC ssl crypto gtest boost_url cpr simdjson websocketpp)
#set_property(TARGET Strategy_SMA PROPERTY CXX_STANDARD 20)

find_package(GTest REQUIRED)
#add_executable(Tests tests/tests.cpp)
#target_link_libraries(Tests PUBLIC gtest)
#set_property(TARGET Tests PROPERTY CXX_STANDARD 20)

add_executable(market_making src/examples/market_making.cpp)
set_property(TARGET market_making PROPERTY CXX_STANDARD 20)

pybind11_add_module(python_simulator src/convenience/python_simulator.cpp)