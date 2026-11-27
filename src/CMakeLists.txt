# src_lib contains sources that can be unit-tested without linking the
# rest of xsystem35. They may reference sys_* and other small hooks;
# tests provide stub definitions for those.
add_library(src_lib STATIC
  ald_manager.c
  alpha_plane.c
  audio_meta.c
  bmp.c
  cache.c
  cali.c
  dri.c
  filecheck.c
  font.c
  gameresource.c
  hankaku.c
  mmap.c
  msgqueue.c
  pms.c
  profile.c
  qnt.c
  scenario.c
  sdl_compat.c
  utfsjis.c
  variable.c
  vsp.c
  )
target_compile_options(src_lib PRIVATE -Wno-pointer-sign -Wall)
target_include_directories(src_lib PRIVATE .)

if (ALICEHUB_IOS)
  add_library(xsystem35 STATIC)
elseif (ANDROID)
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
  ecopy.c
  effect.c
  event.c
  hacks.c
  gfx_draw.c
  gfx_image.c
  gfx_video.c
  image.c
  input.c
  input_modal.c
  jpeg.c
  menu.c
  message.c
  microui/microui.c
  midi.c
  modal.c
  msgskip.c
  mt19937-1.c
  music.c
  music_cdrom.c
  music_midi.c
  nact.c
  network.c
  s39ain.c
  savedata.c
  scheduler.c
  selection.c
  system.c
  texthook.c
  volume.c
  xsystem35.c
  )

if (EMSCRIPTEN)
  target_sources(xsystem35 PRIVATE pcm.emscripten.c bgm.emscripten.c)
else()
  target_sources(xsystem35 PRIVATE
    pcm.sdlmixer.c bgm.sdlmixer.c mixer.sdl${XSYSTEM35_SDL_VERSION}mixer.c)
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
elseif (ALICEHUB_IOS OR NINTENDO_SWITCH)
  target_sources(xsystem35 PRIVATE midi.tsf.c)
else()
  target_sources(xsystem35 PRIVATE midi.sdlmixer.c)
  if (ENABLE_MIDI_PORTMIDI)
    target_sources(xsystem35 PRIVATE midi.portmidi.c midifile.c)
  endif()
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
  target_sources(src_lib PRIVATE win/resources.c)
  target_sources(xsystem35 PRIVATE
    win/dialog.c win/resources.rc win/menubar.c win/console.c)
endif()

if (NOT HAVE_LIBINTL)
  target_sources(xsystem35 PRIVATE nls.c)
endif()

target_link_libraries(src_lib PRIVATE sdl freetype2 zlib)
target_link_libraries(xsystem35 PRIVATE m zlib sdl)
if (TARGET sdl_mixer)
  target_link_libraries(xsystem35 PRIVATE sdl_mixer)
endif()

if (EMSCRIPTEN)
  target_link_libraries(xsystem35 PRIVATE idbfs.js)

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  if (JSPI)
    target_link_options(xsystem35 PRIVATE
      -sJSPI
      -sJSPI_IMPORTS=wait_vsync
      )
  else()
    target_link_options(xsystem35 PRIVATE
      -sASYNCIFY=1
      -sASYNCIFY_IGNORE_INDIRECT=1
      -sASYNCIFY_REMOVE=SDL_Delay
      -sASYNCIFY_IMPORTS=wait_vsync
      # Functions that call an asynchronous function via a function pointer must
      # be added to ASYNCIFY_ADD, even if it had a direct call to another
      # asynchronous function, so that the indirect call itself is instrumented.
      -sASYNCIFY_ADD=commands2F60,nact_main,send_agsevent,cb_waitkey_sprite,modal_run
      )
  endif()

  target_link_options(xsystem35 PRIVATE
    -sENVIRONMENT=web
    -sMODULARIZE=1
    -sEXPORT_ES6=1
    -sALLOW_MEMORY_GROWTH=1
    -sNO_EXIT_RUNTIME=1
    -sEXPORTED_FUNCTIONS=_main,_malloc
    -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,HEAPU8,HEAP16,getValue,addRunDependency,removeRunDependency,stringToUTF8OnStack
    --emit-tsd=xsystem35.d.ts
    )

