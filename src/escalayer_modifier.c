#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sdl_compat.h"
#include "cJSON.h"
#include "nact.h"
#include "utfsjis.h"
#include "variable.h"
#include "escalayer_modifier.h"

#define ESC_LOCK_HP_FULL     (1 << 0)
#define ESC_LOCK_LIBIDO_MAX  (1 << 1)
#define ESC_LOCK_ONEHIT      (1 << 2)
#define ESC_LOCK_LOVE_MAX    (1 << 3)

enum s35_mod_game {
	S35_GAME_NONE = 0,
	S35_GAME_ESCALAYER,
	S35_GAME_DAIAKUJI,
	S35_GAME_RANCE5D,
	S35_GAME_TSUMAMIGUI,
	S35_GAME_KICHIKUOU,
	S35_GAME_KAERUNYO,
};

static pthread_mutex_t live_state_lock = PTHREAD_MUTEX_INITIALIZER;
static char *live_state_json;
static bool published;
static uint32_t last_publish_tick;

char *xsystem35_modifier_live_state(void) {
	pthread_mutex_lock(&live_state_lock);
#ifdef ALICEHUB_IOS
	char *out = live_state_json ? strdup(live_state_json) : NULL;
#else
	char *out = strdup(live_state_json ? live_state_json : "{}");
#endif
	pthread_mutex_unlock(&live_state_lock);
	return out;
}

static void publish_live_state_json(char *out) {
	pthread_mutex_lock(&live_state_lock);
	char *old = live_state_json;
	live_state_json = out;
	pthread_mutex_unlock(&live_state_lock);
	free(old);
}

static pthread_mutex_t action_lock = PTHREAD_MUTEX_INITIALIZER;
static int esc_live_locks = 0;
#define MAX_PENDING_ACTIONS 16
static char pending_actions[MAX_PENDING_ACTIONS][64];
static int pending_action_count = 0;

static pthread_mutex_t esc_cache_lock = PTHREAD_MUTEX_INITIALIZER;
static struct {
	char **ain_ptr;
	int varnum;
	const char *sco_fname;
	bool checked;
	enum s35_mod_game game;
	/* Escalayer */
	int hp, hp_max;
	int atk, def, rsp;
	int libido, libido_max;
	int d2, love, training, peace;
	int enemy_hp, enemy_hp_max;
	int clear_count, ed_flags, cg_seen;
	int easy_menu, easy_menu2, hard_menu, hissatsu_flags;
	int recall_vars[16];
	int recall_count;
	/* Daiakuji */
	int da_gold, da_debt, da_debt_monthly, da_subsidy;
	int da_hp, da_hp_max, da_loyalty, da_unit_ctrl, da_unit_cnt, da_unit_max;
	int da_security, da_popular, da_item_equip, da_sit;
	/* Rance 5D */
	int r5d_cur_hp, r5d_hp0, r5d_atk0, r5d_def0, r5d_mag0, r5d_res0, r5d_eva0;
	int r5d_actions, r5d_goldbar, r5d_exporb, r5d_durability;
	int r5d_love_vars[4];
	/* Tsumamigui */
	int tm1_stamina, tm1_kanae, tm1_chiho, tm1_aibu, tm1_paizuri, tm1_fera, tm1_honban, tm1_lv, tm1_exp;
} esc_cache;

static bool str_contains_icase(const char *hay, const char *needle) {
	if (!hay || !needle || !needle[0])
		return false;
	size_t nlen = strlen(needle);
	for (const char *p = hay; *p; p++) {
		size_t i = 0;
		while (i < nlen && p[i] && tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i]))
			i++;
		if (i == nlen)
			return true;
	}
	return false;
}

static enum s35_mod_game detect_sco_game(const char *sco, const char *catalog_id) {
	if (!sco)
		return S35_GAME_NONE;
	if (catalog_id && strcmp(catalog_id, "kichikuou") == 0)
		return S35_GAME_KICHIKUOU;
	if (catalog_id && strcmp(catalog_id, "kaerunyo") == 0)
		return S35_GAME_KAERUNYO;
	if (str_contains_icase(sco, "kichiku") || strstr(sco, "鬼畜王"))
		return S35_GAME_KICHIKUOU;
	if (str_contains_icase(sco, "kaeru") || str_contains_icase(sco, "panyo") ||
	    strstr(sco, "ぱにょ") || strstr(sco, "\xe3\x81\xaf\xe3\x82\x9a\xe3\x81\xab\xe3\x82\x87") ||
	    strstr(sco, "\x82\xcf\x82\xc9\x82\xe5") ||
	    strstr(sco, "\xa4\xd1\xa4\xcb\xa4\xe7"))
		return S35_GAME_KAERUNYO;
	return S35_GAME_NONE;
}

static int find_var_names(const char *sjis_name, const char *utf8_name) {
	if (!nact || nact->ain.varnum <= 0 || !nact->ain.var)
		return -1;
	uint8_t *gbk_name = utf8_name ? utf2gbk((const uint8_t *)utf8_name) : NULL;
	int found = -1;
	for (int i = 0; i < nact->ain.varnum; i++) {
		const char *n = v_name(i);
		if (!n)
			continue;
		if ((sjis_name && strcmp(n, sjis_name) == 0) ||
			(utf8_name && strcmp(n, utf8_name) == 0) ||
			(gbk_name && strcmp(n, (const char *)gbk_name) == 0)) {
			found = i;
			break;
		}
	}
	free(gbk_name);
	if (found >= 0 || !utf8_name)
		return found;
	/* Fallback for GBK VARI tables whose codepoints round-trip via gbk2utf */
	for (int i = 0; i < nact->ain.varnum; i++) {
		const char *n = v_name(i);
		if (!n)
			continue;
		uint8_t *u = gbk2utf((const uint8_t *)n);
		if (u) {
			bool eq = strcmp((const char *)u, utf8_name) == 0;
			free(u);
			if (eq)
				return i;
		}
	}
	return -1;
}

