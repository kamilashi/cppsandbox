file(GLOB children LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")
get_filename_component(CUR_FOLDER_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

add_library(${CUR_FOLDER_NAME} STATIC)                      
set_target_properties(${CUR_FOLDER_NAME} PROPERTIES LINKER_LANGUAGE CXX)  
target_compile_features(${CUR_FOLDER_NAME} PUBLIC cxx_std_20)

foreach(child ${children})
  get_filename_component(name "${child}" NAME)
  if(EXISTS "${child}/include" OR EXISTS "${child}/source")
    add_module(${name} ${CUR_FOLDER_NAME})
    target_link_libraries(${name}
    PUBLIC 
      runtime
    )
  endif()
endforeach()


