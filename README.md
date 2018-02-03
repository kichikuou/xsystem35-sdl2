# xsytem35-sdl2

アリスソフトのゲームエンジン System3.x のフリー実装である xsystem35 を SDL2 に対応して、emscripten でコンパイルできるようにしたものです。

## ビルド方法
#### Emscripten

    $ emmake make -f Makefile.emscripten

実行するには、[鬼畜王 on Webのリポジトリ](https://github.com/kichikuou/web)をチェックアウトして、`docs`ディレクトリに `out/xsystem35.*` をすべてコピーしてください。

#### Linux

[オリジナルのドキュメント](https://github.com/kichikuou/xsystem35-sdl2/tree/emscripten/doc)も参考にしてください。

    $ ./configure
    $ make
