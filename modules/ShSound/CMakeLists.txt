add_library(ShSound MODULE ShSound.c)
if (SDL2MIXER_FOUND)
  target_link_libraries(ShSound PRIVATE PkgConfig::SDL2MIXER)
endif()

install(TARGETS ShSound DESTINATION lib/xsystem35)
