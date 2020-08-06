# xsytem35-sdl2

This is a multi-platform port of xsystem35, a free implementation of AriceSoft's System3.x game engine.

## Download
Prebuilt binaries for Windows and Android can be downloaded from the [Releases](https://github.com/kichikuou/xsystem35-sdl2/releases) page.

## Build
### Linux (Debian / Ubuntu)

    $ sudo apt install build-essential cmake libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../../
    $ make && make install

You can choose between X11 and SDL2 as the graphics backend. If both libraries are found, X11 is used by default, but if `-DENABLE_X11=NO` flag is given to the `cmake` invocation, then SDL2 will be used.

### MacOS

[Homebrew](https://brew.sh/index_ja) is needed.

    $ brew install cmake pkg-config sdl2 sdl2_mixer freetype libjpeg
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../../
    $ make && make install

### Windows

[MSYS2](https://www.msys2.org) is needed.

    $ pacman -S cmake mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-libjpeg-turbo
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug ../../
    $ make

### Emscripten

    $ mkdir -p out/wasm
    $ cd out/wasm
    $ emcmake cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../../
    $ make

To use the generated binary, checkout [Kichikuou on Web](https://github.com/kichikuou/web) and copy `out/xsystem35.*` into its `docs` directory.

### Android

See [android/README.md](android/).