static void reset_cache_indices_locked(void) {
	memset(&esc_cache, 0, sizeof(esc_cache));
	esc_cache.hp = esc_cache.hp_max = esc_cache.atk = esc_cache.def = esc_cache.rsp = -1;
	esc_cache.libido = esc_cache.libido_max = esc_cache.d2 = esc_cache.love = -1;
	esc_cache.training = esc_cache.peace = esc_cache.enemy_hp = esc_cache.enemy_hp_max = -1;
	esc_cache.clear_count = esc_cache.ed_flags = esc_cache.cg_seen = -1;
	esc_cache.easy_menu = esc_cache.easy_menu2 = esc_cache.hard_menu = esc_cache.hissatsu_flags = -1;
	esc_cache.da_gold = esc_cache.da_debt = esc_cache.da_debt_monthly = esc_cache.da_subsidy = -1;
	esc_cache.da_hp = esc_cache.da_hp_max = esc_cache.da_loyalty = esc_cache.da_unit_ctrl = -1;
	esc_cache.da_unit_cnt = esc_cache.da_unit_max = -1;
	esc_cache.da_security = esc_cache.da_popular = esc_cache.da_item_equip = esc_cache.da_sit = -1;
	esc_cache.r5d_cur_hp = esc_cache.r5d_hp0 = esc_cache.r5d_atk0 = esc_cache.r5d_def0 = -1;
	esc_cache.r5d_mag0 = esc_cache.r5d_res0 = esc_cache.r5d_eva0 = -1;
	esc_cache.r5d_actions = esc_cache.r5d_goldbar = esc_cache.r5d_exporb = esc_cache.r5d_durability = -1;
	for (int i = 0; i < 4; i++) esc_cache.r5d_love_vars[i] = -1;
	esc_cache.tm1_stamina = esc_cache.tm1_kanae = esc_cache.tm1_chiho = -1;
	esc_cache.tm1_aibu = esc_cache.tm1_paizuri = esc_cache.tm1_fera = esc_cache.tm1_honban = -1;
	esc_cache.tm1_lv = esc_cache.tm1_exp = -1;
}

