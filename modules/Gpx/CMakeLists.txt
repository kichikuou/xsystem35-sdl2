add_library(Gpx MODULE Gpx.c effectcopy.c)

target_link_libraries(Gpx PRIVATE graph)

install(TARGETS Gpx DESTINATION lib/xsystem35)
