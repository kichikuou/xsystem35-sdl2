# Contributing to xsystem35-sdl2

Thank you for your interest in contributing. This project is maintained by a
single person in their spare time, so review capacity is limited.

## Before You Start

- **Bug fixes**: open a pull request directly. Please describe the problem,
  the affected game(s), and how to reproduce it.
- **New features, refactorings, and new dependencies**: open an issue first and
  discuss the idea. Pull requests for large changes that were not discussed in
  advance are likely to be closed.

## Pull Requests

- **Keep it small and focused.** One topic per pull request. Split large work
  into a series of incremental changes, each of which builds and works on its
  own.
- **Write a clear description**: what the change does, why it is needed, and
  how you tested it (platform, game title, scene).
- **Make sure CI passes.** If you change engine behavior covered by the
  scenario tests in `test/`, update the tests as well.
- **Follow the style of the surrounding code.**
- AI tools are fine to use, but you are responsible for every line you submit
  and should be able to explain it in review.

## Scope

xsystem35-sdl2 aims to be a faithful implementation of the System 3.x engine
that runs on many platforms.

- **No xsystem35-specific scenario commands.** xsystem35 does not add
  commands or DLL functions that do not exist in System 3.9. Scenarios must
  remain runnable on the original engine, so extensions to the scenario
  language are not accepted.

  The one exception is Unicode support (see "Unicode Translation Support" in
  `README.md`). Translated games should use this mechanism rather than adding
  support for other character encodings.
- **Features should work on all supported platforms.** New features that are
  available on only one platform are generally not accepted.
- Game-specific workarounds belong in `src/hacks.c` and should be minimal.
- Do not include game data or any file derived from copyrighted game files.

## License

By submitting a contribution, you agree that it is distributed under the GNU
General Public License version 2 or later (see `COPYING`).
