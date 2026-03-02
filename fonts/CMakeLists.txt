install(FILES
  MTLc3m.ttf
  mincho.otf
  DESTINATION share/xsystem35/fonts)

# pyftsubset is included in fonttools.
find_program(PYFTSUBSET_FOUND pyftsubset)

if (PYFTSUBSET_FOUND)
  add_custom_target(fonts
    COMMAND uchars > uchars.txt
    COMMAND wget -O SourceHanSerifJP-Regular.otf https://github.com/adobe-fonts/source-han-serif/blob/release/SubsetOTF/JP/SourceHanSerifJP-Regular.otf?raw=true
    COMMAND pyftsubset SourceHanSerifJP-Regular.otf --text-file=uchars.txt --output-file=subset.otf
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/adjust_metrics.py subset.otf ${CMAKE_CURRENT_SOURCE_DIR}/mincho.otf
    )
endif()
