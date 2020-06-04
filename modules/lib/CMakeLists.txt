add_library(graph STATIC
  graph.c
  surface.c
  cg.c
  graph_expandcolor_blend.c
  graph_fillrect.c
  graph_fillrect_amap.c
  graph_fillrect_acolor.c
  graph_rect.c
  graph_copy.c
  graph_copy_amap.c
  graph_copy_bright.c
  graph_copy_whiteout.c
  graph_blend_amap.c
  graph_blend_screen.c
  graph_saturadd_amap.c
  graph_draw_amap.c
  graph_stretch.c
  graph_cg.c
  graph_buller.c
  gre_blend_useamap.c
  gre_blend.c
  gre_blend_screen.c
  )

add_library(alk STATIC alk.c)
target_compile_options(alk PRIVATE -Wno-pointer-sign)

add_library(drawtext STATIC drawtext.c)
target_compile_options(drawtext PRIVATE -Wno-pointer-sign)

add_library(list STATIC list.c)

target_include_directories(graph PUBLIC .)
target_include_directories(alk PUBLIC .)
target_include_directories(drawtext PUBLIC .)
target_include_directories(list PUBLIC .)

add_executable(lib_test list_test.c)
target_link_libraries(lib_test PRIVATE list)
add_test(NAME lib_test COMMAND lib_test)
