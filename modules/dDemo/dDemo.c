#include "config.h"

#include <stdio.h>
#include <math.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "input.h"
#include "music.h"
#include "alk.h"
#include "cg.h"
#include "gfx.h"
#include "gfx_private.h"

// Perspective projection parameters
#define PROJ_SCALE_X  1.2993f   // cot(30°) / (4/3)
#define PROJ_SCALE_Y  1.7321f   // cot(30°) = √3
#define PROJ_W_DENOM  0.5000625f

static inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }

static void render_quad(SDL_Texture *tex, SDL_Vertex verts[4]) {
	static const int indices[] = {0, 1, 2, 0, 2, 3};
	SDL_RenderGeometry(gfx_renderer, tex, verts, 4, indices, 6);
}

static void fade_overlay(Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) {
	SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gfx_renderer, r, g, b, alpha);
	SDL_RenderFillRect(gfx_renderer, NULL);
}

#define NR_SCENES 13

static const int scene_duration[] = {
	5400,
	4752,
	24963,
	4000,
	20000,
	4000,
	21000,
	4000,
	6402,
	4000,
	10370,
	31613,
	10338,
};

#define NR_TEXTURES 86

static struct {
	bool loop;
	bool key_cancel;
	SDL_Texture *images[NR_TEXTURES];
} dd;

// For each corner, if a coordinate is out of [0, view_w-1] x [0, view_h-1],
// shift all other corners by the overshoot amount, then hard-clamp everything
// to the screen boundary.
static void clamp_rect_to_screen(int xs[4], int ys[4]) {
	for (int i = 0; i < 4; i++) {
		if (xs[i] < 0) {
			for (int j = 0; j < 4; j++) if (j != i) xs[j] -= xs[i];
			xs[i] = 0;
		}
		if (ys[i] < 0) {
			for (int j = 0; j < 4; j++) if (j != i) ys[j] -= ys[i];
			ys[i] = 0;
		}
		if (xs[i] >= view_w - 1) {
			for (int j = 0; j < 4; j++) if (j != i) xs[j] += (view_w - 1 - xs[i]);
			xs[i] = view_w - 1;
		}
		if (ys[i] >= view_h - 1) {
			for (int j = 0; j < 4; j++) if (j != i) ys[j] += (view_h - 1 - ys[i]);
			ys[i] = view_h - 1;
		}
	}
	for (int i = 0; i < 4; i++) {
		if (xs[i] < 0) xs[i] = 0;
		if (xs[i] >= view_w) xs[i] = view_w - 1;
		if (ys[i] < 0) ys[i] = 0;
		if (ys[i] >= view_h) ys[i] = view_h - 1;
	}
}

static void Init() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int *var = getCaliVariable();
	
	if (!nact->files.alk[0]) {
		WARNING("dDEMO.alk not found");
		return;
	}
	alk_t *alk = alk_new(nact->files.alk[0]);  // dDEMO.alk
	if (!alk) {
		WARNING("Cannot open %s", nact->files.alk[0]);
		*var = 0;
		return;
	}
	if (alk->datanum < NR_TEXTURES) {
		WARNING("dDEMO.alk has %d entries, expected %d", alk->datanum, NR_TEXTURES);
		alk_free(alk);
		*var = 0;
		return;
	}
	for (int i = 0; i < NR_TEXTURES; i++) {
		bool load_as_alpha_map = i < 18 || i > 60;
		SDL_Surface *sf = cg_load_as_sdlsurface_from_data(alk->entries[i].data, alk->entries[i].size, false, load_as_alpha_map);
		if (!sf) {
			WARNING("failed to load image %d in dDEMO.alk", i);
			dd.images[i] = NULL;
			continue;
		}
		dd.images[i] = SDL_CreateTextureFromSurface(gfx_renderer, sf);
		SDL_FreeSurface(sf);
	}
	*var = 1;
	TRACE("dDemo.Init %d,%d,%d,%p:", p1, p2, p3, var);
}

static void SetKeyCancelFlag() {
	int cancelflag = getCaliValue();
	dd.key_cancel = cancelflag != 0;
	TRACE("dDemo.SetKeyCancelFlag %d:", cancelflag);
}

static void SetLoopFlag() {
	int loopflag = getCaliValue();
	dd.loop = loopflag != 0;
	TRACE("dDemo.SetLoopFlag %d:", loopflag);
}

