# Changelog

## 2.11.4 - 2024-05-22
- Windows: In full-screen mode, the menu bar now appears only when the mouse pointer is at the top of the screen. (#53)
- Fixed text rendering issue in Rance 3 intro. (#54)

## 2.11.3 - 2024-04-11
- Android: Fixed a bug where save files from older versions could not be loaded in version 2.11.2.

## 2.11.2 - 2024-04-10
- Added `-savedir` option to specify save directory.
- Android: Fixed a bug where .xsys35rc in a subdirectory of ZIP was not loaded. (#51)

## 2.11.1 - 2024-02-18
- Fixed a black screen issue in the MangaGamer version of Rance 5D (#45).
- Fixed a crash bug in command-line debugger.

## 2.11.0 - 2024-01-20
- Windows: Now `xsystem35.exe` is a standalone executable. You can just copy it to the game folder and run it.
- Windows: The installer is no longer provided. If you have an earlier version installed, please uninstall it.
- Windows: The debugger is no longer available in the 32-bit version. Please use the 64-bit version if you need it.
- Added experimental `enable_zb` option (#44). See `xsys35rc.sample` for details.

## 2.10.1 - 2024-01-01
- Windows: Added "Integer Scaling" menu option
- Fixed initial palette colors (to match System3.9)
- Fixed a bug where palette 246 to 249 were unintentionally changed after loading 256-color CGs

## 2.10.0 - 2023-12-09
- Breaking change: A [bug](https://github.com/kichikuou/xsystem35-sdl2/issues/41) has been fixed in which save format for 大悪司 and かえるにょ国にょアリス was not compatible with System3.9. As a result, save files for these games created with older versions of xsystem35 are no longer usable. (Loading will not fail, but wrong values will be loaded.)
- Now the 64-bit executable supports Windows 10 or later. For older Windows, please use the 32-bit version.
- Fixed a problem with the download edition of Daiakuji requiring insertion of Disk 2. (#43)
- Debugger: Added debug commands for monitoring the color palette.
- Minor bug fixes.

## 2.9.1 - 2023-06-22
- Fixed a bug where the `CX 1` command (copy with transparent color) did not work in some games (e.g. Daiakuji)
- Debugger: Now breakpoints do not make the window completely unresponsive
- Debugger: Fixed a conditional breakpoint crash bug

## 2.9.0 - 2023-05-04
- Now xsystem35 uses the System3.9 save file format (unless `-saveformat` command line flag is specified). Old save files can still be loaded.
- Changed the default naming convention for save files from `[a-z]sleep.asd` to `<gamename>s[a-z].asd`.
- Fixed a bug where SACT games (e.g. Rance 5D) were not responding to touch.
- Fixed memory leaks in debugger.

## 2.8.0 - 2023-02-12
- Added support for Rance 4.1/4.2 ver1.05
- Fixed movement animation in rooms in Rance 4 v2.05 (#35)
- Added `-game` option which can be used to enable game-specific hacks in translated games

## 2.7.0 - 2023-01-28
- Added support for Rance4 ver.2.05
- Now supports BGM playback for the download edition of Daiakuji
- Bug fixes

## 2.6.0 - 2023-01-01
- Android: Introduced a new way to simulate right-click; tapping on the black bars at the left/right or top/bottom of the screen is treated as a right click. (Two-finger touch, the old way to simulate a right click, still works.)
- Windows: Added a menu command to enable/disable automatic mouse movements.

## 2.5.1 - 2022-07-31
- Implemented "palette shift" graphic effect (used in 闘神都市II).
- Implemented `grDrawFillCircle` command (used in グレイメルカ).
- Fixed color update bug in 256-color games.
- Fixed crash by out-of-bounds variable access in debugger.

## 2.5.0 - 2022-07-17
- Supported JPEG image format on Android.

## 2.4.0 - 2022-07-02
- Windows: Added "Restart" menu command.
- Fixed a bug where wrong music is played in Kichikuou Rance (System3.9 version). #32
- Supported mouse wheel input.
- Fixed a bug in `MF` command.
- Implemented `grEffectMoveView` command.

## 2.3.0 - 2022-04-08
- Code for screen transition effects have been substantially rewritten (but you probably wouldn't notice any difference).
- Fixed bugs in System3.9 games.
- Android: Enabled joystick input.
- Debugger improvements.

## 2.2.0 - 2021-12-03
From now on, the Android and Windows versions will use a common version number.

- Android: Use custom fonts specified in `.xsys35c`. (#20)
- Supported "王子さまLv1".
- Debugger improvements.
- Fixed some bugs and compatibility issues.

## 2.1.0 - 2021-11-07
- Fixed issues with MangaGamer version of Rance 5D.
- Fixed garbled text in some System3.9 games with Unicode mode.
- Display a message box on fatal errors.
- Various debugger improvements.

## 2.0.1 - 2021-10-24
- Fixed crash during shutdown.

## 2.0.0 - 2021-10-23
- Added window menubar from which you can save screenshots, change fullscreen mode, and enable message skipping.
- Added debugger support that can be used from Visual Studio Code. See [vscode-system3x](https://github.com/kichikuou/vscode-system3x) for details.
- Many bug fixes and compatibility fixes.
