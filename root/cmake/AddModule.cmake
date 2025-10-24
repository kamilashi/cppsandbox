include_guard(GLOBAL)

function(add_module MODULE_NAME UMBRELLA_TARGET)
  set(MOD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${MODULE_NAME}")
  set(INC_DIR "${MOD_DIR}/include")
  set(SRC_DIR "${MOD_DIR}/source")

  file(GLOB_RECURSE MOD_HEADERS CONFIGURE_DEPENDS "${INC_DIR}/*.h" "${INC_DIR}/*.hpp")
  file(GLOB_RECURSE MOD_SOURCES CONFIGURE_DEPENDS "${SRC_DIR}/*.c" "${SRC_DIR}/*.cpp")

  message(STATUS "${MODULE_NAME} header files found in ${INC_DIR} : ${MOD_HEADERS} ")
  message(STATUS "${MODULE_NAME} source files found in ${SRC_DIR} : ${MOD_SOURCES} ")

  if(MOD_SOURCES STREQUAL "" AND MOD_HEADERS STREQUAL "")
    message(STATUS "Skipping empty module '${MODULE_NAME}' in ${MOD_DIR}")
    return()
  endif()

  if(MOD_SOURCES STREQUAL "")
    add_library(${MODULE_NAME} INTERFACE)
    target_include_directories(${MODULE_NAME} INTERFACE "${INC_DIR}")
    target_compile_features(${MODULE_NAME} INTERFACE cxx_std_20)

    # Make headers visible and exportable via the umbrella
    if(MOD_HEADERS)
      if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.23")
        target_sources(${UMBRELLA_TARGET}
          INTERFACE
            FILE_SET HEADERS
            BASE_DIRS "${INC_DIR}"
            FILES ${MOD_HEADERS}
        )
      endif()
    endif()
  else()
  
    add_library("${MODULE_NAME}" OBJECT ${MOD_SOURCES} ${MOD_HEADERS})
    target_include_directories("${MODULE_NAME}" PUBLIC "${INC_DIR}")
    target_compile_features("${MODULE_NAME}" PUBLIC cxx_std_20)

    target_sources(${UMBRELLA_TARGET} PRIVATE $<TARGET_OBJECTS:${MODULE_NAME}>)
  endif()
  
  target_link_libraries(${UMBRELLA_TARGET} PUBLIC ${MODULE_NAME})
  target_include_directories("${UMBRELLA_TARGET}" PUBLIC "${INC_DIR}")
endfunction()