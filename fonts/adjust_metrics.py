#!/usr/bin/env python3
import sys
from fontTools.ttLib import TTFont

if len(sys.argv) != 3:
    print('Usage: adjust_metrics.py infile outfile')
    exit(1)

font = TTFont(sys.argv[1])

hhea = font['hhea']
os2 = font['OS/2']

if hhea.ascent != os2.sTypoAscender:
    print(f'Changing hhea.ascent from {hhea.ascent} to {os2.sTypoAscender}')
    hhea.ascent = os2.sTypoAscender

if hhea.descent != os2.sTypoDescender:
    print(f'Changing hhea.descent from {hhea.descent} to {os2.sTypoDescender}')
    hhea.descent = os2.sTypoDescender

font.save(sys.argv[2])
