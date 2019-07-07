add_library(Math MODULE Math.c)

target_include_directories(Math PRIVATE ${CMAKE_SOURCE_DIR}/src)

install(TARGETS Math DESTINATION lib/xsystem35)
