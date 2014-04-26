void gr_fill_alpha_overborder(surface_t *dst, int dx, int dy, int dw, int dh, int s, int d) {
	BYTE *dp;
	int x, y;
	
	if (dst == NULL) return;
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) return;
	
	dp = GETOFFSET_ALPHA(dst, dx, dy);
	if (dp == NULL) return;
	
	for (y = 0; y < dh; y++) {
		for (x = 0; x < dw; x++) {
			if (*(dp + x) >= (BYTE)s) *(dp + x) = (BYTE)d;
		}
		dp += dst->width;
	}
}

void gr_fill_alpha_underborder(surface_t *dst, int dx, int dy, int dw, int dh, int s, int d) {
	BYTE *dp;
	int x, y;

	if (dst == NULL) return;
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) return;
	
	dp = GETOFFSET_ALPHA(dst, dx, dy);
	if (dp == NULL) return;
	
	for (y = 0; y < dh; y++) {
		for (x = 0; x < dw; x++) {
			if (*(dp + x) <= (BYTE)s) *(dp + x) = (BYTE)d;
		}
		dp += dst->width;
	}
}

void gr_copy_alpha_map_sprite(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh, int cl) {
	BYTE *sp, *dp;
	int x, y;
	
	if (!gr_clip(src, &sx, &sy, &sw, &sh, dst, &dx, &dy)) return;
	
	sp = GETOFFSET_ALPHA(src, sx, sy);
	dp = GETOFFSET_ALPHA(dst, dx, dy);
	
	if (src == dst) {
		if (sy <= dy && dy < (sy + sh)) {
			sp += (sh -1) * src->width;
			dp += (sh -1) * dst->width;
			for (y = 0; y < sh; y++) {
				for (x = 0; x < sw; x++) {
					if (*(sp + x) != cl) *(dp + x) = *(sp + x);
				}
				sp -= src->width;
				dp -= dst->width;
			}
		} else {
			for (y = 0; y < sh; y++) {
				for (x = 0; x < sw; x++) {
					if (*(sp + x) != cl) *(dp + x) = *(sp + x);
				}
				sp += src->width;
				dp += dst->width;
			}
		}
	} else {
		for (y = 0; y < sh; y++) {
			for (x = 0; x < sw; x++) {
				if (*(sp + x) != cl) *(dp + x) = *(sp + x);
			}
			sp += src->width;
			dp += dst->width;
		}
	}
}

void gr_blend_alpha_wds_stretch2x2(surface_t *src1, int sx1, int sy1, surface_t *src2, int sx2, int sy2, int sw, int sh, surface_t *dst, int dx, int dy) {
	surface_t *t = sf_create_surface(sw * 2, sh * 2, dst->depth);
	
	gr_copy_stretch(t, 0, 0, sw * 2, sh * 2, src2, sx2, sy2, sw, sh);
	gr_blend_alpha_wds(t, 0, 0, src1, sx1, sy1, sw * 2, sh * 2, dst, dx, dy);
	
	sf_free(t);

}

void gr_blend_alpha_wds(surface_t *src1, int sx1, int sy1, surface_t *src2, int sx2, int sy2, int sw, int sh, surface_t *dst, int dx, int dy) {
	int x, y;
	BYTE *sp1, *sp2, *dp, *sa;
	
	sp1  = GETOFFSET_PIXEL(src1, sx1, sy1);
	sp2  = GETOFFSET_PIXEL(src2, sx2, sy2);
	sa   = GETOFFSET_ALPHA(src1, sx1, sy1);
	dp   = GETOFFSET_PIXEL(dst, dx, dy);
	
	switch(dst->depth) {
	case 15:
	{
		WORD *yls1, *yls2, *yld;
		BYTE *yla;
		
		for (y = 0; y < sh; y++) {
			yls1 = (WORD *)(sp1  + y * src1->bytes_per_line);
			yls2 = (WORD *)(sp2  + y * src2->bytes_per_line);
			yld  = (WORD *)(dp   + y * dst->bytes_per_line);
			yla  = (BYTE *)(sa   + y * src1->width);
			
			for (x = 0; x < sw; x++) {
				*yld = SUTURADD15(*yls1, ALPHABLEND15(*yls1, *yls2, *yla));
				yls1++; yls2++; yld++; yla++;
			}
		}
		break;
	}
	case 16:
//		if (nact->mmx_is_ok) {
		if (0) {
#ifdef ENABLE_MMX
			ablend16_wds_pppp(dp, sp1, sp2, sa, sw, sh,
					  dst->bytes_per_line,
					  src1->bytes_per_line,
					  src2->bytes_per_line,
					  src1->width);
#endif
		} else {
			WORD *yls1, *yls2, *yld;
			BYTE *yla;
			
			for (y = 0; y < sh; y++) {
				yls1 = (WORD *)(sp1  + y * src1->bytes_per_line);
				yls2 = (WORD *)(sp2  + y * src2->bytes_per_line);
				yld  = (WORD *)(dp   + y * dst->bytes_per_line);
				yla  = (BYTE *)(sa   + y * src1->width);
				
				for (x = 0; x < sw; x++) {
					*yld = SUTURADD16(*yls1, ALPHABLEND16(*yls1, *yls2, *yla));
					//*yld = SUTURADD16(*yls1, ALPHALEVEL16(*yls2, *yla));
					yls1++; yls2++; yld++; yla++;
				}
			}
		}
		break;
	case 24:
	case 32:
	{
		DWORD *yls1, *yls2, *yld;
		BYTE *yla;
		
		for (y = 0; y < sh; y++) {
			yls1 = (DWORD *)(sp1  + y * src1->bytes_per_line);
			yls2 = (DWORD *)(sp2  + y * src2->bytes_per_line);
			yld  = (DWORD *)(dp   + y * dst->bytes_per_line);
			yla  = (BYTE  *)(sa   + y * src1->width);
			
			for (x = 0; x < sw; x++) {
				*yld = SUTURADD24(*yls1, ALPHABLEND24(*yls1, *yls2, *yla));
				yls1++; yls2++; yld++; yla++;
			}
		}
		break;
	}
	}
}

