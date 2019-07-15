include(FindJPEG)

if (JPEG_FOUND)
  add_library(NightDemonDemo MODULE NightDemonDemo.c ndd.c jpeg2surface.c)

  target_link_libraries(NightDemonDemo PRIVATE graph alk JPEG::JPEG)

  install(TARGETS NightDemonDemo DESTINATION lib/xsystem35)
endif()
