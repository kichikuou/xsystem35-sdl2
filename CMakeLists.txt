cmake_minimum_required(VERSION 3.13...4.0)

project(xsystem35 LANGUAGES C)
set(CMAKE_C_STANDARD 99)
enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(XSYSTEM35_VERSION "2.20.0")

if (ANDROID)
  set(XSYSTEM35_DEFAULT_SDL_VERSION 3)
else()
  set(XSYSTEM35_DEFAULT_SDL_VERSION 2)
endif()
set(XSYSTEM35_SDL_VERSION "${XSYSTEM35_DEFAULT_SDL_VERSION}" CACHE STRING
  "SDL major version (2 or 3)")
set_property(CACHE XSYSTEM35_SDL_VERSION PROPERTY STRINGS 2 3)
if (NOT XSYSTEM35_SDL_VERSION STREQUAL "2" AND
    NOT XSYSTEM35_SDL_VERSION STREQUAL "3")
  message(FATAL_ERROR "XSYSTEM35_SDL_VERSION must be 2 or 3")
endif()
add_library(xsystem35_sdl INTERFACE)
target_compile_definitions(xsystem35_sdl INTERFACE
  XSYSTEM35_SDL_VERSION=${XSYSTEM35_SDL_VERSION})
add_library(xsystem35_sdl_mixer INTERFACE)

include(CheckSymbolExists)
include(FetchContent)

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

function(fetch_webp)
  FetchContent_Declare(
    libwebp
    URL https://storage.googleapis.com/downloads.webmproject.org/releases/webp/libwebp-1.5.0.tar.gz
    URL_HASH SHA1=b21aa842136dc59a72a38776a5aa73f4d0b00ac5
    )
  set(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "Build animation utilities." FORCE)
  set(WEBP_BUILD_CWEBP OFF CACHE BOOL "Build the cwebp command line tool." FORCE)
  set(WEBP_BUILD_DWEBP OFF CACHE BOOL "Build the dwebp command line tool." FORCE)
  set(WEBP_BUILD_GIF2WEBP OFF CACHE BOOL "Build the gif2webp conversion tool." FORCE)
  set(WEBP_BUILD_IMG2WEBP OFF CACHE BOOL "Build the img2webp animation tool." FORCE)
  set(WEBP_BUILD_VWEBP OFF CACHE BOOL "Build the vwebp viewer tool." FORCE)
  set(WEBP_BUILD_WEBPINFO OFF CACHE BOOL "Build the webpinfo command line tool." FORCE)
  set(WEBP_BUILD_LIBWEBPMUX OFF CACHE BOOL "Build the libwebpmux library." FORCE)
  set(WEBP_BUILD_WEBPMUX OFF CACHE BOOL "Build the webpmux command line tool." FORCE)
  set(WEBP_BUILD_EXTRAS OFF CACHE BOOL "Build extras." FORCE)
  set(WEBP_BUILD_WEBP_JS OFF CACHE BOOL "Emscripten build of webp.js." FORCE)
  FetchContent_MakeAvailable(libwebp)
  add_library(WebP ALIAS webp)
endfunction()

check_symbol_exists(getlogin "unistd.h" HAVE_GETLOGIN)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(sigaction "signal.h" HAVE_SIGACTION)

if (EMSCRIPTEN)
  option(JSPI "Use JavaScript Promise Integration instead of Asyncify" OFF)

  function(add_emscripten_library name option)
    add_library(${name} INTERFACE)
    set_target_properties(${name} PROPERTIES
        INTERFACE_COMPILE_OPTIONS ${option}
        INTERFACE_LINK_OPTIONS ${option})
  endfunction()
  add_emscripten_library(zlib -sUSE_ZLIB=1)
  target_compile_options(xsystem35_sdl INTERFACE
    -sUSE_SDL=${XSYSTEM35_SDL_VERSION})
  target_link_options(xsystem35_sdl INTERFACE
    -sUSE_SDL=${XSYSTEM35_SDL_VERSION})
  add_emscripten_library(freetype2 -sUSE_FREETYPE=1)
  set(DEFAULT_FONT_PATH /fonts/)
  fetch_webp()
  set(HAVE_WEBP 1)

