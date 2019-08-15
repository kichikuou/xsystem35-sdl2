if (ENABLE_MODULES)
  set(CMAKE_BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR}/../modules)
  if (APPLE)
	set(CMAKE_INSTALL_RPATH @loader_path/../lib/xsystem35)
  else()
	set(CMAKE_INSTALL_RPATH $ORIGIN/../lib/xsystem35)
  endif()
endif()

add_executable(xsystem35)

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

# Music
target_sources(xsystem35 PRIVATE
  music.c music_cdrom.c music_midi.c midi.c cdrom.c)

# Misc
target_sources(xsystem35 PRIVATE
  LittleEndian.c counter.c imput.c profile.c mt19937-1.c utfsjis.c
  filecheck.c tempname.c)

# Scenario
target_sources(xsystem35 PRIVATE
  cali.c scenario.c variable.c cmd_check.c hankana2sjis.c nact.c
  selection.c message.c savedata.c hankaku.c s39ain.c)

# Graphics
target_sources(xsystem35 PRIVATE
  ags.c font.c flood.c cg.c ecopy.c antialiase.c alpha_plane.c cursor.c)

# Network
target_sources(xsystem35 PRIVATE
  network.c)

if (ENABLE_MODULES)
  target_link_libraries(xsystem35 PRIVATE -rdynamic)
endif()

if (EMSCRIPTEN)

  set(LIBS "SHELL:-s USE_ZLIB=1 -s USE_FREETYPE=1 -s USE_SDL=2")
  target_compile_options(xsystem35 PRIVATE ${LIBS})
  target_link_options(xsystem35 PRIVATE ${LIBS})

  # Without optimizations, Asyncify generates very large code.
  list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG "-O1")

  target_link_options(xsystem35 PRIVATE
	"SHELL:-s ENVIRONMENT=web"
	"SHELL:-s ASYNCIFY=1 -s ASYNCIFY_IGNORE_INDIRECT=1"
	"SHELL:-s ASYNCIFY_IMPORTS=\"['emscripten_sleep','SDL_Delay','__syscall118','muspcm_load_no','muspcm_load_mixlr','muspcm_waitend','wait_vsync','load_mincho_font']\""
	"SHELL:-s ALLOW_MEMORY_GROWTH=1"
	"SHELL:-s NO_EXIT_RUNTIME=1"
	"SHELL:-s EXPORTED_FUNCTIONS=\"['_main','_ags_setAntialiasedStringMode', '_ald_getdata', '_ald_freedata', '_va_alarm_handler']\""
	"SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=\"['getValue','getMemory','addRunDependency','removeRunDependency']\"")

else()  # non-emscripten
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
  if (SDL2_FOUND)
	target_link_libraries(xsystem35 PRIVATE PkgConfig::SDL2)
  endif()
  if (SDL2MIXER_FOUND)
	target_link_libraries(xsystem35 PRIVATE PkgConfig::SDL2MIXER)
  endif()
  if (SDL2TTF_FOUND)
	target_link_libraries(xsystem35 PRIVATE PkgConfig::SDL2TTF)
  endif()

  install(TARGETS xsystem35 RUNTIME DESTINATION bin)
endif()

# FIXME: set up dependency for xsystem35 on this
if (ENABLE_SDL AND NOT EMSCRIPTEN)
  add_custom_target(sdl_input.c
	COMMAND perl xsyskey.pl ${SDL2_INCLUDE_DIRS} > sdl_input.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
