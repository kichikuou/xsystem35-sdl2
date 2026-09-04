cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# Compilation of SDL and companion libraries
include(FetchContent)
FetchContent_Declare(
  SDL
  URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-2.32.10.tar.gz
  URL_HASH SHA1=ce98fa93e31836a751feca374ab28a0770b63c16
)
FetchContent_Declare(
  freetype
  URL https://downloads.sourceforge.net/project/freetype/freetype2/2.14.3/freetype-2.14.3.tar.gz
      https://download.savannah.gnu.org/releases/freetype/freetype-2.14.3.tar.gz
  URL_HASH SHA1=a911f65c6355ddcf7aaba46e250dbc00f67e1678
)
FetchContent_Declare(
  SDL_mixer
  URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-2.8.0/SDL2_mixer-2.8.0.tar.gz
  URL_HASH SHA1=a58c69f9d00e44833b9e00e1adb58d85759ca499
)

# Only FreeType's own rasterizer is needed, none of its optional dependencies.
set(FT_DISABLE_ZLIB ON CACHE BOOL "Disable use of system zlib" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable use of system libbz2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "Disable use of system libpng" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable use of harfbuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable use of brotli" FORCE)

set(SDL2MIXER_OPUS OFF CACHE BOOL "Enable Opus music" FORCE)
set(SDL2MIXER_FLAC OFF CACHE BOOL "Enable FLAC music" FORCE)
set(SDL2MIXER_MOD  OFF CACHE BOOL "Support loading MOD music" FORCE)
set(SDL2MIXER_MIDI OFF CACHE BOOL "Enable MIDI music" FORCE)
set(SDL2MIXER_WAVPACK OFF CACHE BOOL "Enable WavPack music" FORCE)
set(SDL2MIXER_SAMPLES OFF CACHE BOOL "Build the SDL2_mixer sample program(s)" FORCE)
set(SDL2MIXER_INSTALL OFF CACHE BOOL "Enable SDL2_mixer install target" FORCE)

FetchContent_MakeAvailable(SDL freetype SDL_mixer)

# The main CMakeLists.txt of xsystem35
add_subdirectory(${PROJECT_ROOT_DIR} xsystem35)

# Copy asset files
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src/main/assets)
file(MAKE_DIRECTORY ${ASSETS_DIR}/licenses)
file(COPY_FILE ${PROJECT_ROOT_DIR}/fonts/MTLc3m.ttf ${ASSETS_DIR}/MTLc3m.ttf)
file(COPY_FILE ${PROJECT_ROOT_DIR}/fonts/mincho.ttf ${ASSETS_DIR}/mincho.ttf)
file(COPY_FILE ${PROJECT_ROOT_DIR}/COPYING ${ASSETS_DIR}/licenses/xsystem35)
file(COPY_FILE ${PROJECT_ROOT_DIR}/licenses/MTLc3m.txt ${ASSETS_DIR}/licenses/MTLc3m)
file(COPY_FILE ${PROJECT_ROOT_DIR}/licenses/mincho.txt ${ASSETS_DIR}/licenses/mincho)
file(COPY_FILE ${PROJECT_ROOT_DIR}/licenses/nanojpeg.txt ${ASSETS_DIR}/licenses/nanojpeg)
file(COPY_FILE ${PROJECT_ROOT_DIR}/licenses/microui.txt ${ASSETS_DIR}/licenses/microui)
file(COPY_FILE ${sdl_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL)
file(COPY_FILE ${freetype_SOURCE_DIR}/docs/GPLv2.TXT ${ASSETS_DIR}/licenses/freetype)
file(COPY_FILE ${sdl_mixer_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_mixer)
