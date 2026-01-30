cmake_minimum_required(VERSION 3.21)

project(CppSandbox VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(AddApp)

cmake_minimum_required(VERSION 3.21)


# stdexec configuration
set(STDEXEC_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(STDEXEC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(STDEXEC_ENABLE_IO_URING_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(code/external/stdexec EXCLUDE_FROM_ALL)

add_subdirectory(code/wsanet)
add_subdirectory(code/datastructs)
add_subdirectory(code/stdexio)
add_subdirectory(code/shared)
add_subdirectory(code/dataflow)
add_subdirectory(code/tests)   

 