add_library(src_lib STATIC
  gameresource.c
  hankaku.c
  utfsjis.c
  )
target_compile_options(src_lib PRIVATE -Wno-pointer-sign)
target_include_directories(src_lib PRIVATE .)

if (ANDROID)
  add_library(xsystem35 SHARED)
else()
  add_executable(xsystem35)
endif()

target_compile_options(xsystem35 PRIVATE -Wno-pointer-sign)
target_include_directories(xsystem35 PRIVATE .)
target_link_libraries(xsystem35 PRIVATE src_lib)

target_sources(xsystem35 PRIVATE
  xsystem35.c
  dri.c
  ald_manager.c
  cache.c
  ${SRC_AUDIO}
  ${SRC_CDROM}
  ${SRC_MIDI}
  ${SRC_GRAPHDEV}
  ${SRC_FONT}
  ${SRC_MENU}
  ${SRC_JOYSTICK})

# System3.x commands
target_sources(xsystem35 PRIVATE
  cmdb.c cmdc.c cmdd.c cmde.c cmdf.c cmdg.c cmdh.c cmdi.c cmdj.c
  cmdk.c cmdl.c cmdm.c cmdn.c cmdo.c cmdp.c cmdq.c cmds.c cmdt.c
  cmdu.c cmdv.c cmdw.c cmdy.c cmdz.c cmd2F.c cmd2F60.c)

# CG
target_sources(xsystem35 PRIVATE
  pms.c vsp.c bmp.c qnt.c)
if (HAVE_JPEG)
  target_sources(xsystem35 PRIVATE jpeg.c)
  target_link_libraries(xsystem35 PRIVATE JPEG::JPEG)
endif()

# Music
target_sources(xsystem35 PRIVATE
  music.c music_cdrom.c music_midi.c midi.c cdrom.c)

# Misc
target_sources(xsystem35 PRIVATE
  LittleEndian.c counter.c input.c profile.c mt19937-1.c filecheck.c)

# Scenario
target_sources(xsystem35 PRIVATE
  cali.c scenario.c variable.c cmd_check.c nact.c
  selection.c message.c savedata.c s39ain.c texthook.c)

# Graphics
target_sources(xsystem35 PRIVATE
  ags.c font.c flood.c cg.c ecopy.c antialiase.c alpha_plane.c cursor.c)

# Network
target_sources(xsystem35 PRIVATE
  network.c)

if (WIN32)
  target_sources(xsystem35 PRIVATE win/dialog.c win/resources.rc)
endif()

if (EMSCRIPTEN)

  set(LIBS "SHELL:-s USE_ZLIB=1 -s USE_FREETYPE=1 -s USE_SDL=2")
  target_compile_options(xsystem35 PRIVATE ${LIBS})
  target_link_options(xsystem35 PRIVATE ${LIBS})
  target_link_libraries(xsystem35 PRIVATE idbfs.js)

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  target_link_options(xsystem35 PRIVATE
    "SHELL:-s ENVIRONMENT=web"
    "SHELL:-s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1"
    "SHELL:-s ASYNCIFY_IMPORTS=\"['muspcm_load_no','muspcm_load_mixlr','muspcm_waitend','wait_vsync','load_mincho_font']\""
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s NO_EXIT_RUNTIME=1"
    "SHELL:-s EXPORTED_FUNCTIONS=\"['_main','_ags_setAntialiasedStringMode', '_ald_getdata', '_ald_freedata']\""
    "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=\"['getValue','getMemory','addRunDependency','removeRunDependency']\"")

elseif (ANDROID)

  target_link_libraries(xsystem35 PRIVATE SDL2 SDL2_ttf SDL2_mixer ${ndk_zlib} ${ndk_log})

else()  # non-emscripten, non-android
  target_link_libraries(xsystem35 PRIVATE m ZLIB::ZLIB)
  if (X11_FOUND)
    target_include_directories(xsystem35 PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(xsystem35 PRIVATE ${X11_LIBRARIES})
    if (X11_xf86vmode_FOUND)
      target_link_libraries(xsystem35 PRIVATE ${X11_Xxf86vm_LIB})
    endif()
  endif()
  if (GTK2_FOUND)
    target_link_libraries(xsystem35 PRIVATE GTK2::gtk)
  endif()
  if (FREETYPE_FOUND)
    target_link_libraries(xsystem35 PRIVATE Freetype::Freetype)
  endif()
  if (ENABLE_SDL)
    target_link_libraries(xsystem35 PRIVATE PkgConfig::SDL2)
  endif()
  if (SDL2MIXER_FOUND)
    target_link_libraries(xsystem35 PRIVATE PkgConfig::SDL2MIXER)
  endif()
  if (SDL2TTF_FOUND)
    target_link_libraries(xsystem35 PRIVATE PkgConfig::SDL2TTF)
  endif()

  install(TARGETS xsystem35 RUNTIME DESTINATION bin)

  add_executable(src_tests
    src_tests.c
    gameresource_test.c
    hankaku_test.c
    )
  target_compile_options(src_tests PRIVATE -Wno-pointer-sign)
  target_link_libraries(src_tests PRIVATE src_lib)
  add_test(NAME src_tests COMMAND src_tests)
  configure_file(testdata/test.gr ${CMAKE_CURRENT_BINARY_DIR}/testdata/test.gr COPYONLY)
endif()

# FIXME: set up dependency for xsystem35 on this
if (ENABLE_SDL AND NOT EMSCRIPTEN AND NOT ANDROID)
  add_custom_target(sdl_input.c
    COMMAND perl xsyskey.pl ${SDL2_INCLUDE_DIRS} > sdl_input.c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
