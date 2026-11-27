TinySoundFont by Bernhard Schelling is MIT licensed (see LICENSE and tsf.h).
TinyMidiLoader is zlib licensed; its full license is preserved in tml.h.
Vendored from https://github.com/schellingb/TinySoundFont
commit a3621a19c6504885cd9583e6e26c35bc954fc8ec.

Local `tml.h` patches: defer relocation of the first chronological message until
track merging finishes, reject invalid timing headers, cap individual tracks at
16 MiB, and check track-buffer allocations before reading into them.
