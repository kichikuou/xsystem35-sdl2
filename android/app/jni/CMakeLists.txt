cmake_minimum_required(VERSION 3.14)

project(GAME)
set(PROJECT_ROOT_DIR ../../..)

# SDL3 is provided by the AAR through Prefab (see build.gradle).
find_package(SDL3 REQUIRED CONFIG)

# Compilation of companion libraries
include(FetchContent)
FetchContent_Declare(
  freetype
  URL https://downloads.sourceforge.net/project/freetype/freetype2/2.14.3/freetype-2.14.3.tar.gz
      https://download.savannah.gnu.org/releases/freetype/freetype-2.14.3.tar.gz
  URL_HASH SHA1=a911f65c6355ddcf7aaba46e250dbc00f67e1678
)

# Only FreeType's own rasterizer is needed, none of its optional dependencies.
set(FT_DISABLE_ZLIB ON CACHE BOOL "Disable use of system zlib" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "Disable use of system libbz2" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "Disable use of system libpng" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable use of harfbuzz" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable use of brotli" FORCE)

FetchContent_MakeAvailable(freetype)

# The main CMakeLists.txt of xsystem35
add_subdirectory(${PROJECT_ROOT_DIR} xsystem35)

# SDL3_mixer is fetched by fetch_sdl3_mixer() in the main CMakeLists.txt.
FetchContent_GetProperties(SDL3_mixer SOURCE_DIR sdl3_mixer_source_dir)

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
file(COPY_FILE ${sdl3_mixer_source_dir}/LICENSE.txt ${ASSETS_DIR}/licenses/SDL_mixer)
