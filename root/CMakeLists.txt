list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(AddModule)

cmake_minimum_required(VERSION 3.21)

project(CppSandbox VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CPP_SANDBOX_OUTPUT_BASE "${CMAKE_SOURCE_DIR}/builds")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CPP_SANDBOX_OUTPUT_BASE}/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CPP_SANDBOX_OUTPUT_BASE}/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CPP_SANDBOX_OUTPUT_BASE}/$<CONFIG>")

if(MSVC)
  add_compile_options(
    /W4           # high warnings
    /permissive-  # stricter standard conformance
    /Zc:__cplusplus  # make __cplusplus report the correct value
    /EHsc         # standard C++ exceptions model
    /external:W0  # do not spam warnings from external headers
  )

  # treat warnings as errors:
  # add_compile_options(/WX)
endif()

add_subdirectory(code/local/runtime)  
add_subdirectory(code/local/tests)
add_subdirectory(code/app)           


 