#!/usr/bin/env python3
# Rename the font's internal name records.
#
# IPA明朝 is distributed under the IPA Font License v1.0. A subset of it is a
# "Derived Program" under that license, and Article 3.1(4) forbids using or
# including the Licensed Program's name ("IPA"/"IPAMincho") as the program,
# font, or file name of the Derived Program. pyftsubset preserves the original
# name table, so rename it here to a name that does not contain "IPA".
import sys
from fontTools.ttLib import TTFont

if len(sys.argv) != 3:
    print('Usage: rename_font.py infile outfile')
    exit(1)

FAMILY = 'xsystem35 Mincho'
PSNAME = 'xsystem35-Mincho'

font = TTFont(sys.argv[1])
name = font['name']

# Rewrite the family / full / typographic-family / PostScript / unique-ID
# records in place, preserving each record's platform, encoding and language.
for rec in list(name.names):
    if rec.nameID in (1, 4, 16):
        value = FAMILY
    elif rec.nameID in (3, 6):
        value = PSNAME
    else:
        continue
    name.setName(value, rec.nameID, rec.platformID, rec.platEncID, rec.langID)

font.save(sys.argv[2])
