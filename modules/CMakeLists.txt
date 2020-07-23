include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_subdirectory(lib)

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
  nDEMO/nDEMO.c
  nDEMOE/nDEMOE.c
  oDEMO/oDEMO.c
  oujimisc/oujimisc.c
  tDemo/tDemo.c
  )

target_include_directories(modules PUBLIC .)

target_compile_options(modules PRIVATE -Wno-pointer-sign)
target_link_libraries(modules PRIVATE graph alk drawtext list)

if (SDL2MIXER_FOUND)
  if (ANDROID)
    target_link_libraries(modules PRIVATE SDL2 SDL2_mixer)
  else()
    target_link_libraries(modules PRIVATE PkgConfig::SDL2MIXER)
  endif()
endif()

if (HAVE_JPEG)
  target_sources(modules PRIVATE
    NightDemonDemo/ndd.c
    NightDemonDemo/jpeg2surface.c
    )
  target_link_libraries(modules PRIVATE JPEG::JPEG)
endif()
