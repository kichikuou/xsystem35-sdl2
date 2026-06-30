# A normal build does not re-extract strings, so after adding/changing a
# _()/N_() string build the `pot` target and translate the new ja.po entries.
# CI fails if xsystem35.pot is out of date.

# Compile ja.po into the runtime catalog.
gettext_create_translations(xsystem35.pot ALL ja.po)

# The `pot` target re-extracts strings from POTFILES.in into xsystem35.pot.
add_custom_target(pot
  xgettext --default-domain=${PACKAGE} --directory=${CMAKE_SOURCE_DIR}
           --keyword=_ --keyword=N_
           --files-from=${CMAKE_CURRENT_SOURCE_DIR}/POTFILES.in
           --output=${CMAKE_CURRENT_SOURCE_DIR}/xsystem35.pot
  )
