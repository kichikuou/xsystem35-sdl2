cmake_minimum_required(VERSION 3.13)

project(xsystem35 LANGUAGES C)
set(CMAKE_C_STANDARD 99)
enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(CheckSymbolExists)

# A wrapper around pkg_check_modules() that skips the check if ENABLE_<prefix> is false
macro (optional_pkg_check_modules prefix)
  option(ENABLE_${prefix} "Use ${prefix} if available" ON)
  if (ENABLE_${prefix})
    pkg_check_modules(${prefix} ${ARGN})
  endif()
endmacro()

# Generates a static library from pkg_check_modules() result
function(add_static_library name pkg)
  add_library(${name} INTERFACE)
  set_target_properties(${name}
      PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${${pkg}_STATIC_INCLUDE_DIRS}"
          INTERFACE_COMPILE_OPTIONS "${${pkg}_STATIC_CFLAGS_OTHER}"
          INTERFACE_LINK_LIBRARIES "${${pkg}_STATIC_LIBRARIES}"
          INTERFACE_LINK_OPTIONS "${${pkg}_STATIC_LDFLAGS_OTHER}")
endfunction()

check_symbol_exists(getlogin "unistd.h" HAVE_GETLOGIN)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(sigaction "signal.h" HAVE_SIGACTION)

if (EMSCRIPTEN)
  function(add_emscripten_library name option)
    add_library(${name} INTERFACE)
    set_target_properties(${name} PROPERTIES
        INTERFACE_COMPILE_OPTIONS ${option}
        INTERFACE_LINK_OPTIONS ${option})
  endfunction()
  add_emscripten_library(zlib "SHELL:-s USE_ZLIB=1")
  add_emscripten_library(sdl2 "SHELL:-s USE_SDL=2")
  add_emscripten_library(sdl2_ttf "SHELL:-s USE_SDL_TTF=2")
  set(DEFAULT_FONT_PATH /fonts/)

elseif (ANDROID)
  add_library(sdl2 ALIAS SDL2)
  add_library(sdl2_ttf ALIAS SDL2_ttf)
  add_library(sdl2_mixer ALIAS SDL2_mixer)
  find_library(ndk_log log)
  find_library(ndk_zlib z)
  add_library(zlib INTERFACE)
  set_target_properties(zlib PROPERTIES INTERFACE_LINK_LIBRARIES ${ndk_zlib})