elseif (ANDROID)
  if (NOT XSYSTEM35_SDL_VERSION STREQUAL "3")
    message(FATAL_ERROR "Android supports SDL3 only")
  endif()
  target_link_libraries(xsystem35_sdl INTERFACE SDL3::SDL3)
  target_link_libraries(xsystem35_sdl_mixer INTERFACE
    SDL3_mixer::SDL3_mixer)
  add_library(freetype2 ALIAS freetype)
  find_library(ndk_log log)
  find_library(ndk_zlib z)
  add_library(zlib INTERFACE)
  set_target_properties(zlib PROPERTIES INTERFACE_LINK_LIBRARIES ${ndk_zlib})
  fetch_webp()
  set(HAVE_WEBP 1)

else()  # non-emscripten, non-android
  if (WIN32)
    set(ZLIB_USE_STATIC_LIBS ON)
  endif()
  find_package(ZLIB REQUIRED)
  add_library(zlib ALIAS ZLIB::ZLIB)

  include(FindPkgConfig)
  pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
  if (XSYSTEM35_SDL_VERSION STREQUAL "2")
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2>=2.18.0)
    pkg_check_modules(SDL2MIXER REQUIRED IMPORTED_TARGET SDL2_mixer)
    if (WIN32)
      add_static_library(sdl2 SDL2)
      add_static_library(sdl2_mixer SDL2MIXER)
      target_link_libraries(xsystem35_sdl INTERFACE sdl2)
      target_link_libraries(xsystem35_sdl_mixer INTERFACE sdl2_mixer)
    else()
      target_link_libraries(xsystem35_sdl INTERFACE PkgConfig::SDL2)
      target_link_libraries(xsystem35_sdl_mixer INTERFACE
        PkgConfig::SDL2MIXER)
    endif()
  else()
    find_package(SDL3 3.4 REQUIRED CONFIG)
    find_package(SDL3_mixer 3.2 REQUIRED CONFIG)
    target_link_libraries(xsystem35_sdl INTERFACE SDL3::SDL3)
    target_link_libraries(xsystem35_sdl_mixer INTERFACE
      SDL3_mixer::SDL3_mixer)
  endif()
  if (WIN32)
    add_static_library(freetype2 FREETYPE)
    # harfbuzz and graphite2, which the static freetype depends on, are C++ libraries.
    target_link_libraries(freetype2 INTERFACE -lstdc++)
  else()
    add_library(freetype2 ALIAS PkgConfig::FREETYPE)
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

  if (WIN32)
    optional_pkg_check_modules(PORTMIDI IMPORTED_TARGET portmidi)
    if (PORTMIDI_FOUND)
      set(HAVE_PORTMIDI 1)
      add_static_library(portmidi_static PORTMIDI)
      set(PORTMIDI portmidi_static)
    endif()
  else ()
    find_library(PORTMIDI portmidi)
    if (PORTMIDI)
      set(HAVE_PORTMIDI 1)
    endif()
  endif()
endif()

# i18n support
#
# Translate via libintl if available; otherwise fall back to a built-in catalog
# compiled from the .po files.
#
# To add a language, create po/<lang>.po and list <lang> here.
set(NLS_LANGUAGES ja)

if (NOT EMSCRIPTEN AND NOT ANDROID AND NOT WIN32)
  include(FindIntl)
  include(FindGettext)
  if (Intl_FOUND AND GETTEXT_FOUND)
    set(HAVE_LIBINTL 1)
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
print_summary(SDL ${XSYSTEM35_SDL_VERSION})
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
if (NOT ANDROID AND NOT EMSCRIPTEN)
  add_subdirectory(doc)
endif()
add_subdirectory(po)

add_subdirectory(modules)
target_link_libraries(xsystem35 PRIVATE modules)

if (NOT ANDROID AND NOT EMSCRIPTEN)
  add_subdirectory(test)
endif()
