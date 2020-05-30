install(FILES
  MTLc3m.ttf
  MTLc3m.ttf.license
  mincho.otf
  mincho.otf.license
  DESTINATION share/xsystem35/fonts)

# pyftsubset is included in fonttools.
find_program(PYFTSUBSET_FOUND pyftsubset)

if (PYFTSUBSET_FOUND)
  add_custom_target(fonts
    pyftsubset SourceHanSerifJP-Regular.otf --text-file=uchars.txt --output-file=${CMAKE_CURRENT_SOURCE_DIR}/mincho.otf
    DEPENDS uchars.txt SourceHanSerifJP-Regular.otf)

  add_custom_command(OUTPUT uchars.txt
    COMMAND uchars > uchars.txt)

  add_custom_command(OUTPUT SourceHanSerifJP-Regular.otf
    COMMAND wget -O SourceHanSerifJP-Regular.otf https://github.com/adobe-fonts/source-han-serif/blob/release/SubsetOTF/JP/SourceHanSerifJP-Regular.otf?raw=true)
endif()
