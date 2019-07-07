set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if (APPLE)
  add_link_options("SHELL:-undefined dynamic_lookup")
endif()

add_subdirectory(lib)
add_subdirectory(Gpx)
add_subdirectory(Math)
