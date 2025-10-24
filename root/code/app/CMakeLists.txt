file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

message(STATUS "application header files found: ${APP_HEADERS} ")
message(STATUS "application source files found: ${APP_SOURCES} ")

add_executable(${PROJECT_NAME} ${APP_SOURCES} ${APP_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        runtime
	    tests
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${APP_SOURCES} ${APP_HEADERS})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE OFF
        MACOSX_BUNDLE OFF
)
