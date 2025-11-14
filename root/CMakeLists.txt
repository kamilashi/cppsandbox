cmake_minimum_required(VERSION 3.21)

project(CppSandbox VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(AddApp)

cmake_minimum_required(VERSION 3.21)

add_subdirectory(code/wsanet)
add_subdirectory(code/shared)
add_subdirectory(code/dataflow)
add_subdirectory(code/tests)   

 