install(FILES
  MTLc3m.ttf
  mincho.ttf
  DESTINATION share/xsystem35/fonts)

# pyftsubset is included in fonttools.
find_program(PYFTSUBSET_FOUND pyftsubset)

if (PYFTSUBSET_FOUND)
  add_custom_target(fonts
    COMMAND uchars > uchars.txt
    COMMAND wget -O ipam.zip https://moji.or.jp/wp-content/ipafont/IPAfont/ipam00303.zip
    COMMAND unzip -o -j ipam.zip ipam00303/ipam.ttf
    COMMAND pyftsubset ipam.ttf --text-file=uchars.txt --output-file=subset.ttf
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/rename_font.py subset.ttf ${CMAKE_CURRENT_SOURCE_DIR}/mincho.ttf
    )
endif()
