# xsytem35-sdl2

アリスソフトのゲームエンジン System3.x のフリー実装である xsystem35 を SDL2 に対応して、emscripten でコンパイルできるようにしたものです。

## ビルド方法
### Linux

[cmake](https://cmake.org/) が必要です。

    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../../
    $ make && make install

cmake の実行でエラーになる場合は必要なライブラリをインストールしてください。

グラフィックスシステムとして X11 と SDL2 が使用可能です。両方存在する場合は X11 が優先されますが、`cmake` のオプションに `-DENABLE_X11=NO` を指定すると SDL2 が使われます。

### MacOS

[Homebrew](https://brew.sh/index_ja) が必要です。

    $ brew install cmake pkg-config sdl2 sdl2_mixer freetype
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ../../
    $ make && make install

### Windows

[MSYS2](https://www.msys2.org) が必要です。

    $ pacman -S cmake mingw-w64-x86_64-cmake mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-SDL2_mixer
    $ mkdir -p out/debug
    $ cd out/debug
    $ cmake -G"MSYS Makefiles" -DCMAKE_BUILD_TYPE=Debug ../../
    $ make

### Emscripten

    $ mkdir -p out/wasm
    $ cd out/wasm
    $ emcmake cmake -DCMAKE_BUILD_TYPE=MinSizeRel ../../
    $ make

実行するには、[鬼畜王 on Webのリポジトリ](https://github.com/kichikuou/web)をチェックアウトして、`docs`ディレクトリに `out/xsystem35.*` をすべてコピーしてください。

### Android

[android/README.md](android/) を参照してください。
