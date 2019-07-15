add_library(SACT MODULE
  SACT.c
  sactcg.c
  sactsound.c
  sactbgm.c
  sacttimer.c
  sactstring.c
  sactcrypto.c
  sactchart.c
  sactamask.c
  sactlog.c
  sprite.c
  sprite_event.c
  sprite_tevent.c
  sprite_move.c
  sprite_anime.c
  sprite_get.c
  sprite_put.c
  sprite_switch.c
  sprite_sel.c
  sprite_msg.c
  sprite_draw.c
  sprite_update.c
  sprite_eupdate.c
  sprite_quake.c
  sprite_keywait.c
  screen_quake.c
  sprite_xmenu.c
  )

target_link_libraries(SACT PRIVATE graph drawtext)

install(TARGETS SACT DESTINATION lib/xsystem35)
