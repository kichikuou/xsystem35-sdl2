add_library(Gpx MODULE Gpx.c effectcopy.c)

target_link_libraries(Gpx PRIVATE graph)
target_include_directories(Gpx PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/modules/lib)

install(TARGETS Gpx DESTINATION lib/xsystem35)
