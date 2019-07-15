set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if (APPLE)
  add_link_options("SHELL:-undefined dynamic_lookup")
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(lib)
add_subdirectory(dDemo)
add_subdirectory(Gpx)
add_subdirectory(Math)
add_subdirectory(MsgSkip)
add_subdirectory(ShArray)
add_subdirectory(ShCalc)
add_subdirectory(ShGraph)
add_subdirectory(ShPort)
# add_subdirectory(ShSound)
add_subdirectory(ShString)