static void refresh_escalayer_cache_locked(void) {
	if (!nact)
		return;
	const char *sco0 = (nact->files.cnt[DRIFILE_SCO] > 0) ? nact->files.game_fname[DRIFILE_SCO][0] : NULL;
	if (esc_cache.checked && esc_cache.ain_ptr == nact->ain.var &&
	    esc_cache.varnum == nact->ain.varnum && esc_cache.sco_fname == sco0)
		return;

	reset_cache_indices_locked();
	esc_cache.ain_ptr = nact->ain.var;
	esc_cache.varnum = nact->ain.varnum;
	esc_cache.sco_fname = sco0;

	if (nact->ain.varnum > 0 && nact->ain.var) {
		/* 1. Escalayer */
		esc_cache.hp = find_var_names("\x81\x99\x82\x67\x82\x6f", "☆ＨＰ");
		esc_cache.hp_max = find_var_names("\x81\x99\x82\x67\x82\x6f\x82\x6c\x82\x60\x82\x77", "☆ＨＰＭＡＸ");
		esc_cache.atk = find_var_names("\x81\x99\x8d\x55\x8c\x82\x97\xcd", "☆攻撃力");
		esc_cache.def = find_var_names("\x81\x99\x96\x68\x8c\xe4\x97\xcd", "☆防御力");
		esc_cache.rsp = find_var_names("\x81\x99\x94\xbd\x89\x9e\x97\xcd", "☆反応力");
		esc_cache.libido = find_var_names("\x81\x99\x83\x8a\x83\x72\x83\x68\x81\x5b", "☆リビドー");
		esc_cache.libido_max = find_var_names("\x81\x99\x83\x8a\x83\x72\x83\x68\x81\x5b\x82\x6c\x82\x60\x82\x77", "☆リビドーＭＡＸ");
		esc_cache.d2 = find_var_names("\x81\x99\x82\x63\x82\x51", "☆Ｄ２");
		esc_cache.love = find_var_names("\x81\x99\x88\xa4\x8f\xee", "☆愛情");
		esc_cache.training = find_var_names("\x81\x99\x92\xb2\x8b\xb3", "☆調教");
		esc_cache.peace = find_var_names("\x81\x99\x83\x73\x81\x5b\x83\x58", "☆ピース");
		esc_cache.enemy_hp = find_var_names("\x81\x99\x93\x47\x81\x5e\x8c\xbb\x91\xcc\x97\xcd", "☆敵／現体力");
		esc_cache.enemy_hp_max = find_var_names("\x81\x99\x93\x47\x81\x5e\x91\x8d\x91\xcc\x97\xcd", "☆敵／総体力");
		esc_cache.clear_count = find_var_names("\x81\xa6\x83\x4e\x83\x8a\x83\x41\x89\xf1\x90\x94", "※クリア回数");
		esc_cache.easy_menu = find_var_names("\x81\xa6\x83\x43\x81\x5b\x83\x57\x81\x5b\x83\x81\x83\x6a\x83\x85\x81\x5b", "※イージーメニュー");
		esc_cache.easy_menu2 = find_var_names("\x81\xa6\x83\x43\x81\x5b\x83\x57\x81\x5b\x83\x81\x83\x6a\x83\x85\x81\x5b\x82\x51", "※イージーメニュー２");
		esc_cache.hard_menu = find_var_names("\x81\xa6\x83\x6e\x81\x5b\x83\x68\x83\x81\x83\x6a\x83\x85\x81\x5b", "※ハードメニュー");
		esc_cache.ed_flags = find_var_names("\x81\xa6\x82\x64\x82\x63", "※ＥＤ");
		esc_cache.hissatsu_flags = find_var_names("\x81\xa6\x95\x4b\x8e\x45\x8b\x5a", "※必殺技");
		esc_cache.cg_seen = find_var_names("\x81\xa6\x82\x62\x82\x66\x8c\xa9\x82\xbd", "※ＣＧ見た");
		if (esc_cache.hp >= 0 && esc_cache.libido >= 0 && esc_cache.d2 >= 0) {
			esc_cache.game = S35_GAME_ESCALAYER;
			for (int i = 0; i < nact->ain.varnum && esc_cache.recall_count < 16; i++) {
				const char *n = v_name(i);
				if (!n) continue;
				bool is_recall = strncmp(n, "\x81\xa6\x89\xf1\x91\x7a\x81\x5e", 8) == 0 ||
					strncmp(n, "※回想／", strlen("※回想／")) == 0;
				if (!is_recall) {
					uint8_t *u = gbk2utf((const uint8_t *)n);
					if (u) {
						is_recall = strncmp((const char *)u, "※回想／", strlen("※回想／")) == 0;
						free(u);
					}
				}
				if (is_recall)
					esc_cache.recall_vars[esc_cache.recall_count++] = i;
			}
			goto done;
		}

		/* 2. Daiakuji */
		esc_cache.da_gold = find_var_names("GOLD", "GOLD");
		esc_cache.da_debt = find_var_names("\x81\x99\x8e\xd8\x8b\xe0", "☆借金");
		esc_cache.da_debt_monthly = find_var_names("\x81\x99\x8c\x8e\x95\xa5\x82\xa2", "☆月払い");
		esc_cache.da_subsidy = find_var_names("\x81\x99\x95\xe2\x8f\x95\x8b\xe0", "☆補助金");
		esc_cache.da_hp = find_var_names("B\x91\xcc\x97\xcd", "B体力");
		esc_cache.da_hp_max = find_var_names("B\x8d\xc5\x91\xe5\x91\xcc\x97\xcd", "B最大体力");
		esc_cache.da_loyalty = find_var_names("B\x92\x89\x90\xbd\x90\x53", "B忠誠心");
		esc_cache.da_unit_ctrl = find_var_names("B\x90\xa7\x8c\xe4", "B制御");
		esc_cache.da_unit_cnt = find_var_names("G\x95\x94\x89\xba\x90\x94", "G部下数");
		esc_cache.da_unit_max = find_var_names("G\x95\x94\x89\xba\x8d\xc5\x91\xe5\x90\x94", "G部下最大数");
		esc_cache.da_security = find_var_names("R\x8e\xa1\x88\xc0\x83\x8c\x83\x78\x83\x8b", "R治安レベル");
		esc_cache.da_popular = find_var_names("\x81\x99\x83\x4d\x83\x83\x83\x8b\x83\x6f\x81\x5b\x83\x4b\x81\x5b\x90\x6c\x8b\x43", "☆ギャルバーガー人気");
		esc_cache.da_item_equip = find_var_names("I\x91\x95\x94\xf5", "I装備");
		esc_cache.da_sit = find_var_names("SIT", "SIT");
		if (esc_cache.da_gold >= 0 && esc_cache.da_debt >= 0 && esc_cache.da_hp >= 0) {
			esc_cache.game = S35_GAME_DAIAKUJI;
			goto done;
		}

		/* 3. Rance 5D */
		esc_cache.r5d_cur_hp = find_var_names("\x81\x99\x8c\xbb\x91\xcc\x97\xcd", "☆現体力");
		esc_cache.r5d_hp0 = find_var_names("\x81\x99\x91\xcc\x97\xcd\x82\x4f", "☆体力０");
		esc_cache.r5d_atk0 = find_var_names("\x81\x99\x8d\x55\x8c\x82\x97\xcd\x82\x4f", "☆攻撃力０");
		esc_cache.r5d_def0 = find_var_names("\x81\x99\x96\x68\x8c\xe4\x97\xcd\x82\x4f", "☆防御力０");
		esc_cache.r5d_mag0 = find_var_names("\x81\x99\x96\x82\x96\x40\x97\xcd\x82\x4f", "☆魔法力０");
		esc_cache.r5d_res0 = find_var_names("\x81\x99\x96\x82\x92\xef\x97\xcd\x82\x4f", "☆魔抵力０");
		esc_cache.r5d_eva0 = find_var_names("\x81\x99\x89\xf1\x94\xf0\x97\xcd\x82\x4f", "☆回避力０");
		esc_cache.r5d_actions = find_var_names("\x81\x99\x96\xc0\x8b\x7b\x8d\x73\x93\xae\x89\xf1\x90\x94", "☆迷宮行動回数");
		esc_cache.r5d_goldbar = find_var_names("\x81\x99\x82\x6a\x8b\xe0\x89\xf2", "☆Ｋ金塊");
		esc_cache.r5d_exporb = find_var_names("\x81\x99\x82\x6a\x92\xb4\x8c\x6f\x8c\xb1\x8b\xca", "☆Ｋ超経験玉");
		esc_cache.r5d_durability = find_var_names("\x81\x99\x8e\x9d\x82\xbf\x95\xa8\x81\x5e\x91\xcf\x8b\x76\x93\x78", "☆持ち物／耐久度");
		esc_cache.r5d_love_vars[0] = find_var_names("\x81\x99\x8a\x43\x91\xdb\x8e\x71\x81\x5e\x88\xa4\x82\x6f", "☆海苔子／愛Ｐ");
		esc_cache.r5d_love_vars[1] = find_var_names("\x81\x99\x82\xa0\x82\xc4\x82\xc8\x81\x5e\x88\xa4\x82\x6f", "☆あてな／愛Ｐ");
		esc_cache.r5d_love_vars[2] = find_var_names("\x81\x99\x83\x8a\x83\x59\x83\x69\x81\x5e\x88\xa4\x82\x6f", "☆リズナ／愛Ｐ");
		esc_cache.r5d_love_vars[3] = find_var_names("\x81\x99\x83\x52\x83\x70\x83\x93\x83\x68\x83\x93\x81\x5e\x88\xa4\x82\x6f", "☆コパンドン／愛Ｐ");
		if (esc_cache.r5d_cur_hp >= 0 && esc_cache.r5d_actions >= 0 && esc_cache.r5d_goldbar >= 0) {
			esc_cache.game = S35_GAME_RANCE5D;
			goto done;
		}

		/* 4. Tsumamigui 1 */
		esc_cache.tm1_stamina = find_var_names("\x91\xcf\x8b\x76\x97\xcd", "耐久力");
		esc_cache.tm1_kanae = find_var_names("\x8d\x81\x95\x63\x83\x8c\x83\x78\x83\x8b", "香苗レベル");
		esc_cache.tm1_chiho = find_var_names("\x90\xe7\x95\xe4\x83\x8c\x83\x78\x83\x8b", "千穂レベル");
		esc_cache.tm1_aibu = find_var_names("\x88\xa4\x95\x8f\x83\x8c\x83\x78\x83\x8b", "愛撫レベル");
		esc_cache.tm1_paizuri = find_var_names("\x83\x70\x83\x43\x83\x59\x83\x8a\x83\x8c\x83\x78\x83\x8b", "パイズリレベル");
		esc_cache.tm1_fera = find_var_names("\x83\x74\x83\x46\x83\x89\x83\x8c\x83\x78\x83\x8b", "フェラレベル");
		esc_cache.tm1_honban = find_var_names("\x96\x7b\x94\xd4\x83\x8c\x83\x78\x83\x8b", "本番レベル");
		esc_cache.tm1_lv = find_var_names("\x83\x8c\x83\x78\x83\x8b", "レベル");
		esc_cache.tm1_exp = find_var_names("EXP", "EXP");
		if (esc_cache.tm1_stamina >= 0 && esc_cache.tm1_kanae >= 0 && esc_cache.tm1_chiho >= 0) {
			esc_cache.game = S35_GAME_TSUMAMIGUI;
			goto done;
		}
	}

	/* 5 & 6. These games can use generic scenario archives. */
	esc_cache.game = detect_sco_game(sco0, SDL_getenv("XSYSTEM35_MODIFIER_GAME"));

done:
	esc_cache.checked = true;
}

static const char *game_id_str(enum s35_mod_game g) {
	switch (g) {
	case S35_GAME_ESCALAYER:  return "escalayer";
	case S35_GAME_DAIAKUJI:   return "daiakuji";
	case S35_GAME_RANCE5D:    return "rance5d";
	case S35_GAME_TSUMAMIGUI: return "tsumamigui";
	case S35_GAME_KICHIKUOU:  return "kichikuou";
	case S35_GAME_KAERUNYO:   return "kaerunyo";
	default:                  return "";
	}
}

