#include <stdio.h>
#include "../src/s2utbl.h"

void print_utf8(int code) {
	if (code <= 0x7f) {
		putchar(code);
	} else if (code <= 0x7ff) {
		putchar(0xc0 | code >> 6);
		putchar(0x80 | (code & 0x3f));
	} else {
		putchar(0xe0 | code >> 12);
		putchar(0x80 | (code >> 6 & 0x3f));
		putchar(0x80 | (code & 0x3f));
	}
}

int main() {
	// ASCII
	for (int c = ' '; c <= '~'; c++)
		putchar(c);
	putchar('\n');

	// Half-width kana
	for (int c = 0xff61; c <= 0xff9f; c++)
		print_utf8(c);
	putchar('\n');

	for (int hi = 0; hi < 0x80; hi++) {
		for (int lo = 0; lo < 0xc0; lo++) {
			int code = s2u[hi][lo];
			if (code == 0x30fb && !(hi == 0x01 && lo == 0x05))
				continue;
			print_utf8(code);
			putchar('\n');
		}
	}
}
