cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# Compilation of SDL and companion libraries
include(FetchContent)
FetchContent_Declare(
  SDL
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.12/SDL2-2.30.12.tar.gz
  URL_HASH SHA1=36d48b045209dcd3576250c1571795ad0cd4d5a9
)
FetchContent_Declare(
  SDL_ttf
  URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.tar.gz
  URL_HASH SHA1=da5e86b601ad299a697878fab1af6f3be47b529d
)
FetchContent_Declare(
  SDL_mixer
  URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-2.8.0.tar.gz
  URL_HASH SHA1=a58c69f9d00e44833b9e00e1adb58d85759ca499
)

set(SDL2TTF_SAMPLES OFF CACHE BOOL "Build the SDL2_ttf sample program(s)" FORCE)
set(SDL2TTF_INSTALL OFF CACHE BOOL "Enable SDL2_ttf install target" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "Use vendored third-party libraries" FORCE)

set(SDL2MIXER_OPUS OFF CACHE BOOL "Enable Opus music" FORCE)
set(SDL2MIXER_FLAC OFF CACHE BOOL "Enable FLAC music" FORCE)
set(SDL2MIXER_MOD  OFF CACHE BOOL "Support loading MOD music" FORCE)
set(SDL2MIXER_MIDI OFF CACHE BOOL "Enable MIDI music" FORCE)
set(SDL2MIXER_WAVPACK OFF CACHE BOOL "Enable WavPack music" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "Build the SDL2_mixer sample program(s)" FORCE)
set(SDL2MIXER_INSTALL OFF CACHE BOOL "Enable SDL2_mixer install target" FORCE)

FetchContent_MakeAvailable(SDL SDL_ttf SDL_mixer)

# The main CMakeLists.txt of xsystem35
add_subdirectory(${PROJECT_ROOT_DIR} xsystem35)

# Copy asset files
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/assets)
file(MAKE_DIRECTORY ${ASSETS_DIR}/licenses)
file(COPY_FILE ${PROJECT_ROOT_DIR}/fonts/MTLc3m.ttf ${ASSETS_DIR}/MTLc3m.ttf)
file(COPY_FILE ${PROJECT_ROOT_DIR}/fonts/mincho.otf ${ASSETS_DIR}/mincho.otf)
file(COPY_FILE ${PROJECT_ROOT_DIR}/COPYING ${ASSETS_DIR}/licenses/xsystem35)
file(COPY_FILE ${PROJECT_ROOT_DIR}/fonts/MTLc3m.ttf.license ${ASSETS_DIR}/licenses/MTLc3m)
file(COPY_FILE ${PROJECT_ROOT_DIR}/fonts/mincho.otf.license ${ASSETS_DIR}/licenses/mincho)
file(COPY_FILE ${sdl_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL)
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_ttf)
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/external/freetype/docs/GPLv2.TXT ${ASSETS_DIR}/licenses/freetype)
file(COPY_FILE ${sdl_ttf_SOURCE_DIR}/external/harfbuzz/COPYING ${ASSETS_DIR}/licenses/harfbuzz)
file(COPY_FILE ${sdl_mixer_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_mixer)