static inline bool esc_valid_var(int idx) {
	return idx >= 0 && idx < 65537 && (nact->ain.varnum <= 0 || idx < nact->ain.varnum);
}

static vmvar_t clamp_u16(int v) {
	if (v < 0) return 0;
	if (v > 65535) return 65535;
	return (vmvar_t)v;
}

static void esc_unlock_var_all(int var) {
	if (!esc_valid_var(var))
		return;
	struct VarRef ref;
	if (v_ref_indexed(var, 0, &ref) && ref.page > 0 && ref.page < 256 && varPage[ref.page].value) {
		int len = v_boundSliceSize(&ref);
		for (int i = ref.index; i < ref.index + len; i++) {
			if (varPage[ref.page].value[i] == 0)
				varPage[ref.page].value[i] = 1;
		}
	} else {
		if (sysVar[var] == 0)
			sysVar[var] = 1;
	}
}

static int esc_count_unlocked(int var) {
	if (!esc_valid_var(var))
		return 0;
	struct VarRef ref;
	if (v_ref_indexed(var, 0, &ref) && ref.page > 0 && ref.page < 256 && varPage[ref.page].value) {
		int len = v_boundSliceSize(&ref);
		int cnt = 0;
		for (int i = ref.index; i < ref.index + len; i++) {
			if (varPage[ref.page].value[i] != 0)
				cnt++;
		}
		return cnt;
	}
	return sysVar[var];
}

static int s35_read_first(int var) {
	if (!esc_valid_var(var))
		return 0;
	struct VarRef ref;
	if (v_ref_indexed(var, 0, &ref) && ref.page > 0 && ref.page < 256 && varPage[ref.page].value)
		return varPage[ref.page].value[ref.index];
	return sysVar[var];
}

static void s35_set_var_min_all(int var, vmvar_t min_val, bool nonzero_only) {
	if (!esc_valid_var(var))
		return;
	struct VarRef ref;
	if (v_ref_indexed(var, 0, &ref) && ref.page > 0 && ref.page < 256 && varPage[ref.page].value) {
		int len = v_boundSliceSize(&ref);
		if (len > 512) len = 512;
		for (int i = ref.index; i < ref.index + len; i++) {
			vmvar_t cur = varPage[ref.page].value[i];
			if ((!nonzero_only || cur > 0) && cur < min_val)
				varPage[ref.page].value[i] = min_val;
		}
	} else {
		if ((!nonzero_only || sysVar[var] > 0) && sysVar[var] < min_val)
			sysVar[var] = min_val;
	}
}

static void s35_copy_array_or_scalar(int dst_var, int src_var) {
	if (!esc_valid_var(dst_var) || !esc_valid_var(src_var))
		return;
	struct VarRef rd, rs;
	if (v_ref_indexed(dst_var, 0, &rd) && rd.page > 0 && rd.page < 256 && varPage[rd.page].value &&
	    v_ref_indexed(src_var, 0, &rs) && rs.page > 0 && rs.page < 256 && varPage[rs.page].value) {
		int len = v_boundSliceSize(&rd);
		int slen = v_boundSliceSize(&rs);
		if (slen < len) len = slen;
		if (len > 512) len = 512;
		for (int i = 0; i < len; i++) {
			vmvar_t mv = varPage[rs.page].value[rs.index + i];
			if (mv > 0 && varPage[rd.page].value[rd.index + i] < mv)
				varPage[rd.page].value[rd.index + i] = mv;
		}
	} else {
		vmvar_t mv = sysVar[src_var];
		if (mv > 0 && sysVar[dst_var] < mv)
			sysVar[dst_var] = mv;
	}
}

/*
 * Daiakuji unit arrays (B体力/B最大体力/B忠誠心/B制御) share dedicated varPages
 * across all organizations, with 組織設定 dynamically rebinding DS offset to
 * 部下位置. Org 1 (the player's faction) always occupies 1-based indices
 * [1 .. G部下数[1]] at offset 0 of each backing page.
 */
static struct VarPage *da_bound_page(int var) {
	if (!esc_valid_var(var))
		return NULL;
	struct VarRef ref;
	if (!v_ref_indexed(var, 0, &ref) || ref.page <= 0 || ref.page >= 256 || !varPage[ref.page].value)
		return NULL;
	return &varPage[ref.page];
}

static int da_ally_count(void) {
	struct VarPage *cnt_pg = da_bound_page(esc_cache.da_unit_cnt);
	int cnt = (cnt_pg && cnt_pg->size > 1) ? cnt_pg->value[1] : 0;
	struct VarPage *max_pg = da_bound_page(esc_cache.da_unit_max);
	int cap = (max_pg && max_pg->size > 1 && max_pg->value[1] > 0) ? max_pg->value[1] : 60;
	if (cnt > cap)
		cnt = cap;
	return cnt;
}

static bool da_ally_active(const struct VarPage *ctrl_pg, const struct VarPage *hp_max_pg, int i) {
	if (ctrl_pg && i < ctrl_pg->size && ctrl_pg->value[i] == 0)
		return false;
	if (hp_max_pg && i < hp_max_pg->size && hp_max_pg->value[i] == 0)
		return false;
	return true;
}

static void da_clear_debt(void) {
	if (esc_valid_var(esc_cache.da_debt))
		sysVar[esc_cache.da_debt] = 0;
	if (esc_valid_var(esc_cache.da_debt_monthly))
		sysVar[esc_cache.da_debt_monthly] = 0;
}

static void da_boost_ally_hp(bool raise_max) {
	struct VarPage *hp_pg = da_bound_page(esc_cache.da_hp);
	struct VarPage *hp_max_pg = da_bound_page(esc_cache.da_hp_max);
	struct VarPage *ctrl_pg = da_bound_page(esc_cache.da_unit_ctrl);
	if (!hp_pg)
		return;
	int cnt = da_ally_count();
	for (int i = 1; i <= cnt && i < hp_pg->size; i++) {
		if (!da_ally_active(ctrl_pg, hp_max_pg, i))
			continue;
		if (hp_max_pg && i < hp_max_pg->size) {
			if (raise_max && hp_max_pg->value[i] < 999)
				hp_max_pg->value[i] = 999;
			vmvar_t target = hp_max_pg->value[i];
			if (target > 0 && hp_pg->value[i] < target)
				hp_pg->value[i] = target;
		} else if (hp_pg->value[i] > 0 && hp_pg->value[i] < 999) {
			hp_pg->value[i] = 999;
		}
	}
}