static void render_affine_bg(SDL_Texture *scroll_bg, int elapsed, int duration,
                              float angle_deg, float scale, int offset_x, int offset_y) {
	if (!scroll_bg) return;

	int bg_w, bg_h;
	SDL_QueryTexture(scroll_bg, NULL, NULL, &bg_w, &bg_h);

	int xs[4], ys[4];
	const float angle = angle_deg * (float)M_PI / 180.0f;
	const float hw = view_w * 0.5f, hh = view_h * 0.5f;
	const float cosA = cosf(angle), sinA = sinf(angle);
	const float cxs[4] = {-hw,  hw,  hw, -hw};
	const float cys[4] = {-hh, -hh,  hh,  hh};
	for (int i = 0; i < 4; i++) {
		xs[i] = (int)((cosA * cxs[i] - sinA * cys[i]) * scale + hw) + offset_x;
		ys[i] = (int)((sinA * cxs[i] + cosA * cys[i]) * scale + hh) + offset_y;
	}
	clamp_rect_to_screen(xs, ys);

	float t = (float)elapsed / (float)duration;
	float w = (float)view_w, h = (float)view_h;

	float uv[4][2];
	uv[0][0] = 0.0f;
	uv[0][1] = lerpf(0.0f,     (float)ys[0], t);
	uv[1][0] = lerpf(w - 1.0f, (float)xs[1], t);
	uv[1][1] = lerpf(0.0f,     (float)ys[1], t);
	uv[2][0] = lerpf(w - 1.0f, (float)xs[2], t);
	uv[2][1] = lerpf(h - 1.0f, (float)ys[2], t);
	uv[3][0] = lerpf(0.0f,     (float)xs[3], t);
	uv[3][1] = lerpf(h - 1.0f, (float)ys[3], t);

	for (int i = 0; i < 4; i++) {
		uv[i][0] /= (float)bg_w;
		uv[i][1] /= (float)bg_h;
	}

	SDL_Vertex verts[4];
	static int y_toggle = 0;

	// Pass 1: Forward mapping layer
	for (int i = 0; i < 4; i++) {
		verts[i].position.x = (i == 1 || i == 2) ? w : 0;
		verts[i].position.y = (i == 2 || i == 3) ? h : (float)y_toggle;
		verts[i].color = (SDL_Color){255, 255, 255, 255};
		verts[i].tex_coord.x = uv[i][0];
		verts[i].tex_coord.y = uv[i][1];
	}
	SDL_SetTextureBlendMode(scroll_bg, SDL_BLENDMODE_NONE);
	render_quad(scroll_bg, verts);

	// Pass 2: 180° rotated mapping layer
	y_toggle = (y_toggle + 1) & 1;
	for (int i = 0; i < 4; i++) {
		verts[i].position.y = (i == 2 || i == 3) ? h : (float)y_toggle;
		verts[i].color = (SDL_Color){255, 255, 255, 128};
	}
	// Swap mapping: TL<->BR, TR<->BL for 180° rotation effect
	verts[0].tex_coord.x = uv[2][0]; verts[0].tex_coord.y = uv[2][1];
	verts[1].tex_coord.x = uv[3][0]; verts[1].tex_coord.y = uv[3][1];
	verts[2].tex_coord.x = uv[0][0]; verts[2].tex_coord.y = uv[0][1];
	verts[3].tex_coord.x = uv[1][0]; verts[3].tex_coord.y = uv[1][1];

	SDL_SetTextureBlendMode(scroll_bg, SDL_BLENDMODE_BLEND);
	render_quad(scroll_bg, verts);
}

static void render_scene0(int elapsed) {
	SDL_Texture *scroll_bg = dd.images[79];
	SDL_Texture *sprite    = dd.images[77];
	if (!sprite) return;

	int spr_w, spr_h;
	SDL_QueryTexture(sprite, NULL, NULL, &spr_w, &spr_h);
	const int slide_end = 4700;
	const int duration  = scene_duration[0];  // 5400

	// --- Background: DrawAffineSurfaceAlpha50 x2 (alpha=50%) ---
	// Source coordinates are linear-interpolated between identity and a transformed state.
	// Two layers (identity and 180° rotated) are blended together.
	render_affine_bg(scroll_bg, elapsed, duration, 40.0f, 0.55f, -20, 100);

	// --- Sprite and UI elements ---
	// 2. Sprite (ALK 77) with 3-echo horizontal slide-in.
	// Each copy starts wider/further left and converges to the final position.
	SDL_SetTextureBlendMode(sprite, SDL_BLENDMODE_BLEND);
	int dest_y = (view_h - spr_h) / 2;
	if (elapsed < slide_end) {
		SDL_SetTextureAlphaMod(sprite, 0x80);
		const int a0_vals[] = {640, 480, 320};
		for (int i = 0; i < 3; i++) {
			int a0 = a0_vals[i];
			int dest_w = a0 - ((a0 - spr_w) * elapsed) / slide_end;
			if (dest_w <= 0) continue;
			SDL_Rect dst = {view_w - 2 * dest_w, dest_y, dest_w, spr_h};
			SDL_RenderCopy(gfx_renderer, sprite, NULL, &dst);
		}
	} else {
		SDL_SetTextureAlphaMod(sprite, 0xd0);
		SDL_Rect dst = {view_w - 2 * spr_w, dest_y, spr_w, spr_h};
		SDL_RenderCopy(gfx_renderer, sprite, NULL, &dst);
	}

	// 3. Horizontal accent line (grows rightward over 4700ms)
	int line_w = elapsed < slide_end ? 639 * elapsed / slide_end + 1 : 640;
	SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(gfx_renderer, 0xc0, 0xc0, 0xc0, 0xff);
	SDL_Rect hline = {0, (spr_h + view_h) / 2 - 4, line_w, 2};
	SDL_RenderFillRect(gfx_renderer, &hline);

	// 4. Vertical accent line (right edge of sprite, grows downward)
	int line_h = elapsed < slide_end ? 479 * elapsed / slide_end + 1 : 480;
	SDL_Rect vline = {view_w - spr_w, 0, 2, line_h};
	SDL_RenderFillRect(gfx_renderer, &vline);
}

