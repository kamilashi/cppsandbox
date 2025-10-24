file(GLOB children LIST_DIRECTORIES true "*")

foreach(child ${children})
  if(IS_DIRECTORY "${child}")
    get_filename_component(name "${child}" NAME)
    if(EXISTS "${child}/include" OR EXISTS "${child}/source")
      add_app(${name})
    endif()
  endif()
endforeach()