elseif (ANDROID)
  target_sources(xsystem35 PRIVATE alicehub_bridge.c escalayer_modifier.c cJSON.c)
  target_link_libraries(xsystem35 PRIVATE ${ndk_log})
  target_link_options(xsystem35 PRIVATE "LINKER:-z,max-page-size=16384")
  install(TARGETS xsystem35 LIBRARY DESTINATION lib)

elseif (ALICEHUB_IOS)
  target_sources(xsystem35 PRIVATE alicehub_bridge.c escalayer_modifier.c cJSON.c)
  install(TARGETS xsystem35 src_lib ARCHIVE DESTINATION lib)
  install(FILES xsystem35_main.h DESTINATION include)

elseif (NINTENDO_SWITCH)
  include(../../cmake/switch/Packaging.cmake)
  target_sources(xsystem35 PRIVATE escalayer_modifier.c cJSON.c
    ../../cmake/switch/hub/runtime_menu_system35.c)
  target_include_directories(xsystem35 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  target_link_libraries(xsystem35 PRIVATE freetype2)
  set(romfs "${CMAKE_CURRENT_BINARY_DIR}/romfs")
  file(COPY "${PROJECT_SOURCE_DIR}/fonts" "${PROJECT_SOURCE_DIR}/soundfonts" DESTINATION "${romfs}")
  target_compile_definitions(xsystem35 PRIVATE ALICEHUB_SWITCH_XSYSTEM35 SDL_MAIN_HANDLED)
  alicehub_switch_package(xsystem35 "${romfs}")

else()  # non-emscripten, non-android
  if (TARGET cJSON)
    target_link_libraries(xsystem35 PRIVATE cJSON)
  endif()
  if (ENABLE_MIDI_PORTMIDI)
    target_link_libraries(xsystem35 PRIVATE ${PORTMIDI})
  endif()

  install(TARGETS xsystem35 RUNTIME DESTINATION bin)

  add_executable(src_tests
    src_tests.c
    cache_test.c
    font_test.c
    gameresource_test.c
    hankaku_test.c
    qnt_test.c
    variable_test.c
    )
  target_compile_options(src_tests PRIVATE -Wno-pointer-sign -Wall)
  target_compile_definitions(src_tests PRIVATE
      TEST_FONT_DIR="${PROJECT_SOURCE_DIR}/fonts")
  target_link_libraries(src_tests PRIVATE src_lib sdl freetype2)
  add_test(NAME src_tests COMMAND src_tests)

  # Exercise message cursor/capture state without a window or game assets.
  add_executable(message_tests message_test.c message.c)
  target_compile_options(message_tests PRIVATE -Wno-pointer-sign -Wall)
  target_link_libraries(message_tests PRIVATE sdl)
  add_test(NAME message_tests COMMAND message_tests)

  add_executable(effect_tests effect_test.c effect.c)
  target_compile_options(effect_tests PRIVATE -Wno-pointer-sign -Wall)
  target_link_libraries(effect_tests PRIVATE src_lib sdl m)
  add_test(NAME effect_tests COMMAND effect_tests)
  set_tests_properties(effect_tests PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy"
    TIMEOUT 10)

  # Exercise BGM-backed CD fade/volume handling with deterministic ticks.
  add_executable(music_cdrom_tests
    music_cdrom_test.c
    music.c
    music_cdrom.c
    cdrom.bgm.c
    bgm.sdlmixer.c
    )
  target_compile_options(music_cdrom_tests PRIVATE -Wno-pointer-sign -Wall)
  target_include_directories(music_cdrom_tests PRIVATE . ${SDL_INCLUDE_DIRS})
  target_link_libraries(music_cdrom_tests PRIVATE m)
  add_test(NAME music_cdrom_tests COMMAND music_cdrom_tests)
  file(COPY testdata DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