static void render_scene1(int elapsed) {
	SDL_Texture *scroll_bg = dd.images[78];
	SDL_Texture *sprite    = dd.images[76];
	if (!sprite) return;

	int spr_w, spr_h;
	SDL_QueryTexture(sprite, NULL, NULL, &spr_w, &spr_h);
	const int duration = scene_duration[1];  // 4752

	// --- Background: DrawAffineSurfaceAlpha50 x2 (alpha=50%) ---
	render_affine_bg(scroll_bg, elapsed, duration, -45.0f, 0.1f, 0, 200);

	// Foreground (ALK 76): centered, fades in over first 3000ms.
	// graph_BlendAlphaMapColorAlpha (alpha 0->255) for elapsed < 3000,
	// graph_BlendAlphaMapColor (full opacity) for elapsed >= 3000.
	{
		int alpha = elapsed < 3000 ? (elapsed * 255) / 3000 : 255;
		SDL_SetTextureBlendMode(sprite, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(sprite, (Uint8)alpha);
		SDL_Rect dst = {(view_w - spr_w) / 2, (view_h - spr_h) / 2, spr_w, spr_h};
		SDL_RenderCopy(gfx_renderer, sprite, NULL, &dst);
	}

	// Fade to black: graph_FillAlphaColor(0,0,0), starts at 4000ms.
	if (elapsed > 4000) {
		int fade_alpha = ((elapsed - 4000) * 255) / (duration - 4000);
		fade_overlay(0, 0, 0, (Uint8)SDL_min(fade_alpha, 255));
	}
}

// 3D rendering for render_scene2_4_6.
//
// Background quad (model space, XY plane):
//   v0=(-1, -0.73446, 0)  v1=(+1, -0.73446, 0)
//   v2=(+1, +0.73446, 0)  v3=(-1, +0.73446, 0)
// Object border quad (slightly oversized):
//   v0=(-1.00885, -0.74655, 0)  ...  v3=(-1.00885, +0.74655, 0)
//
// Animation phases (phaseDuration = (sceneDuration - 1000) / 8):
//   Phase 0 [0..1pd):     approach from Z=-9 to Z=-3.75, Y-rot -90°→0°, tex1
//   Phase 1 [1..3pd):     static at Z=-3.75, rot=0°, tex1
//   Phase 2 [3..4pd):     recede from Z=-3.75 to Z=-9, Y-rot 0°→90°, tex1
//   Phase 3 [4..5pd):     approach from Z=-9 to Z=-3.75, Y-rot -90°→0°, tex2
//   Phase 4 [5..7pd):     static at Z=-3.75, rot=0°, tex2
//   Phase 5 [7..8pd):     recede from Z=-3.75 to Z=-9, Y-rot 0°→90°, tex2
//
// All phases also have an X-swing: tx = sin(π*t) * swing_amp
//   approach swing_amp = -1.5, recede = +1.5
//
// Projection: same parameters as render_scene12_3d.
static void render_scene_2_4_6_3d(int elapsed, int phase_dur, SDL_Texture *tex1, SDL_Texture *tex2) {
	static const float bg_mv[4][3] = {
		{-1.00000f, -0.73446f, 0.0f},
		{+1.00000f, -0.73446f, 0.0f},
		{+1.00000f, +0.73446f, 0.0f},
		{-1.00000f, +0.73446f, 0.0f},
	};
	static const float obj_mv[4][3] = {
		{-1.00885f, -0.74655f, 0.0f},
		{+1.00885f, -0.74655f, 0.0f},
		{+1.00885f, +0.74655f, 0.0f},
		{-1.00885f, +0.74655f, 0.0f},
	};
	static const float muv[4][2] = {
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
	};

	float tx, tz, rot_y_deg;
	SDL_Texture *tex;

	if (elapsed < phase_dur) {
		// Phase 0: approach with X-swing, tex1
		float t = (float)elapsed / (float)phase_dur;
		tx = sinf(t * (float)M_PI) * (-1.5f);
		tz = lerpf(-9.0f, -3.75f, t);
		rot_y_deg = lerpf(-90.0f, 0.0f, t);
		tex = tex1;
	} else if (elapsed < phase_dur * 3) {
		// Phase 1: static, tex1
		tx = 0.0f; tz = -3.75f; rot_y_deg = 0.0f;
		tex = tex1;
	} else if (elapsed < phase_dur * 4) {
		// Phase 2: recede with X-swing, tex1
		float t = (float)(elapsed - phase_dur * 3) / (float)phase_dur;
		tx = sinf(t * (float)M_PI) * 1.5f;
		tz = lerpf(-3.75f, -9.0f, t);
		rot_y_deg = lerpf(0.0f, 90.0f, t);
		tex = tex1;
	} else if (elapsed < phase_dur * 5) {
		// Phase 3: approach with X-swing, tex2
		float t = (float)(elapsed - phase_dur * 4) / (float)phase_dur;
		tx = sinf(t * (float)M_PI) * (-1.5f);
		tz = lerpf(-9.0f, -3.75f, t);
		rot_y_deg = lerpf(-90.0f, 0.0f, t);
		tex = tex2;
	} else if (elapsed < phase_dur * 7) {
		// Phase 4: static, tex2
		tx = 0.0f; tz = -3.75f; rot_y_deg = 0.0f;
		tex = tex2;
	} else if (elapsed < phase_dur * 8) {
		// Phase 5: recede with X-swing, tex2
		float t = (float)(elapsed - phase_dur * 7) / (float)phase_dur;
		tx = sinf(t * (float)M_PI) * 1.5f;
		tz = lerpf(-3.75f, -9.0f, t);
		rot_y_deg = lerpf(0.0f, 90.0f, t);
		tex = tex2;
	} else {
		return;
	}

	if (!tex) return;

	float angle_rad = rot_y_deg * (float)M_PI / 180.0f;
	float cos_a = cosf(angle_rad);
	float sin_a = sinf(angle_rad);

	const float half_w = (float)view_w * 0.5f;
	const float half_h = (float)view_h * 0.5f;

	SDL_Vertex bg_verts[4], obj_verts[4];
	for (int i = 0; i < 4; i++) {
		for (int pass = 0; pass < 2; pass++) {
			const float (*mv)[3] = (pass == 0) ? bg_mv : obj_mv;
			SDL_Vertex *v = (pass == 0) ? &bg_verts[i] : &obj_verts[i];
			float rx = cos_a * mv[i][0] + sin_a * mv[i][2];
			float ry = mv[i][1];
			float rz = -sin_a * mv[i][0] + cos_a * mv[i][2];
			float wx = rx + tx;
			float wy = ry;
			float wz = rz + tz;
			float w = -wz * PROJ_W_DENOM;
			if (w <= 0.0f) w = 1e-5f;
			v->position.x = (wx * PROJ_SCALE_X / w + 1.0f) * half_w;
			v->position.y = (wy * PROJ_SCALE_Y / w + 1.0f) * half_h;
			v->tex_coord.x = muv[i][0];
			v->tex_coord.y = muv[i][1];
		}
	}

	// Object border quad: solid color RGB(244,221,181) = BGR(0xb5,0xdd,0xf4),
	// drawn first as a backdrop. The bg quad drawn on top leaves only the thin
	// border ring visible.
	for (int i = 0; i < 4; i++) obj_verts[i].color = (SDL_Color){0xf4, 0xdd, 0xb5, 0xff};
	SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_NONE);
	render_quad(NULL, obj_verts);

	// Background quad: textured, opaque, drawn on top (covers center of border quad)
	for (int i = 0; i < 4; i++) bg_verts[i].color = (SDL_Color){255, 255, 255, 255};
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
	render_quad(tex, bg_verts);
}