static void da_max_loyalty_and_popular(void) {
	struct VarPage *loy_pg = da_bound_page(esc_cache.da_loyalty);
	struct VarPage *ctrl_pg = da_bound_page(esc_cache.da_unit_ctrl);
	struct VarPage *hp_max_pg = da_bound_page(esc_cache.da_hp_max);
	if (loy_pg) {
		int cnt = da_ally_count();
		for (int i = 1; i <= cnt && i < loy_pg->size; i++) {
			if (!da_ally_active(ctrl_pg, hp_max_pg, i))
				continue;
			if (loy_pg->value[i] < 99)
				loy_pg->value[i] = 99;
		}
	}
	if (esc_valid_var(esc_cache.da_popular) && sysVar[esc_cache.da_popular] < 100)
		sysVar[esc_cache.da_popular] = 100;
}

static void da_restore_security(void) {
	/* R治安レベル: 0 = 良好 (best), 1 = 普通, 2 = 恶化 (half rev), 3 = 无法 (0 rev) */
	struct VarPage *sec_pg = da_bound_page(esc_cache.da_security);
	if (!sec_pg)
		return;
	int end = sec_pg->size;
	if (end > 38)
		end = 38;
	for (int i = 1; i < end; i++)
		sec_pg->value[i] = 0;
}

static void da_unlock_all_items(void) {
	/* I装備[1..48]: 99 = unowned, 0 = owned in inventory, 1..98 = equipped on ally #n */
	struct VarPage *eq_pg = da_bound_page(esc_cache.da_item_equip);
	if (eq_pg) {
		for (int i = 1; i <= 48 && i < eq_pg->size; i++) {
			if (eq_pg->value[i] == 99)
				eq_pg->value[i] = 0;
		}
	}
	/* SIT[1..48]: Special Item obtained/gallery flags */
	struct VarPage *sit_pg = da_bound_page(esc_cache.da_sit);
	if (sit_pg) {
		for (int i = 1; i <= 48 && i < sit_pg->size; i++) {
			if (sit_pg->value[i] == 0)
				sit_pg->value[i] = 1;
		}
	}
}