else()  # non-emscripten, non-android
  if (WIN32)
    set(ZLIB_USE_STATIC_LIBS ON)
  endif()
  find_package(ZLIB REQUIRED)
  add_library(zlib ALIAS ZLIB::ZLIB)

  include(FindPkgConfig)
  optional_pkg_check_modules(GTK3 IMPORTED_TARGET gtk+-3.0)
  if (GTK3_FOUND)
    set(ENABLE_GTK 1)
  endif()

  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
  pkg_check_modules(SDL2TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
  pkg_check_modules(SDL2MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
  if (WIN32)
    add_static_library(sdl2 SDL2)
    add_static_library(sdl2_ttf SDL2TTF)
    # Workaround for linking error
    set_property(TARGET sdl2_ttf PROPERTY INTERFACE_LINK_LIBRARIES
        $<LINK_GROUP:RESCAN,${SDL2TTF_STATIC_LIBRARIES} -lstdc++>)
    add_static_library(sdl2_mixer SDL2MIXER)
  else()
    add_library(sdl2 ALIAS PkgConfig::SDL2)
    add_library(sdl2_ttf ALIAS PkgConfig::SDL2TTF)
    add_library(sdl2_mixer ALIAS PkgConfig::SDL2MIXER)
    set(DEFAULT_FONT_PATH ${CMAKE_INSTALL_PREFIX}/share/xsystem35/fonts/)
  endif()

  option(ENABLE_DEBUGGER "Enable built-in debugger" ON)
  if (ENABLE_DEBUGGER)
    pkg_check_modules(cJSON IMPORTED_TARGET libcjson)
    if (cJSON_FOUND)
      if (WIN32)
        add_static_library(cJSON cJSON)
      else ()
        add_library(cJSON ALIAS PkgConfig::cJSON)
      endif()
    else ()
      # libcjson-dev of Debian buster / Ubuntu 20.04 does not install pkgconfig files.
      find_library(cJSON cjson)
      if (NOT cJSON)
        message(FATAL_ERROR "libcjson is required but not found.")
      endif()
      add_library(cJSON ALIAS ${cJSON})
    endif()
  endif()

  optional_pkg_check_modules(WEBP IMPORTED_TARGET libwebp)
  if (WEBP_FOUND)
    set(HAVE_WEBP 1)
    if (WIN32)
      add_static_library(WebP WEBP)
    else()
      add_library(WebP ALIAS PkgConfig::WEBP)
    endif()
  endif()

  find_library(PORTMIDI portmidi)
  if (PORTMIDI)
    set(HAVE_PORTMIDI 1)
  endif()
endif()

# Menu

if (ENABLE_GTK)
  # i18n support (currently only menus are translated)
  include(FindIntl)
  include(FindGettext)
  if (Intl_FOUND AND GETTEXT_FOUND)
    set(ENABLE_NLS 1)
    add_compile_definitions(LOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale")
    include_directories(${Intl_INCLUDE_DIRS})
    link_libraries(${Intl_LIBRARIES})
  endif()
endif()

# PCM audio

if (EMSCRIPTEN)
  list(APPEND SUMMARY_AUDIO "Emscripten")
else()
  list(APPEND SUMMARY_AUDIO "SDL_mixer")
endif()

# CDROM

if (EMSCRIPTEN)
  list(APPEND SUMMARY_CDROM "Emscripten")
else()
  list(APPEND SUMMARY_CDROM "SDL_mixer (wav|mp3|ogg...)")
endif()
set(DEFAULT_PLAYLIST_PATH "playlist.txt" CACHE STRING "Default playlist path")

# MIDI

if (EMSCRIPTEN)
  list(APPEND SUMMARY_MIDI "Emscripten")
elseif (ANDROID)
  list(APPEND SUMMARY_MIDI "Android")
else()
  list(APPEND SUMMARY_MIDI "SDL_mixer")
  set(ENABLE_MIDI_SDLMIXER 1)

  if (HAVE_SIGACTION)
    list(APPEND SUMMARY_MIDI "raw")
    set(MIDI_DEVICE "/dev/midi" CACHE STRING "midi device")
    set(ENABLE_MIDI_RAWMIDI 1)
  endif()

  if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    check_symbol_exists(SNDCTL_SEQ_NRSYNTHS "machine/soundcard.h" ENABLE_MIDI_SEQMIDI)
  elseif (CMAKE_SYSTEM_NAME MATCHES "(NetBSD|OpenBSD)")
    check_symbol_exists(SNDCTL_SEQ_NRSYNTHS "soundcard.h" ENABLE_MIDI_SEQMIDI)
  else ()
    check_symbol_exists(SNDCTL_SEQ_NRSYNTHS "sys/soundcard.h" ENABLE_MIDI_SEQMIDI)
  endif()

  if (ENABLE_MIDI_SEQMIDI)
    list(APPEND SUMMARY_MIDI "OSS sequencer")
    set(SEQ_DEVICE "/dev/sequencer" CACHE STRING "sequencer device")
  endif()

  if (HAVE_PORTMIDI)
    list(APPEND SUMMARY_MIDI "PortMidi")
    set(ENABLE_MIDI_PORTMIDI 1)
  endif()
endif()

function (print_summary name)
  if (ARGN)
    list(JOIN ARGN ", " MSG)
  else()
    set(MSG "none")
  endif()
  message(" ${name}: ${MSG}")
endfunction()

message("xsystem35 summary:")
message("------------------")
print_summary(audio ${SUMMARY_AUDIO})
print_summary(cdrom ${SUMMARY_CDROM})
print_summary(midi ${SUMMARY_MIDI})
message("------------------")

set(PACKAGE ${PROJECT_NAME})
configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(fonts)
if (ENABLE_NLS)
  add_subdirectory(po)
endif()

add_subdirectory(modules)
target_link_libraries(xsystem35 PRIVATE modules)
