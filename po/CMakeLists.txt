# This creates a target "translations"
gettext_create_translations(xsystem35.pot ALL ja.po)

add_custom_target(pot
  xgettext --default-domain=${PACKAGE} --directory=${CMAKE_SOURCE_DIR}
           --keyword=_ --keyword=N_
           --files-from=${CMAKE_CURRENT_SOURCE_DIR}/POTFILES.in
           --copyright-holder='Masaki Chikama'
           --output=${CMAKE_CURRENT_SOURCE_DIR}/xsystem35.pot
  )
add_dependencies(translations pot)
