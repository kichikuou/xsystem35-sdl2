set(SRC_SYS35CMD
  cmdb.c cmdc.c cmdd.c cmde.c cmdf.c cmdg.c cmdh.c cmdi.c cmdj.c
  cmdk.c cmdl.c cmdm.c cmdn.c cmdo.c cmdp.c cmdq.c cmds.c cmdt.c
  cmdu.c cmdv.c cmdw.c cmdy.c cmdz.c cmd2F.c cmd2F60.c)
set(SRC_CG
  pms.c vsp.c bmp.c qnt.c)
set(SRC_MUS
  music.c music_cdrom.c music_midi.c midi.c cdrom.c)
set(SRC_MISC
  LittleEndian.c counter.c imput.c profile.c mt19937-1.c utfsjis.c
  filecheck.c tempname.c list.c)
set(SRC_SCO
  cali.c scenario.c variable.c cmd_check.c hankana2sjis.c nact.c
  selection.c message.c savedata.c hankaku.c s39ain.c)
set(SRC_GRAPH
  ags.c font.c flood.c cg.c ecopy.c antialiase.c alpha_plane.c cursor.c)
set(SRC_NET
  network.c)

add_library(objLib OBJECT
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
  ${SRC_JOYSTICK}
  ${SRC_SYS35CMD}
  ${SRC_CG}
  ${SRC_SCO}
  ${SRC_MUS}
  ${SRC_GRAPH}
  ${SRC_MISC}
  ${SRC_NET}
  ${SRC_DLL})

if (ENABLE_MODULES)
  set(CMAKE_BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR}/../modules)
  if (APPLE)
	set(CMAKE_INSTALL_RPATH @loader_path/../lib/xsystem35)
  else()
	set(CMAKE_INSTALL_RPATH $ORIGIN/../lib/xsystem35)
  endif()
endif()

add_executable(xsystem35 $<TARGET_OBJECTS:objLib>)

if (ENABLE_MODULES)
  target_link_libraries(xsystem35 PRIVATE -rdynamic)
endif()

if (EMSCRIPTEN)

  add_executable(xsystem35.asm $<TARGET_OBJECTS:objLib>)

  set(LIBS "SHELL:-s USE_ZLIB=1 -s USE_FREETYPE=1 -s USE_SDL=2")
  target_compile_options(objLib PRIVATE ${LIBS})
  target_link_options(xsystem35 PRIVATE ${LIBS})
  target_link_options(xsystem35.asm PRIVATE ${LIBS})

  set(LINK_OPTS
	"SHELL:-s ENVIRONMENT=web"
	"SHELL:-s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1"
	"SHELL:-s ALLOW_MEMORY_GROWTH=1"
	"SHELL:-s NO_EXIT_RUNTIME=1"
	"SHELL:-s EXPORTED_FUNCTIONS=\"['_main','_ags_setAntialiasedStringMode', '_ald_getdata', '_ald_freedata', '_va_alarm_handler']\""
	"SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=\"['getValue','getMemory','addRunDependency','removeRunDependency']\""
	@${CMAKE_CURRENT_SOURCE_DIR}/../emterpretify_whitelist)

  em_link_js_library(xsystem35 ../emscripten/library.js)
  em_link_js_library(xsystem35.asm ../emscripten/library.js)

  set(SYNCLIST -s EMTERPRETIFY_SYNCLIST="['_muspcm_load_no','_muspcm_load_mixlr','_muspcm_waitend','_wait_vsync','_load_mincho_font']")

  target_link_options(xsystem35 PRIVATE ${LINK_OPTS})
  target_link_options(xsystem35.asm PRIVATE ${LINK_OPTS} "SHELL:-s WASM=0")

  add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../emterpretify_whitelist
	COMMAND ${CMAKE_CXX_COMPILER} -s USE_ZLIB=1 -s USE_FREETYPE=1 -s USE_SDL=2 -s EMTERPRETIFY=1 -s EMTERPRETIFY_ADVISE=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 ${SYNCLIST} $<TARGET_OBJECTS:objLib>
		| grep EMTERPRETIFY_WHITELIST
		> ${CMAKE_CURRENT_SOURCE_DIR}/../emterpretify_whitelist
	COMMAND_EXPAND_LISTS)
  add_custom_target(emterpretify_whitelist DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../emterpretify_whitelist)
  add_dependencies(xsystem35 emterpretify_whitelist)
  add_dependencies(xsystem35.asm emterpretify_whitelist)

else()  # non-emscripten
  target_link_libraries(xsystem35 PRIVATE m)
  install(TARGETS xsystem35 RUNTIME DESTINATION bin)
endif()

# FIXME: set up dependency for xsystem35 on this
if (ENABLE_SDL AND NOT EMSCRIPTEN)
  add_custom_target(sdl_input.c
	COMMAND perl xsyskey.pl ${SDL2_INCLUDE_DIRS} > sdl_input.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
