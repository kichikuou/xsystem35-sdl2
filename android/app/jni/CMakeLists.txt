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
# TODO: Use SDL2_mixer 2.0.5 once released
FetchContent_Declare(
  SDL_mixer
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
  GIT_TAG 20433729fb318f7b6f773a42ffa410114cb067f2
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
  SET(SUPPORT_OGG ON CACHE BOOL "Enable OGG support in SDL_mixer" FORCE)
  add_subdirectory(${sdl_mixer_SOURCE_DIR} ${sdl_mixer_BINARY_DIR})
  target_compile_definitions(SDL2_mixer PRIVATE MUSIC_WAV)
endif()

# The main CMakeLists.txt of xsystem35
add_subdirectory(../../.. xsystem35)
