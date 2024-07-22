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
  ags.c
  ald_manager.c
  alpha_plane.c
  bgi.c
  bmp.c
  cache.c
  cali.c
  cdrom.bgm.c
  cg.c
  cmd2F.c
  cmd2F60.c
  cmd_check.c
  cmdb.c
  cmdc.c
  cmdd.c
  cmde.c
  cmdf.c
  cmdg.c
  cmdh.c
  cmdi.c
  cmdj.c
  cmdk.c
  cmdl.c
  cmdm.c
  cmdn.c
  cmdo.c
  cmdp.c
  cmdq.c
  cmds.c
  cmdt.c
  cmdu.c
  cmdv.c
  cmdw.c
  cmdy.c
  cmdz.c
  cursor.c
  dri.c
  ecopy.c
  filecheck.c
  font.c
  hacks.c
  image.c
  input.c
  jpeg.c
  message.c
  midi.c
  mmap.c
  msgskip.c
  mt19937-1.c
  music.c
  music_cdrom.c
  music_midi.c
  nact.c
  network.c
  pms.c
  profile.c
  qnt.c
  s39ain.c
  savedata.c
  scenario.c
  sdl_cursor.c
  sdl_draw.c
  sdl_effect.c
  sdl_event.c
  sdl_image.c
  sdl_video.c
  selection.c
  texthook.c
  variable.c
  vsp.c
  xsystem35.c
  )

if (EMSCRIPTEN)
  target_sources(xsystem35 PRIVATE pcm.emscripten.c bgm.emscripten.c)
else()
  target_sources(xsystem35 PRIVATE pcm.sdlmixer.c bgm.sdlmixer.c)
endif()

if (EMSCRIPTEN)
  target_sources(xsystem35 PRIVATE cdrom.emscripten.c)
else()
  target_sources(xsystem35 PRIVATE cdrom.mp3.c)
endif()

if (EMSCRIPTEN)
  target_sources(xsystem35 PRIVATE midi.emscripten.c)
elseif (ANDROID)
  target_sources(xsystem35 PRIVATE midi.android.c)
else()
  target_sources(xsystem35 PRIVATE midi.sdlmixer.c)
  if (ENABLE_MIDI_RAWMIDI)
    target_sources(xsystem35 PRIVATE midi.rawmidi.c midifile.c)
  endif()
  if (ENABLE_MIDI_PORTMIDI)
    target_sources(xsystem35 PRIVATE midi.portmidi.c midifile.c)
  endif()
endif()

if (EMSCRIPTEN)
  target_sources(xsystem35 PRIVATE menu_emscripten.c)
elseif (ANDROID)
  target_sources(xsystem35 PRIVATE menu_android.c)
elseif (ENABLE_GTK)
  target_sources(xsystem35 PRIVATE menu.c menu_callback.c menu_gui.c s39init.c)
else ()
  target_sources(xsystem35 PRIVATE menu_sdl.c sdl_input.c)
endif()

if (HAVE_WEBP)
  target_sources(xsystem35 PRIVATE webp.c)
  target_link_libraries(xsystem35 PRIVATE WebP)
endif()

if (ENABLE_DEBUGGER)
  target_sources(xsystem35 PRIVATE
    debug_symbol.c debugger.c debugger_cui.c debugger_dap.c)
endif()

if (WIN32)
  target_sources(xsystem35 PRIVATE
    win/dialog.c win/resources.rc win/resources.c win/menubar.c win/console.c)
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
    -sENVIRONMENT=web
    -sASYNCIFY=1
    -sASYNCIFY_IGNORE_INDIRECT=1
    -sASYNCIFY_REMOVE=SDL_Delay
    -sASYNCIFY_IMPORTS=muspcm_load_no,muspcm_load_data,muspcm_load_mixlr,muspcm_waitend,wait_vsync,load_mincho_font
    -sASYNCIFY_ADD=commands2F60,nact_main,send_agsevent,cb_waitkey_sprite
    -sALLOW_MEMORY_GROWTH=1
    -sNO_EXIT_RUNTIME=1
    -sEXPORTED_FUNCTIONS=_main,_malloc
    -sEXPORTED_RUNTIME_METHODS=getValue,addRunDependency,removeRunDependency)

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
