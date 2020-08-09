/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "hankaku.h"
#include "utfsjis.h"
#include "unittest.h"

typedef struct {
	const char *zen;
	const char *han;
} Zen2hanTestCase;

static void zen2han_test(void) {
	const Zen2hanTestCase testcases[] = {
		{"", ""},
		{"　", " "},
		{"、。，．", "､｡,."},
		{"・：；？！", "･:;?!"},
		{"゛゜｀＾", "ﾞﾟ`^"},
		{"￣＿ー／｜", "~_ｰ/|"},
		{"’”", "'\""},
		{"（）［］｛｝「」", "()[]{}｢｣"},
		{"＋－＝＜＞￥＄％＃＆＊＠", "+-=<>\\$%#&*@"},
		{"０１２３４５６７８９", "0123456789"},
		{"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ", "ABCDEFGHIJKLMNOPQRSTUVWXYZ"},
		{"ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ", "abcdefghijklmnopqrstuvwxyz"},
		{"ァアィイゥウェエォオカキクケコサシスセソタチッツテトナニヌネノハヒフヘホマミムメモャヤュユョヨラリルレロワヲン", "ｧｱｨｲｩｳｪｴｫｵｶｷｸｹｺｻｼｽｾｿﾀﾁｯﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓｬﾔｭﾕｮﾖﾗﾘﾙﾚﾛﾜｦﾝ"},
		{NULL, NULL},
	};
	const char unconvertable[] =
		"´¨ヽヾゝゞ〃仝々〆〇―‐＼～∥…‥‘“〔〕〈〉《》『』【】±×÷≠≦≧"
		"∞∴♂♀°′″℃￠￡§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓∈∋⊆⊇"
		"⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭♪†‡¶◯"
		"ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっ"
		"つづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆ"
		"ょよらりるれろゎわゐゑをんゔガギグゲゴザジズゼゾダヂヅデドバパビピブプ"
		"ベペボポヮヰヱヴヵヶΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩα"
		"βγδεζηθικλμνξοπρστυφχψω";
	for (const Zen2hanTestCase *tc = testcases; tc->zen; tc++) {
		ASSERT_STRCMP(sjis2utf(zen2han(utf2sjis(tc->zen), SHIFT_JIS)), tc->han);
		ASSERT_STRCMP(zen2han(tc->zen, UTF8), tc->han);
	}
	ASSERT_STRCMP(sjis2utf(zen2han(utf2sjis(unconvertable), SHIFT_JIS)), unconvertable);
	ASSERT_STRCMP(zen2han(unconvertable, UTF8), unconvertable);
}

static void format_number_test(void) {
	char buf[256];
	ASSERT_STRCMP(format_number(0, 0, buf), "0");
	ASSERT_STRCMP(format_number(42, 0, buf), "42");
	ASSERT_STRCMP(format_number(0, 3, buf), "  0");
	ASSERT_STRCMP(format_number(42, 3, buf), " 42");
	ASSERT_STRCMP(format_number(65535, 3, buf), "535");
}

static void format_number_zenkaku_test(void) {
	char buf[512];
	ASSERT_STRCMP(sjis2utf(format_number_zenkaku(0, 0, buf)), "０");
	ASSERT_STRCMP(sjis2utf(format_number_zenkaku(42, 0, buf)), "４２");
	ASSERT_STRCMP(sjis2utf(format_number_zenkaku(0, 3, buf)), "　　０");
	ASSERT_STRCMP(sjis2utf(format_number_zenkaku(42, 3, buf)), "　４２");
	ASSERT_STRCMP(sjis2utf(format_number_zenkaku(65535, 3, buf)), "５３５");
}

void hankaku_test(void) {
	zen2han_test();
	format_number_test();
	format_number_zenkaku_test();
}