// Scenes 2, 4, and 6 share the same structure:
//   - Fixed background (ALK[9]) full-screen blit
//   - 3D quad animation (see render_scene_2_4_6_3d above)
//   - Sprite + shadow scroll linearly left-to-right over the full scene duration
//
// sprite_idx:  ALK index of the scrolling sprite (shadow is at sprite_idx + 73)
// tex1_idx:  ALK index of the 3D quad texture for phases 0-2
// tex2_idx:  ALK index of the 3D quad texture for phases 3-5
static void render_scene_2_4_6(int elapsed, int scene_idx, int sprite_idx, int tex1_idx, int tex2_idx) {
	const int duration = scene_duration[scene_idx];
	SDL_Texture *bg            = dd.images[9];
	SDL_Texture *sprite        = dd.images[sprite_idx];
	SDL_Texture *sprite_shadow = dd.images[sprite_idx + 73];

	// 1. Background: full-screen blit, no blending
	if (bg) {
		SDL_SetTextureBlendMode(bg, SDL_BLENDMODE_NONE);
		SDL_SetTextureColorMod(bg, 0xff, 0xff, 0xff);
		SDL_RenderCopy(gfx_renderer, bg, NULL, NULL);
	}

	// 2. 3D rotating quad animation (8-phase timing)
	{
		int phase_dur = (duration - 1000) / 8;
		SDL_Texture *tex1 = dd.images[tex1_idx];
		SDL_Texture *tex2 = dd.images[tex2_idx];
		render_scene_2_4_6_3d(elapsed, phase_dur, tex1, tex2);
	}

	// 3. Scrolling sprite: x moves from -sprite_w (off-screen left) to view_w (off-screen right)
	int sprite_w = 0, sprite_h = 0;
	if (sprite)
		SDL_QueryTexture(sprite, NULL, NULL, &sprite_w, &sprite_h);
	int x = sprite_w ? ((sprite_w + view_w) * elapsed) / duration - sprite_w : 0;

	// Shadow drawn first (color-modded to black), sprite on top (full color)
	if (sprite_shadow) {
		int sw, sh;
		SDL_QueryTexture(sprite_shadow, NULL, NULL, &sw, &sh);
		SDL_SetTextureBlendMode(sprite_shadow, SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(sprite_shadow, 0, 0, 0);
		SDL_Rect dst = {x, 0, sw, sh};
		SDL_RenderCopy(gfx_renderer, sprite_shadow, NULL, &dst);
	}
	if (sprite) {
		SDL_SetTextureBlendMode(sprite, SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(sprite, 0xff, 0xff, 0xff);
		SDL_Rect dst = {x, 0, sprite_w, sprite_h};
		SDL_RenderCopy(gfx_renderer, sprite, NULL, &dst);
	}
}

// Scenes 3, 5, 7, and 9 share the same structure:
//   - Main image (image_idx) fades in and out over 4 seconds
//   - ALK[9] overlay drawn at 50% alpha with an animated quad-warp throughout
//
// Timing of main image:
//   0–250ms:    black (no image)
//   250–1000ms: fade in  (alpha 0→255)
//   1000–3000ms: full opacity
//   3000–4000ms: fade out (alpha 255→0)
//
// Overlay quad-warp: source corners rotate clockwise along image edges over the full duration.
//   At t=0: source = full image (no warp).
//   At t=1: each corner has shifted 1/3 of its edge length, creating a rotation-like distortion.
static void render_scene_3_5_7_9(int elapsed, int image_idx) {
	SDL_Texture *main_img = dd.images[image_idx];
	SDL_Texture *overlay  = dd.images[9];

	if (main_img) {
		int alpha;
		if (elapsed < 250)
			alpha = 0;
		else if (elapsed < 1000)
			alpha = ((elapsed - 250) * 255) / 750;
		else if (elapsed < 3000)
			alpha = 255;
		else
			alpha = ((4000 - elapsed) * 255) / 1000;
		alpha = SDL_max(0, SDL_min(255, alpha));
		if (alpha > 0) {
			SDL_SetTextureBlendMode(main_img, alpha < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
			SDL_SetTextureAlphaMod(main_img, (Uint8)alpha);
			SDL_RenderCopy(gfx_renderer, main_img, NULL, NULL);
		}
	}

	// Overlay: 50% alpha blend with animated quad-warp.
	// UV corner positions (normalized): corners shift by t/3 along each edge.
	if (overlay) {
		float t3 = (float)elapsed / (4000.0f * 3.0f);  // 0 → 1/3
		float w = (float)view_w, h = (float)view_h;
		SDL_Vertex verts[4] = {
			/* TL */ {{0.f, 0.f}, {255, 255, 255, 128}, {t3,        0.f    }},
			/* TR */ {{w,   0.f}, {255, 255, 255, 128}, {1.0f,      t3     }},
			/* BR */ {{w,   h  }, {255, 255, 255, 128}, {1.0f - t3, 1.0f   }},
			/* BL */ {{0.f, h  }, {255, 255, 255, 128}, {0.f,       1.0f-t3}},
		};
		SDL_SetTextureBlendMode(overlay, SDL_BLENDMODE_BLEND);
		render_quad(overlay, verts);
	}
}

// Scene 8 is a slideshow of 6 images (ALK 80–85) with crossfade transitions.
// Total duration (scene_duration[8] = 6402ms) is divided into 7 equal segments:
//   Segment 1:      fade in   image[0] (black → full)
//   Segments 2–6:   crossfade image[i-1] → image[i]
//   Segment 7:      hold      image[5] at full opacity
static void render_scene8(int elapsed) {
	const int duration = scene_duration[8];
	const int seg = duration / 7;

	SDL_Texture **imgs = dd.images + 80;  // ALK[80..85]

	if (elapsed < seg) {
		// Segment 1: fade in image[0] from black
		int alpha = seg > 0 ? (elapsed * 255) / seg : 255;
		if (imgs[0]) {
			SDL_SetTextureBlendMode(imgs[0], SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(imgs[0], (Uint8)SDL_min(alpha, 255));
			SDL_RenderCopy(gfx_renderer, imgs[0], NULL, NULL);
		}
	} else if (elapsed < 6 * seg) {
		// Segments 2–6: crossfade image[idx] → image[idx+1]
		int idx = elapsed / seg - 1;  // 0..4
		int alpha = ((elapsed - (idx + 1) * seg) * 255) / seg;
		if (imgs[idx]) {
			SDL_SetTextureBlendMode(imgs[idx], SDL_BLENDMODE_NONE);
			SDL_SetTextureAlphaMod(imgs[idx], 255);
			SDL_RenderCopy(gfx_renderer, imgs[idx], NULL, NULL);
		}
		if (imgs[idx + 1]) {
			SDL_SetTextureBlendMode(imgs[idx + 1], SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(imgs[idx + 1], (Uint8)SDL_min(alpha, 255));
			SDL_RenderCopy(gfx_renderer, imgs[idx + 1], NULL, NULL);
		}
	} else {
		// Segment 7: hold image[5] at full opacity
		if (imgs[5]) {
			SDL_SetTextureBlendMode(imgs[5], SDL_BLENDMODE_NONE);
			SDL_SetTextureAlphaMod(imgs[5], 255);
			SDL_RenderCopy(gfx_renderer, imgs[5], NULL, NULL);
		}
	}
}

// Scene 10 is a 10.37s animation with 3 phases:
// images[18..31]: surfaces1, 14 animation frames (type A)
// images[32..59]: surfaces2, 28 animation frames (type B/C pairs per slot)
// images[10]:     surface3,  static full-screen image for zoom/fade
//
// Phase 1 (0–6453ms): slideshow of 14 frames × 461ms each.
//   Within each frame, 3 sub-images are shown in sequence:
//     0–152ms:   images[18+frame]        (surfaces1[frame])
//     153–306ms: images[32+frame*2]      (surfaces2[frame*2])
//     307–460ms: images[32+frame*2+1]    (surfaces2[frame*2+1])
//
// Phase 2 (6454–9609ms): ZoomBlend – images[10] zooms in from a tiny
//   bottom-centered region (16 × view_h/4) to full screen with pow(4) ease-in.
//
// Phase 3 (9610–10370ms): images[10] fades out to black (alpha 255→0).
static void render_scene10(int elapsed) {
	const int duration = scene_duration[10];
	SDL_Texture *surface3 = dd.images[10];

	if (elapsed < 6454) {
		// Phase 1: frame animation
		int frame = elapsed / 461;
		int phase = elapsed % 461;
		int idx;
		if (phase < 153)
			idx = 18 + frame;
		else if (phase < 307)
			idx = 32 + frame * 2;
		else
			idx = 32 + frame * 2 + 1;
		SDL_Texture *tex = dd.images[idx];
		if (tex) {
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
			SDL_SetTextureAlphaMod(tex, 255);
			SDL_SetTextureColorMod(tex, 255, 255, 255);
			SDL_RenderCopy(gfx_renderer, tex, NULL, NULL);
		}
	} else if (elapsed < 9610) {
		// Phase 2: zoom in
		int h4 = view_h / 4;
		float progress = 1.0f - (float)(elapsed - 6454) / (float)(9610 - 6454);
		float ease = powf(progress, 4.0f);
		float src_w = lerpf((float)view_w, 16.0f, ease);
		float src_h = lerpf((float)view_h, (float)h4, ease);
		SDL_Rect src_rect = {
			(int)((view_w - src_w) * 0.5f),
			(int)(view_h - src_h),
			(int)src_w,
			(int)src_h,
		};
		if (surface3) {
			SDL_SetTextureBlendMode(surface3, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(surface3, 255);
			SDL_SetTextureColorMod(surface3, 255, 255, 255);
			SDL_RenderCopy(gfx_renderer, surface3, &src_rect, NULL);
		}
	} else {
		// Phase 3: fade out to black
		int blend_alpha = (elapsed - 9610) * 255 / (duration - 9610);
		int combined = 255 - SDL_min(blend_alpha, 255);
		if (surface3) {
			SDL_SetTextureBlendMode(surface3,
				combined < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
			SDL_SetTextureAlphaMod(surface3, (Uint8)combined);
			SDL_SetTextureColorMod(surface3, 255, 255, 255);
			SDL_RenderCopy(gfx_renderer, surface3, NULL, NULL);
		}
	}
}

// Scene 11 is a complex layered animation with 8 segments over 31.6s:
// Background (ALK 60) scrolls upward over the full scene duration.
// The scene duration (31613ms) is divided into 8 equal segments (~3951ms each):
//   Segment 0: background only
//   Segments 1, 3, 5: one of 3 images (ALK 63, 62, 61) slides top-to-bottom at 50% alpha
//   Segments 2, 4, 6: one of 3 overlay images (ALK 69, 70, 71) pingpong-fades in/out
//   Segment 7: white fade out
static void render_scene11(int elapsed) {
	const int duration = scene_duration[11];  // 31613
	const int unit = duration / 8;            // ~3951

	SDL_Texture *bg = dd.images[60];
	// Slide images (alpha maps, drawn at 50% alpha, pass top-to-bottom)
	SDL_Texture **slide = dd.images + 61;  // ALK[61..63]
	// Overlay images (alpha maps, pingpong blend)
	SDL_Texture **overlay = dd.images + 69;  // ALK[69..71]

	// Background: scrolls from bottom to top of source image over full duration.
	// srcY = (bg_h - view_h) * (duration - elapsed) / duration
	if (bg) {
		int bg_w, bg_h;
		SDL_QueryTexture(bg, NULL, NULL, &bg_w, &bg_h);
		int scroll = (int)((long long)(bg_h - view_h) * elapsed / duration);
		int src_y = SDL_max(0, (bg_h - view_h) - scroll);
		SDL_Rect src = {0, src_y, SDL_min(view_w, bg_w), view_h};
		SDL_SetTextureBlendMode(bg, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(gfx_renderer, bg, &src, NULL);
	}

	if (elapsed < unit) {
		// Segment 0: background only, nothing extra
	} else if (elapsed < unit * 2) {
		// Segment 1: slide[2] (ALK 63) passes top-to-bottom at 50% alpha
		SDL_Texture *tex = slide[2];
		if (tex) {
			int tw, th;
			SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
			int y = (int)((long long)(th + view_h) * (elapsed - unit) / unit) - th;
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(tex, 0x80);
			SDL_Rect dst = {(view_w - tw) / 2, y, tw, th};
			SDL_RenderCopy(gfx_renderer, tex, NULL, &dst);
		}
	} else if (elapsed < unit * 3) {
		// Segment 2: overlay[0] (ALK 69) pingpong fade
		SDL_Texture *tex = overlay[0];
		if (tex) {
			int t2 = elapsed % unit;
			if (t2 >= unit / 2) t2 = unit - t2;
			int alpha = (t2 * 255) / (unit / 2);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(tex, (Uint8)SDL_min(alpha, 255));
			SDL_RenderCopy(gfx_renderer, tex, NULL, NULL);
		}
	} else if (elapsed < unit * 4) {
		// Segment 3: slide[1] (ALK 62) passes top-to-bottom at 50% alpha
		SDL_Texture *tex = slide[1];
		if (tex) {
			int tw, th;
			SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
			int y = (int)((long long)(th + view_h) * (elapsed - unit * 3) / unit) - th;
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(tex, 0x80);
			SDL_Rect dst = {(view_w - tw) / 2, y, tw, th};
			SDL_RenderCopy(gfx_renderer, tex, NULL, &dst);
		}
	} else if (elapsed < unit * 5) {
		// Segment 4: overlay[1] (ALK 70) pingpong fade
		SDL_Texture *tex = overlay[1];
		if (tex) {
			int t2 = elapsed % unit;
			if (t2 >= unit / 2) t2 = unit - t2;
			int alpha = (t2 * 255) / (unit / 2);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(tex, (Uint8)SDL_min(alpha, 255));
			SDL_RenderCopy(gfx_renderer, tex, NULL, NULL);
		}
	} else if (elapsed < unit * 6) {
		// Segment 5: slide[0] (ALK 61) passes top-to-bottom at 50% alpha
		SDL_Texture *tex = slide[0];
		if (tex) {
			int tw, th;
			SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
			int y = (int)((long long)(th + view_h) * (elapsed - unit * 5) / unit) - th;
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(tex, 0x80);
			SDL_Rect dst = {(view_w - tw) / 2, y, tw, th};
			SDL_RenderCopy(gfx_renderer, tex, NULL, &dst);
		}
	} else if (elapsed < unit * 7) {
		// Segment 6: overlay[2] (ALK 71) pingpong fade
		SDL_Texture *tex = overlay[2];
		if (tex) {
			int t2 = elapsed % unit;
			if (t2 >= unit / 2) t2 = unit - t2;
			int alpha = (t2 * 255) / (unit / 2);
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(tex, (Uint8)SDL_min(alpha, 255));
			SDL_RenderCopy(gfx_renderer, tex, NULL, NULL);
		}
	} else {
		// Segment 7: white fade out
		int fade_alpha = (duration > unit * 7)
			? ((elapsed - unit * 7) * 255) / (duration - unit * 7)
			: 255;
		fade_overlay(0xff, 0xff, 0xff, (Uint8)SDL_min(fade_alpha, 255));
	}
}

// Scene 12 is a complex 10.37s animation with multiple layers and a 3D spinning quad:
// ALK[64]: background image A
// ALK[65]: background image B
// ALK[66]: overlay image
// ALK[67]: 3D quad texture
//
// Timeline (relative to scene start):
//   0–2076ms:    fade in bg_a from white (white overlay 255→0)
//   2076–2226ms: bg_a static
//   2226–3826ms: crossfade bg_a → bg_b
//   3826–3976ms: bg_b static
//   3976–7803ms: bg_b wave distortion (TODO) + bg_b at α=0xB0 + fade to white
//   7803–9397ms: white bg + 3D rotating quad + overlay fade in (0→255)
//   9397–10338ms: overlay static
//
// 3D rendering (7803–9397ms):
//   A rhombus-shaped textured quad in the XY plane at z=-2.4 (world space),
//   spinning around the Y axis (2 full rotations over 1594ms).
//   Projection: vertical FoV=60°, aspect=4:3, near=0.25, far=2000.
//   Vertex positions (model space):
//     v0=(0, -0.51524, 0)  UV:(0,0)
//     v1=(0.28806, 0.22720, 0)  UV:(1,0)
//     v2=(0, 0.51524, 0)  UV:(1,1)
//     v3=(-0.28806, -0.22720, 0)  UV:(0,1)
static void render_scene12_3d(int elapsed) {
	SDL_Texture *tex = dd.images[67];
	if (!tex) return;

	// Model-space vertices (x, y, z) forming a rhombus in the XY plane.
	static const float mv[4][3] = {
		{  0.00000f, -0.51524f,  0.0f },  // v0
		{  0.28806f,  0.22720f,  0.0f },  // v1
		{  0.00000f,  0.51524f,  0.0f },  // v2
		{ -0.28806f, -0.22720f,  0.0f },  // v3
	};
	// Normalized texture UV per vertex
	static const float muv[4][2] = {
		{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
	};

	// Y-rotation: 2 full rotations over the 1594ms 3D phase.
	float t = (float)(elapsed - 7803);
	float angle = t * (4.0f * (float)M_PI / 1594.0f);
	float cos_a = cosf(angle);
	float sin_a = sinf(angle);

	// Perspective projection parameters: see PROJ_SCALE_X/Y, PROJ_W_DENOM
	const float half_w = (float)view_w * 0.5f;
	const float half_h = (float)view_h * 0.5f;

	SDL_Vertex verts[4];
	for (int i = 0; i < 4; i++) {
		// World transform: Y-rotate first, then translate(+0.01, 0, -2.4).
		float rx = cos_a * mv[i][0] + sin_a * mv[i][2];
		float ry = mv[i][1];
		float rz = -sin_a * mv[i][0] + cos_a * mv[i][2];
		float wx = rx + 0.01f;
		float wy = ry;
		float wz = rz - 2.4f;

		// Perspective divide: w = -wz * PROJ_W_DENOM (wz is negative → w positive)
		float w = -wz * PROJ_W_DENOM;
		if (w <= 0.0f) w = 1e-5f;

		// NDC → screen
		verts[i].position.x = (wx * PROJ_SCALE_X / w + 1.0f) * half_w;
		verts[i].position.y = (wy * PROJ_SCALE_Y / w + 1.0f) * half_h;
		verts[i].color = (SDL_Color){255, 255, 255, 255};
		verts[i].tex_coord.x = muv[i][0];
		verts[i].tex_coord.y = muv[i][1];
	}

	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
	render_quad(tex, verts);
}

static void render_scene12(int elapsed) {
	SDL_Texture *bg_a    = dd.images[64];
	SDL_Texture *bg_b    = dd.images[65];
	SDL_Texture *overlay = dd.images[66];

	if (elapsed < 2076) {
		// 0–2076ms: bg_a with white fade-in overlay
		if (bg_a) {
			SDL_SetTextureBlendMode(bg_a, SDL_BLENDMODE_NONE);
			SDL_RenderCopy(gfx_renderer, bg_a, NULL, NULL);
		}
		int white_alpha = 0xff - (elapsed * 0xff) / 2076;
		fade_overlay(0xff, 0xff, 0xff, (Uint8)white_alpha);
	} else if (elapsed < 2226) {
		// 2076–2226ms: bg_a static
		if (bg_a) {
			SDL_SetTextureBlendMode(bg_a, SDL_BLENDMODE_NONE);
			SDL_RenderCopy(gfx_renderer, bg_a, NULL, NULL);
		}
	} else if (elapsed < 3826) {
		// 2226–3826ms: crossfade bg_a → bg_b
		if (bg_a) {
			SDL_SetTextureBlendMode(bg_a, SDL_BLENDMODE_NONE);
			SDL_RenderCopy(gfx_renderer, bg_a, NULL, NULL);
		}
		if (bg_b) {
			int alpha = ((elapsed - 2226) * 0xff) / 1600;
			SDL_SetTextureBlendMode(bg_b, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(bg_b, (Uint8)SDL_min(alpha, 0xff));
			SDL_RenderCopy(gfx_renderer, bg_b, NULL, NULL);
		}
	} else if (elapsed < 3976) {
		// 3826–3976ms: bg_b static
		if (bg_b) {
			SDL_SetTextureBlendMode(bg_b, SDL_BLENDMODE_NONE);
			SDL_RenderCopy(gfx_renderer, bg_b, NULL, NULL);
		}
	} else if (elapsed < 7803) {
		// 3976–7803ms: TODO wave distortion on bg_b; for now just show bg_b,
		// then fade to white
		if (bg_b) {
			SDL_SetTextureBlendMode(bg_b, SDL_BLENDMODE_BLEND);
			SDL_RenderCopy(gfx_renderer, bg_b, NULL, NULL);
		}
		int white_alpha = ((elapsed - 3976) * 0xff) / 3827;
		fade_overlay(0xff, 0xff, 0xff, (Uint8)SDL_min(white_alpha, 0xff));
	} else if (elapsed < 9397) {
		// 7803–9397ms: white bg + 3D rotating quad + overlay fade-in
		SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(gfx_renderer, 0xff, 0xff, 0xff, 0xff);
		SDL_RenderFillRect(gfx_renderer, NULL);
		render_scene12_3d(elapsed);
		if (overlay) {
			int alpha = ((elapsed - 7803) * 0xff) / 1594;
			SDL_SetTextureBlendMode(overlay, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(overlay, (Uint8)SDL_min(alpha, 0xff));
			SDL_RenderCopy(gfx_renderer, overlay, NULL, NULL);
		}
	} else {
		// 9397–10338ms: overlay static
		if (overlay) {
			SDL_SetTextureBlendMode(overlay, SDL_BLENDMODE_NONE);
			SDL_RenderCopy(gfx_renderer, overlay, NULL, NULL);
		}
	}
}

static void Run() {
	TRACE("dDemo.Run:");

	muscd_start(13, 1);
	uint32_t start = sys_get_ticks();
	int current_scene = 0;
	while (!nact->is_quit && !(sys_getInputInfo() && dd.key_cancel)) {
		uint32_t now = sys_get_ticks();
		if (current_scene < NR_SCENES && now - start >= (uint32_t)scene_duration[current_scene]) {
			current_scene++;
			if (current_scene < NR_SCENES) {
				start = now;
			} else if (dd.loop) {
				muscd_stop();
				current_scene = 0;
				start = now;
				muscd_start(13, 1);
			}
		}
		if (current_scene >= NR_SCENES) {
			break;
		}
		SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, 255);
		SDL_RenderClear(gfx_renderer);
		switch (current_scene) {
		case 0:
			render_scene0(now - start);
			break;
		case 1:
			render_scene1(now - start);
			break;
		case 2:
			render_scene_2_4_6(now - start, 2, 0, 3, 4);
			break;
		case 3:
			render_scene_3_5_7_9(now - start, 11);
			break;
		case 4:
			render_scene_2_4_6(now - start, 4, 1, 5, 6);
			break;
		case 5:
			render_scene_3_5_7_9(now - start, 12);
			break;
		case 6:
			render_scene_2_4_6(now - start, 6, 2, 7, 8);
			break;
		case 7:
			render_scene_3_5_7_9(now - start, 13);
			break;
		case 8:
			render_scene8(now - start);
			break;
		case 9:
			render_scene_3_5_7_9(now - start, 14);
			break;
		case 10:
			render_scene10(now - start);
			break;
		case 11:
			render_scene11(now - start);
			break;
		case 12:
			render_scene12(now - start);
			break;
		}
		SDL_RenderPresent(gfx_renderer);
		sys_wait_vsync();
	}

	muscd_stop();

	for (int i = 0; i < NR_TEXTURES; i++) {
		if (dd.images[i])
			SDL_DestroyTexture(dd.images[i]);
		dd.images[i] = NULL;
	}
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"Run", Run},
	{"SetKeyCancelFlag", SetKeyCancelFlag},
	{"SetLoopFlag", SetLoopFlag},
};

const Module module_dDemo = {"dDemo", functions, sizeof(functions) / sizeof(ModuleFunc)};