static void apply_single_action_locked(const char *spec) {
	refresh_escalayer_cache_locked();
	if (esc_cache.game == S35_GAME_NONE || !spec || !spec[0])
		return;

	if (esc_cache.game == S35_GAME_ESCALAYER) {
		if (strcmp(spec, "esc_heal") == 0) {
			if (esc_valid_var(esc_cache.hp) && esc_valid_var(esc_cache.hp_max)) {
				uint16_t maxv = sysVar[esc_cache.hp_max];
				if (maxv > 0) sysVar[esc_cache.hp] = maxv;
			}
		} else if (strcmp(spec, "esc_libido_max") == 0) {
			if (esc_valid_var(esc_cache.libido) && esc_valid_var(esc_cache.libido_max)) {
				uint16_t maxv = sysVar[esc_cache.libido_max];
				if (maxv > 0) sysVar[esc_cache.libido] = maxv;
			}
		} else if (strcmp(spec, "esc_enemy_onehit") == 0) {
			if (esc_valid_var(esc_cache.enemy_hp) && sysVar[esc_cache.enemy_hp] > 1)
				sysVar[esc_cache.enemy_hp] = 1;
		} else if (strcmp(spec, "esc_love_max") == 0) {
			if (esc_valid_var(esc_cache.love)) sysVar[esc_cache.love] = clamp_u16(sysVar[esc_cache.love] < 100 ? 100 : sysVar[esc_cache.love] + 50);
			if (esc_valid_var(esc_cache.training)) sysVar[esc_cache.training] = clamp_u16(sysVar[esc_cache.training] + 50);
		} else if (strcmp(spec, "esc_peace_100") == 0) {
			if (esc_valid_var(esc_cache.peace)) sysVar[esc_cache.peace] = 100;
		} else if (strcmp(spec, "esc_stats_boost") == 0) {
			if (esc_valid_var(esc_cache.hp_max) && sysVar[esc_cache.hp_max] < 9999)
				sysVar[esc_cache.hp_max] = 9999;
			if (esc_valid_var(esc_cache.hp) && esc_valid_var(esc_cache.hp_max))
				sysVar[esc_cache.hp] = sysVar[esc_cache.hp_max];
			if (esc_valid_var(esc_cache.atk)) sysVar[esc_cache.atk] = clamp_u16(sysVar[esc_cache.atk] + 300);
			if (esc_valid_var(esc_cache.def)) sysVar[esc_cache.def] = clamp_u16(sysVar[esc_cache.def] + 300);
			if (esc_valid_var(esc_cache.rsp)) sysVar[esc_cache.rsp] = clamp_u16(sysVar[esc_cache.rsp] + 100);
		} else if (strcmp(spec, "esc_unlock_gallery") == 0) {
			esc_unlock_var_all(esc_cache.clear_count);
			esc_unlock_var_all(esc_cache.easy_menu);
			esc_unlock_var_all(esc_cache.easy_menu2);
			esc_unlock_var_all(esc_cache.hard_menu);
			esc_unlock_var_all(esc_cache.ed_flags);
			esc_unlock_var_all(esc_cache.hissatsu_flags);
			esc_unlock_var_all(esc_cache.cg_seen);
			for (int i = 0; i < esc_cache.recall_count; i++)
				esc_unlock_var_all(esc_cache.recall_vars[i]);
		} else if (strncmp(spec, "esc_atk_add=", 12) == 0 && esc_valid_var(esc_cache.atk)) {
			sysVar[esc_cache.atk] = clamp_u16(sysVar[esc_cache.atk] + atoi(spec + 12));
		} else if (strncmp(spec, "esc_def_add=", 12) == 0 && esc_valid_var(esc_cache.def)) {
			sysVar[esc_cache.def] = clamp_u16(sysVar[esc_cache.def] + atoi(spec + 12));
		} else if (strncmp(spec, "esc_rsp_add=", 12) == 0 && esc_valid_var(esc_cache.rsp)) {
			sysVar[esc_cache.rsp] = clamp_u16(sysVar[esc_cache.rsp] + atoi(spec + 12));
		} else if (strncmp(spec, "esc_d2_set=", 11) == 0 && esc_valid_var(esc_cache.d2)) {
			sysVar[esc_cache.d2] = clamp_u16(atoi(spec + 11));
		}
		return;
	}

	if (esc_cache.game == S35_GAME_DAIAKUJI) {
		if (strncmp(spec, "da_gold_add=", 12) == 0 && esc_valid_var(esc_cache.da_gold)) {
			sysVar[esc_cache.da_gold] = clamp_u16(sysVar[esc_cache.da_gold] + atoi(spec + 12));
		} else if (strcmp(spec, "da_gold_max") == 0 && esc_valid_var(esc_cache.da_gold)) {
			sysVar[esc_cache.da_gold] = 60000;
		} else if (strcmp(spec, "da_debt_zero") == 0) {
			da_clear_debt();
		} else if (strcmp(spec, "da_hp_boost") == 0) {
			da_boost_ally_hp(true);
		} else if (strcmp(spec, "da_love_max") == 0) {
			da_max_loyalty_and_popular();
		} else if (strcmp(spec, "da_security_max") == 0) {
			da_restore_security();
		} else if (strcmp(spec, "da_items_unlock") == 0) {
			da_unlock_all_items();
		}
		return;
	}

	if (esc_cache.game == S35_GAME_RANCE5D) {
		if (strcmp(spec, "r5d_heal") == 0) {
			s35_copy_array_or_scalar(esc_cache.r5d_cur_hp, esc_cache.r5d_hp0);
		} else if (strcmp(spec, "r5d_stats_boost") == 0) {
			s35_set_var_min_all(esc_cache.r5d_hp0, 9999, true);
			s35_copy_array_or_scalar(esc_cache.r5d_cur_hp, esc_cache.r5d_hp0);
			s35_set_var_min_all(esc_cache.r5d_atk0, 999, true);
			s35_set_var_min_all(esc_cache.r5d_def0, 999, true);
			s35_set_var_min_all(esc_cache.r5d_mag0, 999, true);
			s35_set_var_min_all(esc_cache.r5d_res0, 999, true);
			s35_set_var_min_all(esc_cache.r5d_eva0, 99, true);
		} else if (strcmp(spec, "r5d_gold_exp") == 0) {
			if (esc_valid_var(esc_cache.r5d_goldbar)) sysVar[esc_cache.r5d_goldbar] = 99;
			if (esc_valid_var(esc_cache.r5d_exporb)) sysVar[esc_cache.r5d_exporb] = 99;
		} else if (strcmp(spec, "r5d_love_max") == 0) {
			for (int i = 0; i < 4; i++)
				if (esc_valid_var(esc_cache.r5d_love_vars[i]))
					sysVar[esc_cache.r5d_love_vars[i]] = 100;
		} else if (strcmp(spec, "r5d_durability") == 0) {
			s35_set_var_min_all(esc_cache.r5d_durability, 99, true);
		}
		return;
	}

	if (esc_cache.game == S35_GAME_TSUMAMIGUI) {
		if (strcmp(spec, "tm1_stamina_max") == 0 && esc_valid_var(esc_cache.tm1_stamina)) {
			sysVar[esc_cache.tm1_stamina] = 999;
		} else if (strcmp(spec, "tm1_tech_max") == 0) {
			if (esc_valid_var(esc_cache.tm1_aibu)) sysVar[esc_cache.tm1_aibu] = 10;
			if (esc_valid_var(esc_cache.tm1_paizuri)) sysVar[esc_cache.tm1_paizuri] = 10;
			if (esc_valid_var(esc_cache.tm1_fera)) sysVar[esc_cache.tm1_fera] = 10;
			if (esc_valid_var(esc_cache.tm1_honban)) sysVar[esc_cache.tm1_honban] = 10;
			if (esc_valid_var(esc_cache.tm1_lv)) sysVar[esc_cache.tm1_lv] = 10;
		} else if (strcmp(spec, "tm1_heroine_max") == 0) {
			if (esc_valid_var(esc_cache.tm1_kanae)) sysVar[esc_cache.tm1_kanae] = 10;
			if (esc_valid_var(esc_cache.tm1_chiho)) sysVar[esc_cache.tm1_chiho] = 10;
		} else if (strncmp(spec, "tm1_exp_add=", 12) == 0 && esc_valid_var(esc_cache.tm1_exp)) {
			sysVar[esc_cache.tm1_exp] = clamp_u16(sysVar[esc_cache.tm1_exp] + atoi(spec + 12));
		}
		return;
	}

	if (esc_cache.game == S35_GAME_KICHIKUOU) {
		if (strncmp(spec, "kr_gold_add=", 12) == 0) {
			sysVar[522] = clamp_u16(sysVar[522] + atoi(spec + 12));
		} else if (strcmp(spec, "kr_gold_max") == 0) {
			sysVar[522] = 60000;
		} else if (strcmp(spec, "kr_reset_actions") == 0) {
			s35_copy_array_or_scalar(90, 91);
		}
		return;
	}

	if (esc_cache.game == S35_GAME_KAERUNYO) {
		if (strncmp(spec, "kn_gold_add=", 12) == 0) {
			sysVar[171] = clamp_u16(sysVar[171] + atoi(spec + 12));
		} else if (strcmp(spec, "kn_gold_max") == 0) {
			sysVar[171] = 60000;
		} else if (strcmp(spec, "kn_items_max") == 0) {
			struct VarRef r0, r17;
			if (v_ref_indexed(399, 0, &r0) && r0.page > 0 && varPage[r0.page].value)
				varPage[r0.page].value[r0.index] = 99;
			if (v_ref_indexed(399, 17, &r17) && r17.page > 0 && varPage[r17.page].value)
				varPage[r17.page].value[r17.index] = 99;
		}
		return;
	}
}

static void apply_single_action_sdl_thread(const char *spec) {
	if (!spec || !spec[0])
		return;
	pthread_mutex_lock(&esc_cache_lock);
	apply_single_action_locked(spec);
	pthread_mutex_unlock(&esc_cache_lock);
}

