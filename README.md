# xsytem35-sdl2

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
can play music from audio files, eliminating the need to insert CDs. To use
ripped audio files, create a file named `playlist.txt` in the game directory
and list the paths to your tracks, one per line. For example:

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

## Installation

Prebuilt packages for Windows and Android can be downloaded from the
[Releases](https://github.com/kichikuou/xsystem35-sdl2/releases) page. For
other platforms, refer to the [Building](#building) section.

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
### Linux (Debian / Ubuntu)

```bash
$ sudo apt install build-essential cmake libgtk-3-dev libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev libwebp-dev libcjson-dev asciidoctor
$ mkdir -p out/debug
$ cd out/debug
$ cmake -DCMAKE_BUILD_TYPE=Debug ../../
$ make && make install
```

### MacOS

[Homebrew](https://brew.sh/) is required.

```bash
$ brew install cmake pkg-config sdl2 sdl2_mixer sdl2_ttf webp cjson asciidoctor
$ mkdir -p out/debug
$ cd out/debug
$ cmake -DCMAKE_BUILD_TYPE=Debug ../../
$ make && make install
```

### Windows

[MSYS2](https://www.msys2.org) is required.

```bash
$ pacman -S cmake mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_ttf mingw-w64-ucrt-x86_64-SDL2_mixer mingw-w64-ucrt-x86_64-libwebp mingw-w64-ucrt-x86_64-cjson
$ mkdir -p out/debug
$ cd out/debug
$ cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug ../../
$ make
```

### Emscripten

```bash
$ mkdir -p out/wasm
$ cd out/wasm
$ emcmake cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../../
$ make
```

To use the generated binary, check out
[Kichikuou on Web](https://github.com/kichikuou/web) and copy `out/xsystem35.*`
into its `docs` directory.

### Android

See [android/README.md](android/).
