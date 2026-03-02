# Censor Lists for Streamer Mode

This directory contains sample censor lists for various games, intended for use
with xsystem35's Streamer Mode.

## Usage

To use a censor list, specify its path with the `-censor` command-line option
or in your `.xsys35rc` file. For example:

```bash
xsystem35 -censor misc/censor/kichikuou.txt
```

The list files contain image numbers (integers), one per line. Lines starting
with `#` are treated as comments.

## Contributing

We welcome contributions of censor lists for other games! If you have created
a censor list that you'd like to share, please open a Pull Request.