static void apply_escalayer_locks_locked(int locks) {
	if (!locks)
		return;
	refresh_escalayer_cache_locked();
	if (esc_cache.game == S35_GAME_NONE)
		return;

	if (esc_cache.game == S35_GAME_ESCALAYER) {
		if ((locks & ESC_LOCK_HP_FULL) && esc_valid_var(esc_cache.hp) && esc_valid_var(esc_cache.hp_max)) {
			uint16_t m = sysVar[esc_cache.hp_max];
			if (m > 0 && sysVar[esc_cache.hp] < m)
				sysVar[esc_cache.hp] = m;
		}
		if ((locks & ESC_LOCK_LIBIDO_MAX) && esc_valid_var(esc_cache.libido) && esc_valid_var(esc_cache.libido_max)) {
			uint16_t m = sysVar[esc_cache.libido_max];
			if (m > 0 && sysVar[esc_cache.libido] < m)
				sysVar[esc_cache.libido] = m;
		}
		if ((locks & ESC_LOCK_ONEHIT) && esc_valid_var(esc_cache.enemy_hp)) {
			if (sysVar[esc_cache.enemy_hp] > 1)
				sysVar[esc_cache.enemy_hp] = 1;
		}
		if ((locks & ESC_LOCK_LOVE_MAX) && esc_valid_var(esc_cache.love)) {
			if (sysVar[esc_cache.love] < 100)
				sysVar[esc_cache.love] = 100;
		}
	} else if (esc_cache.game == S35_GAME_DAIAKUJI) {
		if ((locks & (1 << 0)) && esc_valid_var(esc_cache.da_gold) && sysVar[esc_cache.da_gold] < 50000)
			sysVar[esc_cache.da_gold] = 50000;
		if (locks & (1 << 1))
			da_clear_debt();
		if (locks & (1 << 2))
			da_boost_ally_hp(false);
		if (locks & (1 << 3))
			da_max_loyalty_and_popular();
	} else if (esc_cache.game == S35_GAME_RANCE5D) {
		if (locks & (1 << 0))
			s35_copy_array_or_scalar(esc_cache.r5d_cur_hp, esc_cache.r5d_hp0);
		if ((locks & (1 << 1)) && esc_valid_var(esc_cache.r5d_actions) && sysVar[esc_cache.r5d_actions] < 99)
			sysVar[esc_cache.r5d_actions] = 99;
		if (locks & (1 << 2))
			s35_set_var_min_all(esc_cache.r5d_durability, 99, true);
		if (locks & (1 << 3)) {
			if (esc_valid_var(esc_cache.r5d_goldbar) && sysVar[esc_cache.r5d_goldbar] < 99)
				sysVar[esc_cache.r5d_goldbar] = 99;
			if (esc_valid_var(esc_cache.r5d_exporb) && sysVar[esc_cache.r5d_exporb] < 99)
				sysVar[esc_cache.r5d_exporb] = 99;
		}
	} else if (esc_cache.game == S35_GAME_TSUMAMIGUI) {
		if ((locks & (1 << 0)) && esc_valid_var(esc_cache.tm1_stamina) && sysVar[esc_cache.tm1_stamina] < 999)
			sysVar[esc_cache.tm1_stamina] = 999;
		if (locks & (1 << 1)) {
			if (esc_valid_var(esc_cache.tm1_aibu) && sysVar[esc_cache.tm1_aibu] < 10) sysVar[esc_cache.tm1_aibu] = 10;
			if (esc_valid_var(esc_cache.tm1_paizuri) && sysVar[esc_cache.tm1_paizuri] < 10) sysVar[esc_cache.tm1_paizuri] = 10;
			if (esc_valid_var(esc_cache.tm1_fera) && sysVar[esc_cache.tm1_fera] < 10) sysVar[esc_cache.tm1_fera] = 10;
			if (esc_valid_var(esc_cache.tm1_honban) && sysVar[esc_cache.tm1_honban] < 10) sysVar[esc_cache.tm1_honban] = 10;
		}
	} else if (esc_cache.game == S35_GAME_KICHIKUOU) {
		if ((locks & (1 << 0)) && sysVar[522] < 5000)
			sysVar[522] = 5000;
		if (locks & (1 << 1))
			s35_copy_array_or_scalar(90, 91);
	} else if (esc_cache.game == S35_GAME_KAERUNYO) {
		if ((locks & (1 << 0)) && sysVar[171] < 50000)
			sysVar[171] = 50000;
		if (locks & (1 << 1)) {
			struct VarRef r0;
			if (v_ref_indexed(399, 0, &r0) && r0.page > 0 && varPage[r0.page].value &&
			    varPage[r0.page].value[r0.index] < 99)
				varPage[r0.page].value[r0.index] = 99;
		}
	}
}

static void apply_escalayer_locks_sdl_thread(int locks) {
	if (!locks)
		return;
	pthread_mutex_lock(&esc_cache_lock);
	apply_escalayer_locks_locked(locks);
	pthread_mutex_unlock(&esc_cache_lock);
}

