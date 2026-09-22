cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# SDL3 is provided by the official Android AAR through Prefab.
find_package(SDL3 3.4 REQUIRED CONFIG)

include(FetchContent)
FetchContent_Declare(
  freetype
  URL https://downloads.sourceforge.net/project/freetype/freetype2/2.14.3/freetype-2.14.3.tar.gz
      https://download.savannah.gnu.org/releases/freetype/freetype-2.14.3.tar.gz
  URL_HASH SHA1=a911f65c6355ddcf7aaba46e250dbc00f67e1678
)
FetchContent_Declare(
  SDL_mixer
  URL https://github.com/libsdl-org/SDL_mixer/releases/download/release-3.2.4/SDL3_mixer-3.2.4.tar.gz
  URL_HASH SHA256=182a07c745375e113dc740d43964ff21b0be29f29f59876c4dbc4db3d32f6901
)

# Only FreeType's own rasterizer is needed, none of its optional dependencies.
set(FT_DISABLE_ZLIB ON CACHE BOOL "Disable use of system zlib" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable use of system libbz2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "Disable use of system libpng" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable use of harfbuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable use of brotli" FORCE)

set(SDLMIXER_VENDORED OFF CACHE BOOL "Use vendored third-party libraries" FORCE)
set(SDLMIXER_DEPS_SHARED OFF CACHE BOOL "Load dependencies dynamically" FORCE)
set(SDLMIXER_FLAC OFF CACHE BOOL "Enable FLAC audio" FORCE)
set(SDLMIXER_GME OFF CACHE BOOL "Enable GME audio" FORCE)
set(SDLMIXER_MOD OFF CACHE BOOL "Enable MOD audio" FORCE)
set(SDLMIXER_MP3_MPG123 OFF CACHE BOOL "Enable mpg123 MP3 decoder" FORCE)
set(SDLMIXER_MIDI OFF CACHE BOOL "Enable MIDI audio" FORCE)
set(SDLMIXER_OPUS OFF CACHE BOOL "Enable Opus audio" FORCE)
set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "Enable vorbisfile decoder" FORCE)
set(SDLMIXER_VORBIS_TREMOR OFF CACHE BOOL "Enable Tremor decoder" FORCE)
set(SDLMIXER_WAVPACK OFF CACHE BOOL "Enable WavPack audio" FORCE)
set(SDLMIXER_TESTS OFF CACHE BOOL "Build SDL3_mixer tests" FORCE)
set(SDLMIXER_EXAMPLES OFF CACHE BOOL "Build SDL3_mixer examples" FORCE)
set(SDLMIXER_INSTALL OFF CACHE BOOL "Enable SDL3_mixer install target" FORCE)

FetchContent_MakeAvailable(freetype SDL_mixer)

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
file(COPY_FILE ${freetype_SOURCE_DIR}/docs/GPLv2.TXT ${ASSETS_DIR}/licenses/freetype)
file(COPY_FILE ${sdl_mixer_SOURCE_DIR}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_mixer)
