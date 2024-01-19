add_library(src_lib STATIC
  gameresource.c
  hankaku.c
  msgqueue.c
  utfsjis.c
  )
target_compile_options(src_lib PRIVATE -Wno-pointer-sign -Wall)
target_include_directories(src_lib PRIVATE .)

if (ANDROID)
  add_library(xsystem35 SHARED)
else()
  add_executable(xsystem35)
  if (WIN32)
    target_link_options(xsystem35 PRIVATE -static)
  endif()
endif()

target_compile_options(xsystem35 PRIVATE -Wno-pointer-sign $<$<CONFIG:Debug>:-Wall>)
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
  ${SRC_MENU})

# System3.x commands
target_sources(xsystem35 PRIVATE
  cmdb.c cmdc.c cmdd.c cmde.c cmdf.c cmdg.c cmdh.c cmdi.c cmdj.c
  cmdk.c cmdl.c cmdm.c cmdn.c cmdo.c cmdp.c cmdq.c cmds.c cmdt.c
  cmdu.c cmdv.c cmdw.c cmdy.c cmdz.c cmd2F.c cmd2F60.c)

# Graphics

target_sources(xsystem35 PRIVATE
  sdl_video.c sdl_draw.c sdl_event.c sdl_image.c sdl_cursor.c sdl_effect.c
  image.c font.c)

# CG
target_sources(xsystem35 PRIVATE
  pms.c vsp.c bmp.c qnt.c jpeg.c)
if (HAVE_WEBP)
  target_sources(xsystem35 PRIVATE webp.c)
  target_link_libraries(xsystem35 PRIVATE WebP)
endif()

# Music
target_sources(xsystem35 PRIVATE
  music.c music_cdrom.c music_midi.c midi.c cdrom.c)

# Misc
target_sources(xsystem35 PRIVATE
  input.c profile.c mt19937-1.c filecheck.c mmap.c hacks.c)

# Scenario
target_sources(xsystem35 PRIVATE
  cali.c scenario.c variable.c cmd_check.c nact.c
  selection.c message.c savedata.c s39ain.c texthook.c msgskip.c)

# Graphics
target_sources(xsystem35 PRIVATE
  ags.c cg.c ecopy.c alpha_plane.c cursor.c)

# Network
target_sources(xsystem35 PRIVATE
  network.c)

# Debugger
if (ENABLE_DEBUGGER)
  target_sources(xsystem35 PRIVATE
    debug_symbol.c debugger.c debugger_cui.c debugger_dap.c)
endif()

if (WIN32)
  target_sources(xsystem35 PRIVATE win/dialog.c win/resources.rc win/menubar.c win/console.c)
endif()

target_link_libraries(src_lib PRIVATE sdl2 zlib)
target_link_libraries(xsystem35 PRIVATE m zlib sdl2 sdl2_ttf)
if (TARGET sdl2_mixer)
  target_link_libraries(xsystem35 PRIVATE sdl2_mixer)
endif()

if (EMSCRIPTEN)
  target_link_libraries(xsystem35 PRIVATE idbfs.js)

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  target_link_options(xsystem35 PRIVATE
    "SHELL:-s ENVIRONMENT=web"
    "SHELL:-s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1"
    "SHELL:-s ASYNCIFY_REMOVE=SDL_Delay"
    "SHELL:-s ASYNCIFY_IMPORTS=muspcm_load_no,muspcm_load_data,muspcm_load_mixlr,muspcm_waitend,wait_vsync,load_mincho_font"
    "SHELL:-s ASYNCIFY_ADD=commands2F60,nact_main,send_agsevent,cb_waitkey_sprite"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s NO_EXIT_RUNTIME=1"
    "SHELL:-s EXPORTED_FUNCTIONS=_main,_malloc"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=getValue,addRunDependency,removeRunDependency")

elseif (ANDROID)
  target_link_libraries(xsystem35 PRIVATE ${ndk_log})

else()  # non-emscripten, non-android
  if (TARGET cJSON)
    target_link_libraries(xsystem35 PRIVATE cJSON)
  endif()
  if (GTK3_FOUND)
    target_link_libraries(xsystem35 PRIVATE PkgConfig::GTK3)
  endif()
  if (ENABLE_MIDI_PORTMIDI)
    target_link_libraries(xsystem35 PRIVATE ${PORTMIDI})
  endif()

  install(TARGETS xsystem35 RUNTIME DESTINATION bin)

  add_executable(src_tests
    src_tests.c
    gameresource_test.c
    hankaku_test.c
    )
  target_compile_options(src_tests PRIVATE -Wno-pointer-sign -Wall)
  target_link_libraries(src_tests PRIVATE src_lib)
  add_test(NAME src_tests COMMAND src_tests)
  configure_file(testdata/test.gr ${CMAKE_CURRENT_BINARY_DIR}/testdata/test.gr COPYONLY)
endif()
