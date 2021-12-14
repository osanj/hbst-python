cmake_minimum_required(VERSION 3.4)

project(hbst_python)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Werror=return-type")
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set(INCLUDE_ROOT include)

include_directories(${INCLUDE_ROOT}/srrg_hbst)
include_directories(${INCLUDE_ROOT}/eigen)
add_subdirectory(${INCLUDE_ROOT}/pybind11 pybind11)
pybind11_add_module(hbst src/module.cpp)

target_compile_definitions(hbst PRIVATE VERSION_INFO=${VERSION_INFO})
