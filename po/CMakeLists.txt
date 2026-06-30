# Translation catalogs, built from the po/<lang>.po files listed in
# NLS_LANGUAGES. On platforms with libintl they are compiled to .mo files and
# loaded at runtime; otherwise they are compiled into the executable as a
# built-in catalog (see src/nls.c).

if (HAVE_LIBINTL)
  # Compile the .po files into .mo catalogs for libintl.
  list(TRANSFORM NLS_LANGUAGES APPEND ".po" OUTPUT_VARIABLE po_files)
  gettext_create_translations(xsystem35.pot ALL ${po_files})

  # The `pot` target re-extracts strings from POTFILES.in into xsystem35.pot.
  # A normal build does not re-extract, so after adding/changing a _()/N_()
  # string build this target and translate the new entries. CI fails if
  # xsystem35.pot is out of date.
  add_custom_target(pot
    xgettext --default-domain=${PACKAGE} --directory=${PROJECT_SOURCE_DIR}
             --keyword=_ --keyword=N_
             --files-from=${CMAKE_CURRENT_SOURCE_DIR}/POTFILES.in
             --output=${CMAKE_CURRENT_SOURCE_DIR}/xsystem35.pot
    )
else()
  # Generate a header with the built-in translation tables (nls_catalog.h),
  # included by src/nls.c. xsystem35 is made to depend on it and to look in
  # this build directory for the header.
  set(po_files "")
  foreach (lang IN LISTS NLS_LANGUAGES)
    list(APPEND po_files ${CMAKE_CURRENT_SOURCE_DIR}/${lang}.po)
  endforeach()
  set(nls_header ${CMAKE_CURRENT_BINARY_DIR}/nls_catalog.h)
  add_custom_command(
    OUTPUT ${nls_header}
    COMMAND ${CMAKE_COMMAND}
            -DPO_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            "-DLANGUAGES=${NLS_LANGUAGES}"
            -DOUTPUT=${nls_header}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_nls_catalog.cmake
    DEPENDS ${po_files} ${CMAKE_CURRENT_SOURCE_DIR}/generate_nls_catalog.cmake
    VERBATIM)
  add_custom_target(generate_nls_catalog DEPENDS ${nls_header})
  add_dependencies(xsystem35 generate_nls_catalog)
  target_include_directories(xsystem35 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()
