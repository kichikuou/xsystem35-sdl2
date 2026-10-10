include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_library(modules STATIC
  modules.c
  AliceLogo/AliceLogo.c
  Confirm/Confirm.c
  Gpx/Gpx.c
  Gpx/cg.c
  Gpx/effectcopy.c
  Gpx/graph_copy_amap.c
  Gpx/graph_fillrect_amap.c
  Gpx/graph.c
  Gpx/surface.c
  Math/Math.c
  MsgSkip/MsgSkip.c
  NightDemonDemo/NightDemonDemo.c
  RandMT/RandMT.c
  S3xMusic/S3xMusic.c
  SACT/SACT.c
  SACT/drawtext.c
  SACT/list.c
  SACT/sactcg.c
  SACT/sactsound.c
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
  lib/graph_blend_amap.c
  nDEMO/nDEMO.c
  nDEMOE/nDEMOE.c
  oDEMO/oDEMO.c
  oujimisc/oujimisc.c
  tDemo/tDemo.c
  )

target_include_directories(modules PUBLIC . lib)

if (NOT ANDROID AND NOT EMSCRIPTEN)
  add_executable(modules_tests SACT/list_test.c)
  target_link_libraries(modules_tests PRIVATE modules)
  add_test(NAME modules_tests COMMAND modules_tests)
endif()

target_compile_options(modules PRIVATE -Wno-pointer-sign)

target_link_libraries(modules PRIVATE sdl2 zlib)
if (TARGET sdl2_mixer)
  target_link_libraries(modules PRIVATE sdl2_mixer)
endif()
