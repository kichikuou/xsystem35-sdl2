set(SOURCES
  xsystem35.6.adoc
  xsystem35.gr.5.adoc
)

find_program(ASCIIDOCTOR asciidoctor)
if (NOT ASCIIDOCTOR)
  message(WARNING "asciidoctor not found, manpages will not be generated")
else()
  if (WIN32)
    set(BACKEND html)
  else()
    set(BACKEND manpage)
    include(GNUInstallDirs)
  endif()

  set(MANUALS "")
  foreach(SRC ${SOURCES})
    string(REGEX MATCH "^(.*)\\.([0-9])\\.adoc$" _ ${SRC})
    set(NAME ${CMAKE_MATCH_1})
    set(SECTION ${CMAKE_MATCH_2})
    if (WIN32)
      set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.html)
    else()
      set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.${SECTION})
      install(FILES ${OUTPUT} DESTINATION ${CMAKE_INSTALL_MANDIR}/man${SECTION})
    endif()
    add_custom_command(
      OUTPUT ${OUTPUT}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
      COMMAND ${ASCIIDOCTOR}
        --attribute xsystem35-version=${XSYSTEM35_VERSION}
        --backend ${BACKEND}
        --out-file ${OUTPUT}
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC}
    )
    list(APPEND MANUALS ${OUTPUT})
  endforeach()

  add_custom_target(manual ALL DEPENDS ${MANUALS})
endif()

