add_library(NIGHTDLL MODULE
  NIGHTDLL.c
  nt_scenario.c
  sactcg.c
  sactstring.c
  sprite.c
  sprite_draw.c
  sprite_update.c
  sprite_eupdate.c
  nt_event.c
  nt_msg.c
  nt_sel.c
  nt_graph.c
  nt_sound.c
  )

target_link_libraries(NIGHTDLL PRIVATE graph drawtext list)

install(TARGETS NIGHTDLL DESTINATION lib/xsystem35)
