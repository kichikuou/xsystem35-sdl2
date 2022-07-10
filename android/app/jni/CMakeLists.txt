cmake_minimum_required(VERSION 3.13)

project(GAME)

include(FetchContent)
FetchContent_Declare(
  SDL
  URL https://libsdl.org/release/SDL2-2.0.20.tar.gz
  URL_HASH SHA1=18ce006cbf07502f48abfedc088c14404091f3d3
)
FetchContent_Declare(
  SDL_ttf
  URL https://github.com/libsdl-org/SDL_ttf/archive/refs/tags/release-2.0.18.tar.gz
  URL_HASH SHA1=6fc5130757b447d46c3da6abd58687e8576de8b8
)
FetchContent_Declare(
  SDL_mixer
  URL https://github.com/libsdl-org/SDL_mixer/archive/refs/tags/release-2.6.0.tar.gz
  URL_HASH SHA1=a2aef03d74ac3dd9cd0004cff1834602a5dafa66
)

# Compilation of SDL and companion libraries
FetchContent_GetProperties(SDL)
if(NOT sdl_POPULATED)
  FetchContent_Populate(SDL)
  add_subdirectory(${sdl_SOURCE_DIR} ${sdl_BINARY_DIR})
endif()
FetchContent_GetProperties(SDL_ttf)
if(NOT sdl_ttf_POPULATED)
  FetchContent_Populate(SDL_ttf)
  add_subdirectory(${sdl_ttf_SOURCE_DIR} ${sdl_ttf_BINARY_DIR})
endif()
FetchContent_GetProperties(SDL_mixer)
if(NOT sdl_mixer_POPULATED)
  FetchContent_Populate(SDL_mixer)
  SET(SDL2MIXER_OPUS OFF CACHE BOOL "Enable Opus music" FORCE)
  SET(SDL2MIXER_FLAC OFF CACHE BOOL "Enable FLAC music" FORCE)
  SET(SDL2MIXER_MOD  OFF CACHE BOOL "Support loading MOD music" FORCE)
  SET(SDL2MIXER_MIDI OFF CACHE BOOL "Enable MIDI music" FORCE)
  SET(SDL2MIXER_SAMPLES OFF CACHE BOOL "Build the SDL2_mixer sample program(s)" FORCE)
  add_subdirectory(${sdl_mixer_SOURCE_DIR} ${sdl_mixer_BINARY_DIR})
endif()

# The main CMakeLists.txt of xsystem35
add_subdirectory(../../.. xsystem35)
