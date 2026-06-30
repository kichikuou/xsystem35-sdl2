# Generate a C header with the built-in translation catalogs, for the NLS used
# on platforms without libintl. It defines one table per language plus a
# registry (nls_catalogs) mapping language codes to tables, all as file-local
# statics, and is included by src/nls.c.
#
# .po and C share the same string-escape conventions (\n, \", \\, ...), so the
# quoted bodies are copied through verbatim and multi-line entries become
# adjacent C string literals, which the compiler concatenates.
#
# Expected variables: PO_DIR, LANGUAGES (list), OUTPUT

set(tables "")
set(registry "")

foreach(lang IN LISTS LANGUAGES)
  file(STRINGS "${PO_DIR}/${lang}.po" lines ENCODING UTF-8)

  set(entries "")
  set(cur_id "")
  set(cur_str "")
  set(state none)

  # Appends the current msgid/msgstr pair to `entries`, skipping the header
  # entry (empty msgid).
  macro(flush_entry)
    if(NOT cur_id STREQUAL "" AND NOT cur_id STREQUAL "\"\"")
      string(APPEND entries "\t{ ${cur_id}, ${cur_str} },\n")
    endif()
  endmacro()

  foreach(line IN LISTS lines)
    if(line MATCHES "^msgid \"(.*)\"$")
      flush_entry()
      set(cur_id "\"${CMAKE_MATCH_1}\"")
      set(cur_str "")
      set(state id)
    elseif(line MATCHES "^msgstr \"(.*)\"$")
      set(cur_str "\"${CMAKE_MATCH_1}\"")
      set(state str)
    elseif(line MATCHES "^\"(.*)\"$")
      if(state STREQUAL id)
        set(cur_id "${cur_id} \"${CMAKE_MATCH_1}\"")
      elseif(state STREQUAL str)
        set(cur_str "${cur_str} \"${CMAKE_MATCH_1}\"")
      endif()
    else()
      set(state none)
    endif()
  endforeach()
  flush_entry()

  string(APPEND tables
    "static const struct nls_entry nls_table_${lang}[] = {\n"
    "${entries}"
    "\t{ 0, 0 },  // terminator\n"
    "};\n\n")
  string(APPEND registry "\t{ \"${lang}\", nls_table_${lang} },\n")
endforeach()

file(WRITE "${OUTPUT}"
  "// Generated from the .po files. Do not edit.\n"
  "#include \"nls.h\"\n"
  "\n"
  "${tables}"
  "static const struct nls_catalog nls_catalogs[] = {\n"
  "${registry}"
  "\t{ 0, 0 },  // terminator\n"
  "};\n")
