get_filename_component(MOD_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(MOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(INC_DIR "${MOD_DIR}/include")
set(SRC_DIR "${MOD_DIR}/source")

file(GLOB_RECURSE MOD_HEADERS CONFIGURE_DEPENDS "${INC_DIR}/*.h" "${INC_DIR}/*.hpp")
file(GLOB_RECURSE MOD_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")

message(STATUS "${MOD_NAME} header files found in ${INC_DIR} : ${MOD_HEADERS} ")
message(STATUS "${MOD_NAME} source files found in ${SRC_DIR} : ${MOD_SOURCES} ")

add_library(${MOD_NAME} STATIC ${MOD_HEADERS} ${MOD_SOURCES})  

target_include_directories(${MOD_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${INC_DIR}>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(${MOD_NAME}
  PUBLIC $<$<PLATFORM_ID:Windows>:ws2_32;mswsock>)

target_compile_features(${MOD_NAME} PUBLIC cxx_std_20)



