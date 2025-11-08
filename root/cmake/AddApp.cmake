include_guard(GLOBAL)

function(add_app TEST_NAME)
    set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}")

    set(TEST_OUTPUT_BASE "${CMAKE_SOURCE_DIR}/builds/${TEST_NAME}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${TEST_OUTPUT_BASE}/$<CONFIG>")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${TEST_OUTPUT_BASE}/$<CONFIG>")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${TEST_OUTPUT_BASE}/$<CONFIG>")

    file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS "${TEST_DIR}/include/*.h" "${TEST_DIR}/include/*.hpp")
    file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${TEST_DIR}/source/*.c" "${TEST_DIR}/source/*.cpp")

    message(STATUS "${TEST_NAME} header files found: ${APP_HEADERS} ")
    message(STATUS "${TEST_NAME} source files found: ${APP_SOURCES} ")

    add_executable(${TEST_NAME} ${APP_SOURCES} ${APP_HEADERS})

    target_include_directories(${TEST_NAME} PRIVATE ${TEST_DIR}/include)

    target_link_libraries(${TEST_NAME} PRIVATE wsanet)
    target_link_libraries(${TEST_NAME} PRIVATE shared)

    set_target_properties(${TEST_NAME}
        PROPERTIES
            WIN32_EXECUTABLE OFF
            MACOSX_BUNDLE OFF
    )
endfunction()