static void update_live_state_sdl_thread(int locks) {
	uint32_t now = SDL_GetTicks();
	pthread_mutex_lock(&live_state_lock);
	if (published && (uint32_t)(now - last_publish_tick) < 250) {
		pthread_mutex_unlock(&live_state_lock);
		return;
	}
	published = true;
	last_publish_tick = now;
	pthread_mutex_unlock(&live_state_lock);

	pthread_mutex_lock(&esc_cache_lock);
	refresh_escalayer_cache_locked();
	if (esc_cache.game == S35_GAME_NONE) {
		pthread_mutex_unlock(&esc_cache_lock);
		publish_live_state_json(strdup("{\"game\":\"system3\",\"ready\":true}"));
		return;
	}

	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "game", game_id_str(esc_cache.game));
	cJSON_AddBoolToObject(root, "ready", true);
	cJSON_AddBoolToObject(root, "inGame", true);
	cJSON_AddNumberToObject(root, "locks", locks);

	if (esc_cache.game == S35_GAME_ESCALAYER) {
		if (esc_valid_var(esc_cache.hp)) cJSON_AddNumberToObject(root, "hp", sysVar[esc_cache.hp]);
		if (esc_valid_var(esc_cache.hp_max)) cJSON_AddNumberToObject(root, "hpMax", sysVar[esc_cache.hp_max]);
		if (esc_valid_var(esc_cache.atk)) cJSON_AddNumberToObject(root, "atk", sysVar[esc_cache.atk]);
		if (esc_valid_var(esc_cache.def)) cJSON_AddNumberToObject(root, "def", sysVar[esc_cache.def]);
		if (esc_valid_var(esc_cache.rsp)) cJSON_AddNumberToObject(root, "rsp", sysVar[esc_cache.rsp]);
		if (esc_valid_var(esc_cache.libido)) cJSON_AddNumberToObject(root, "libido", sysVar[esc_cache.libido]);
		if (esc_valid_var(esc_cache.libido_max)) cJSON_AddNumberToObject(root, "libidoMax", sysVar[esc_cache.libido_max]);
		if (esc_valid_var(esc_cache.d2)) cJSON_AddNumberToObject(root, "d2", sysVar[esc_cache.d2]);
		if (esc_valid_var(esc_cache.love)) cJSON_AddNumberToObject(root, "love", sysVar[esc_cache.love]);
		if (esc_valid_var(esc_cache.training)) cJSON_AddNumberToObject(root, "training", sysVar[esc_cache.training]);
		if (esc_valid_var(esc_cache.peace)) cJSON_AddNumberToObject(root, "peace", sysVar[esc_cache.peace]);
		if (esc_valid_var(esc_cache.enemy_hp)) cJSON_AddNumberToObject(root, "enemyHp", sysVar[esc_cache.enemy_hp]);
		if (esc_valid_var(esc_cache.enemy_hp_max)) cJSON_AddNumberToObject(root, "enemyHpMax", sysVar[esc_cache.enemy_hp_max]);
		if (esc_valid_var(esc_cache.clear_count)) cJSON_AddNumberToObject(root, "clearCount", sysVar[esc_cache.clear_count]);
		if (esc_valid_var(esc_cache.ed_flags)) cJSON_AddNumberToObject(root, "edFlags", esc_count_unlocked(esc_cache.ed_flags));
		if (esc_valid_var(esc_cache.cg_seen)) cJSON_AddNumberToObject(root, "cgSeen", esc_count_unlocked(esc_cache.cg_seen));
	} else if (esc_cache.game == S35_GAME_DAIAKUJI) {
		if (esc_valid_var(esc_cache.da_gold)) cJSON_AddNumberToObject(root, "gold", sysVar[esc_cache.da_gold]);
		if (esc_valid_var(esc_cache.da_debt)) cJSON_AddNumberToObject(root, "debt", sysVar[esc_cache.da_debt]);
		if (esc_valid_var(esc_cache.da_subsidy)) cJSON_AddNumberToObject(root, "subsidy", sysVar[esc_cache.da_subsidy]);
		if (esc_valid_var(esc_cache.da_popular)) cJSON_AddNumberToObject(root, "popular", sysVar[esc_cache.da_popular]);
		struct VarPage *hp_pg = da_bound_page(esc_cache.da_hp);
		if (hp_pg && hp_pg->size > 1 && da_ally_count() > 0)
			cJSON_AddNumberToObject(root, "hp", hp_pg->value[1]);
	} else if (esc_cache.game == S35_GAME_RANCE5D) {
		if (esc_valid_var(esc_cache.r5d_cur_hp)) cJSON_AddNumberToObject(root, "hp", s35_read_first(esc_cache.r5d_cur_hp));
		if (esc_valid_var(esc_cache.r5d_hp0)) cJSON_AddNumberToObject(root, "hpMax", s35_read_first(esc_cache.r5d_hp0));
		if (esc_valid_var(esc_cache.r5d_actions)) cJSON_AddNumberToObject(root, "ap", sysVar[esc_cache.r5d_actions]);
		if (esc_valid_var(esc_cache.r5d_goldbar)) cJSON_AddNumberToObject(root, "gold", sysVar[esc_cache.r5d_goldbar]);
		if (esc_valid_var(esc_cache.r5d_exporb)) cJSON_AddNumberToObject(root, "expOrb", sysVar[esc_cache.r5d_exporb]);
	} else if (esc_cache.game == S35_GAME_TSUMAMIGUI) {
		if (esc_valid_var(esc_cache.tm1_stamina)) cJSON_AddNumberToObject(root, "stamina", sysVar[esc_cache.tm1_stamina]);
		if (esc_valid_var(esc_cache.tm1_lv)) cJSON_AddNumberToObject(root, "lv", sysVar[esc_cache.tm1_lv]);
		if (esc_valid_var(esc_cache.tm1_exp)) cJSON_AddNumberToObject(root, "exp", sysVar[esc_cache.tm1_exp]);
		if (esc_valid_var(esc_cache.tm1_kanae)) cJSON_AddNumberToObject(root, "kanaeLv", sysVar[esc_cache.tm1_kanae]);
		if (esc_valid_var(esc_cache.tm1_chiho)) cJSON_AddNumberToObject(root, "chihoLv", sysVar[esc_cache.tm1_chiho]);
	} else if (esc_cache.game == S35_GAME_KICHIKUOU) {
		cJSON_AddNumberToObject(root, "gold", sysVar[522]);
		cJSON_AddNumberToObject(root, "month", sysVar[546]);
		cJSON_AddNumberToObject(root, "week", sysVar[547]);
	} else if (esc_cache.game == S35_GAME_KAERUNYO) {
		cJSON_AddNumberToObject(root, "gold", sysVar[171]);
		struct VarRef r0;
		if (v_ref_indexed(399, 0, &r0) && r0.page > 0 && varPage[r0.page].value)
			cJSON_AddNumberToObject(root, "ropes", varPage[r0.page].value[r0.index]);
	}
	pthread_mutex_unlock(&esc_cache_lock);

	char *out = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	publish_live_state_json(out);
}

void xsystem35_modifier_set_locks(int mask)
{
	pthread_mutex_lock(&action_lock);
	esc_live_locks = mask;
	pthread_mutex_unlock(&action_lock);
}

int xsystem35_modifier_get_locks(void)
{
	pthread_mutex_lock(&action_lock);
	int mask = esc_live_locks;
	pthread_mutex_unlock(&action_lock);
	return mask;
}

void xsystem35_modifier_apply_action(const char *action)
{
	if (!action)
		return;
	pthread_mutex_lock(&action_lock);
	if (pending_action_count < MAX_PENDING_ACTIONS) {
		snprintf(pending_actions[pending_action_count], sizeof(pending_actions[0]), "%s", action);
		pending_action_count++;
	}
	pthread_mutex_unlock(&action_lock);
}

bool escalayer_modifier_available(void)
{
	pthread_mutex_lock(&esc_cache_lock);
	refresh_escalayer_cache_locked();
	bool avail = esc_cache.game != S35_GAME_NONE;
	pthread_mutex_unlock(&esc_cache_lock);
	return avail;
}

const char *xsystem35_modifier_current_game(void)
{
	pthread_mutex_lock(&esc_cache_lock);
	refresh_escalayer_cache_locked();
	enum s35_mod_game g = esc_cache.game;
	pthread_mutex_unlock(&esc_cache_lock);
	return game_id_str(g);
}

void escalayer_modifier_tick(void)
{
	char actions[MAX_PENDING_ACTIONS][sizeof(pending_actions[0])];
	pthread_mutex_lock(&action_lock);
	int count = pending_action_count;
	memcpy(actions, pending_actions, count * sizeof(actions[0]));
	pending_action_count = 0;
	int locks = esc_live_locks;
	pthread_mutex_unlock(&action_lock);

	for (int i = 0; i < count; i++)
		apply_single_action_sdl_thread(actions[i]);
	apply_escalayer_locks_sdl_thread(locks);
	update_live_state_sdl_thread(locks);
}

void escalayer_modifier_reset(bool active)
{
	pthread_mutex_lock(&esc_cache_lock);
	reset_cache_indices_locked();
	pthread_mutex_unlock(&esc_cache_lock);
	pthread_mutex_lock(&live_state_lock);
	published = false;
	last_publish_tick = 0;
	pthread_mutex_unlock(&live_state_lock);
	pthread_mutex_lock(&action_lock);
	pending_action_count = 0;
	esc_live_locks = 0;
	pthread_mutex_unlock(&action_lock);
	publish_live_state_json(active ? strdup("{\"game\":\"system3\"}") : NULL);
}
