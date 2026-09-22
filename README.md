# xsystem35 for SDL2/SDL3

This is a multi-platform port of `xsystem35`, a free implementation of
AliceSoft's System 3.x game engine.

## Compatibility

See the [game compatibility table](game_compatibility.md) for a list of games
that can be played with xsystem35-sdl2.

## Unique Features

In addition to the original System 3.x functionalities, xsystem35-sdl2 offers
the following features:

### Playing Audio Files as Virtual CD Music

Many System 3.x games feature music as audio tracks on the CD-ROM. xsystem35
can play music from audio files, eliminating the need to insert CDs. Supported
audio file formats are MP3 and Ogg. To use ripped audio files, create a file
named `playlist.txt` in the game directory and list the paths to your tracks,
one per line. For example:

```
# The first line is not used
BGM/track02.mp3
BGM/track03.mp3
...
```

The first line is not used because the first track on a game CD is typically a
data track.

Some games have integrated music as MIDI. In such cases, the music won't play
using the virtual CD feature. If you encounter a `Cannot load MIDI` error
message, you might need to set the `SDL_SOUNDFONTS` environment variable to
point to an `.sf2` file. For example:

```
SDL_SOUNDFONTS=/usr/share/soundfonts/GeneralUser.sf2 xsystem35
```

### Unicode Translation Support

While the original System 3.x only supported Shift_JIS (a Japanese character
encoding), xsystem35 supports Unicode and can run games translated into
languages other than Japanese and English.

For instructions on how to build a game with Unicode support, see the
[xsys35c](https://github.com/kichikuou/xsys35c) documentation.

### Debugging

xsystem35 features a built-in debugger that allows you to step through the game
and examine or modify game variables. There are two ways to use the debugger:

- Through [Visual Studio Code](https://code.visualstudio.com/) (recommended):
  The [vscode-system3x](https://github.com/kichikuou/vscode-system3x) extension
  provides a graphical debugging interface for System 3.x.
- Using the CLI Debugger: Running xsystem35 with the `-debug` option will
  launch the debugger with a console interface. Type `help` to see a list of
  available commands.

### Streamer Mode

xsystem35 introduces a "Streamer Mode" to make playing games with NSFW content
safer for streaming or public viewing. When enabled with the `-censor <file>`
option, images specified in the provided file will be automatically mosaiced.

The `misc/censor/` directory contains example censor list files for some games.

## Installation

Prebuilt packages for Windows and Android can be downloaded from the
[Releases](https://github.com/kichikuou/xsystem35-sdl2/releases) page.

Note for Windows:
- The 64-bit version supports Windows 10 or later. For older versions of
  Windows, please use the 32-bit version.
- Debugging is supported only in the 64-bit version.

For other platforms, refer to the [Building](#building) section.

## Running
### Windows

Copy `xsystem35.exe` to the game folder and run it.

### Android

See [android/README.md](android/README.md#usage).

### Other Platforms

Run xsystem35 from within the game directory.

```bash
$ cd /path/to/game_directory
$ xsystem35
```

See [xsystem35 command manual](doc/xsystem35.6.adoc) for detailed usage.

## Building

SDL2 is used by default on desktop platforms and Emscripten. Pass
`-DXSYSTEM35_SDL_VERSION=3` to CMake to select SDL3. Use a separate build
directory for each SDL version.

The supported configurations are:

| Platform | SDL2 | SDL3 |
| --- | --- | --- |
| Linux and macOS | Yes | Yes |
| Windows 64-bit | Yes | Yes |
| Windows 32-bit | Yes | No |
| Emscripten | Yes | Yes |
| Android | No | Yes |

SDL2 builds require SDL 2.18 or later and SDL2_mixer. SDL3 builds require
SDL 3.4 or later and SDL3_mixer 3.2 or later. The remaining common
dependencies are FreeType, zlib, and optionally libwebp, PortMidi, and cJSON.

### Linux (Debian / Ubuntu)

```bash
$ sudo apt install build-essential cmake libsdl2-dev libsdl2-mixer-dev \
    libfreetype-dev libwebp-dev libportmidi-dev libcjson-dev asciidoctor
$ cmake -S . -B out/sdl2 -DCMAKE_BUILD_TYPE=Debug \
    -DXSYSTEM35_SDL_VERSION=2
$ cmake --build out/sdl2
$ sudo cmake --install out/sdl2
```

For SDL3 on a distribution that provides SDL 3.4 or later (for example,
Ubuntu 26.04), install `libsdl3-dev` instead of the SDL2 packages and
configure `out/sdl3` with `-DXSYSTEM35_SDL_VERSION=3`. If SDL3_mixer 3.2 or
later is installed, CMake uses it; otherwise, CMake downloads and builds
SDL3_mixer 3.2.4 during configuration.

### MacOS

[Homebrew](https://brew.sh/) is required.

```bash
$ brew install cmake pkg-config sdl2 sdl2_mixer freetype webp portmidi \
    cjson asciidoctor
$ cmake -S . -B out/sdl2 -DCMAKE_BUILD_TYPE=Debug \
    -DXSYSTEM35_SDL_VERSION=2
$ cmake --build out/sdl2
$ sudo cmake --install out/sdl2
```

For SDL3, install `sdl3 sdl3_mixer` instead of `sdl2 sdl2_mixer` and
configure `out/sdl3` with `-DXSYSTEM35_SDL_VERSION=3`.

### Windows

[MSYS2](https://www.msys2.org) is required.

```bash
$ pacman -S mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-SDL2 \
    mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-freetype \
    mingw-w64-ucrt-x86_64-libwebp mingw-w64-ucrt-x86_64-portmidi \
    mingw-w64-ucrt-x86_64-cjson
$ cmake -S . -B out/sdl2 -G"MSYS Makefiles" \
    -DCMAKE_BUILD_TYPE=Debug -DXSYSTEM35_SDL_VERSION=2
$ cmake --build out/sdl2
```

For a 64-bit SDL3 build, install `mingw-w64-ucrt-x86_64-sdl3` and
`mingw-w64-ucrt-x86_64-sdl3-mixer` instead of the SDL2 packages, then
configure `out/sdl3` with `-DXSYSTEM35_SDL_VERSION=3`. The MSYS2 mingw32
repository does not provide SDL3, so 32-bit Windows builds remain on SDL2.

### Emscripten

```bash
$ emcmake cmake -S . -B out/wasm-sdl2 -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DXSYSTEM35_SDL_VERSION=2
$ cmake --build out/wasm-sdl2
```

For SDL3, use a different build directory and
`-DXSYSTEM35_SDL_VERSION=3`. Both versions use Emscripten's bundled SDL port;
SDL_mixer is not required.

To use the generated binary, check out
[Kichikuou on Web](https://github.com/kichikuou/web) and copy
`out/wasm-sdl2/src/xsystem35.*` into its `docs` directory.

### Android

See [android/README.md](android/).

Android uses SDL3 exclusively. The Gradle build downloads fixed official SDL3
3.4.16 and SDL3_mixer 3.2.4 AARs and exposes them to CMake through Prefab.
