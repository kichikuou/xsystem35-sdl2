include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_library(modules STATIC
  modules.c
  AliceLogo/AliceLogo.c
  Confirm/Confirm.c
  Gpx/Gpx.c
  Gpx/effectcopy.c
  Math/Math.c
  MsgSkip/MsgSkip.c
  NIGHTDLL/NIGHTDLL.c
  NIGHTDLL/nt_scenario.c
  NIGHTDLL/sactcg.c
  NIGHTDLL/sactstring.c
  NIGHTDLL/sprite.c
  NIGHTDLL/sprite_draw.c
  NIGHTDLL/sprite_update.c
  NIGHTDLL/sprite_eupdate.c
  NIGHTDLL/nt_event.c
  NIGHTDLL/nt_msg.c
  NIGHTDLL/nt_sel.c
  NIGHTDLL/nt_graph.c
  NIGHTDLL/nt_sound.c
  NightDemonDemo/NightDemonDemo.c
  RandMT/RandMT.c
  SACT/SACT.c
  SACT/sactcg.c
  SACT/sactsound.c
  SACT/sactbgm.c
  SACT/sacttimer.c
  SACT/sactstring.c
  SACT/sactcrypto.c
  SACT/sactchart.c
  SACT/sactamask.c
  SACT/sactlog.c
  SACT/sprite.c
  SACT/sprite_event.c
  SACT/sprite_tevent.c
  SACT/sprite_move.c
  SACT/sprite_anime.c
  SACT/sprite_get.c
  SACT/sprite_put.c
  SACT/sprite_switch.c
  SACT/sprite_sel.c
  SACT/sprite_msg.c
  SACT/sprite_draw.c
  SACT/sprite_update.c
  SACT/sprite_eupdate.c
  SACT/sprite_quake.c
  SACT/sprite_keywait.c
  SACT/screen_quake.c
  SACT/sprite_xmenu.c
  ShArray/ShArray.c
  ShCalc/ShCalc.c
  ShGraph/ShGraph.c
  ShPort/ShPort.c
  ShSound/ShSound.c
  ShString/ShString.c
  dDemo/dDemo.c
  eDemo/eDemo.c
  eeDemo/eeDemo.c
  lib/alk.c
  lib/drawtext.c
  lib/graph.c
  lib/list.c
  lib/surface.c
  lib/cg.c
  lib/graph_expandcolor_blend.c
  lib/graph_fillrect.c
  lib/graph_fillrect_amap.c
  lib/graph_fillrect_acolor.c
  lib/graph_rect.c
  lib/graph_copy.c
  lib/graph_copy_amap.c
  lib/graph_copy_bright.c
  lib/graph_copy_whiteout.c
  lib/graph_blend_amap.c
  lib/graph_blend_screen.c
  lib/graph_saturadd_amap.c
  lib/graph_draw_amap.c
  lib/graph_stretch.c
  lib/graph_cg.c
  lib/graph_buller.c
  lib/gre_blend_useamap.c
  lib/gre_blend.c
  lib/gre_blend_screen.c
  nDEMO/nDEMO.c
  nDEMOE/nDEMOE.c
  oDEMO/oDEMO.c
  oujimisc/oujimisc.c
  tDemo/tDemo.c
  )

target_include_directories(modules PUBLIC . lib)

if (NOT ANDROID AND NOT EMSCRIPTEN)
  add_executable(modules_tests lib/list_test.c)
  target_link_libraries(modules_tests PRIVATE modules)
  add_test(NAME modules_tests COMMAND modules_tests)
endif()

target_compile_options(modules PRIVATE -Wno-pointer-sign)

if (EMSCRIPTEN)
  set(LIBS
    "SHELL:-s USE_ZLIB=1"
    "SHELL:-s USE_LIBJPEG=1"
    "SHELL:-s USE_SDL=2")
  target_compile_options(modules PRIVATE ${LIBS})
  target_link_options(modules PRIVATE ${LIBS})
elseif (ANDROID)
  target_link_libraries(modules PRIVATE ${ndk_zlib})
  if (SDL2MIXER_FOUND)
    target_link_libraries(modules PRIVATE SDL2 SDL2_mixer)
  endif()
else()
  target_link_libraries(modules PRIVATE ZLIB::ZLIB)
  if (SDL2MIXER_FOUND)
    target_link_libraries(modules PRIVATE PkgConfig::SDL2MIXER)
  endif()
endif()

if (HAVE_JPEG)
  target_sources(modules PRIVATE
    NightDemonDemo/ndd.c
    NightDemonDemo/jpeg2surface.c
    )
  if (NOT EMSCRIPTEN)
    target_link_libraries(modules PRIVATE JPEG::JPEG)
  endif()
endif()
