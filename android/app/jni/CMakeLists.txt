cmake_minimum_required(VERSION 3.13)

project(GAME)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()

include(FetchContent)
FetchContent_Declare(
  SDL
  URL http://libsdl.org/release/SDL2-2.0.12.tar.gz
  URL_HASH SHA1=e8ba91bf5eb438dbcf1fedf0530a1bcbe30d3bb6
  PATCH_COMMAND patch -p1 -i ${CMAKE_CURRENT_LIST_DIR}/SDL.patch
)
# TODO: Use SDL2_ttf 2.0.16 once released
FetchContent_Declare(
  SDL_ttf
  URL https://hg.libsdl.org/SDL_ttf/archive/d56d67db41d8.tar.gz # rev 420
  URL_HASH SHA1=143089a2b2973a5baeee0914af43608cfe8e1d87
)
# TODO: Use SDL2_mixer 2.0.5 once released
FetchContent_Declare(
  SDL_mixer
  URL https://hg.libsdl.org/SDL_mixer/archive/d2686bcfcba2.tar.gz # rev 1127
  URL_HASH SHA1=4e62e29bd5628b262b3ffb6d5d29861c76a1500e
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
  # Workaround for CMakeLists.txt bugs of SDL_mixer.
  # TODO: Fix these in upstream
  add_library(vorbisfile INTERFACE)
  target_include_directories(SDL2_mixer PUBLIC ${sdl_mixer_SOURCE_DIR}/include)
  target_compile_definitions(SDL2_mixer PRIVATE MUSIC_WAV)
endif()

# The main CMakeLists.txt of xsystem35
add_subdirectory(../../.. xsystem35)
