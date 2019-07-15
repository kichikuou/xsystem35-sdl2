add_library(dDemo MODULE dDemo.c)

target_link_libraries(dDemo PRIVATE graph alk)

install(TARGETS dDemo DESTINATION lib/xsystem35)
