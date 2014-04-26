// 
static surface_t *blend(surface_t *base, int x, int y, surface_t *blend, int mode) {
	surface_t *dst = sf_create_surface(base->width, base->height, base->depth);
	
	gr_copy(dst, 0, 0, base, 0, 0, base->width, base->height);
	
	if (base->has_alpha) {
		// ベースに alpha map がある場合はそれをコピー
		gr_copy_alpha_map(dst, 0, 0, base, 0, 0, base->width, base->height);
	} else {
		// 無い場合は全て 255 の map を作成
		gr_fill_alpha_map(dst, 0, 0, base->width, base->height, 255);
	}
	
	if (blend->has_alpha) {
		// 重ね合わせ先の alpha map があるときはそれを使う
		gre_BlendUseAMap(dst, x, y, base, x, y, blend, 0, 0, blend->width, blend->height, blend, 0, 0, 255);
	} else {
		// 無いときは dst の alpha map を使う
		gre_BlendUseAMap(dst, x, y, base, x, y, blend, 0, 0, blend->width, blend->height, dst, x, y, 255);
	}
	
	// alpha 作成モードが dst + blend の時は飽和加算
	if (mode == 1) {
		gr_saturadd_alpha_map(dst, x, y, blend, 0, 0, blend->width, blend->height);
	}
	
	return dst;
}


