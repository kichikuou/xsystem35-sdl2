
#ifndef __SPRITE_H__
#define __SPRITE_H__

#include "portab.h"
#include <SDL_rect.h>

#define DEFAULT_UPDATE nt_sp_draw


#define SPRITEMAX 20
#define SPNO_WALL    0  // 壁紙
#define SPNO_SCENERY 1  // 背景
#define SPNO_TACHI_L 2  // 立ち絵左
#define SPNO_TACHI_M 3  // 立ち絵中央
#define SPNO_TACHI_R 4  // 立ち絵右
#define SPNO_MSGBG   5  // 文字下地
#define SPNO_MSGFRAME_BG 6 // メッセージウィンド枠
#define SPNO_MSGFRAME_FG 7 // メッセージ文字描画キャンバス
#define SPNO_FACE 11        // 人物顔CG
#define SPNO_MSG_KEYANIM 12 // メッセージキー入力アニメーション
#define SPNO_MSG_ICON_MUTE 13 // メッセージウィンドアイコン

#define CGMAX 65536
// 0-9999: reserve for Link CG
// 2051: メッセージウィンド枠CG
#define CGNO_MSGFRAME_LCG 2051
// 4017: メッセージウィンドで声をmuteにするCG
#define CGNO_MSGFRAME_NOVICE_LCG 4017
// 4018: メッセージウィンドのアイコンにマウスが重なったときのCG
#define CGNO_MSGFRAME_ICONREV_LCG 4018
// 4034: キー入力を促すアニメーションCG
#define CGNO_MSGHAK_LCG 4034

// 10000: メッセージウィンドでのキー待ちアニメ その1
#define CGNO_MSGHAK_1 10000
// 10001: メッセージウィンドでのキー待ちアニメ その2
#define CGNO_MSGHAK_2 10001
// 10002: 文字下地CG
#define CGNO_MSGFR_BG 10002


// CG_XX で作るCGの種類
enum cgtype {
	CG_NOTUSED = 0,
	CG_LINKED  = 1,
	CG_SET     = 2,
	CG_REVERSE = 3,
	CG_STRETCH = 4
};

// cgに関する情報
struct _cginfo {
	enum cgtype type;  // CGの種類, 0: 未使用, 1:リンクされている, ...
	int no;            // CGの番号
	struct SDL_Surface *sf;
	int refcnt;        // 参照カウンタ。０になったら開放してもよい。
};
typedef struct _cginfo cginfo_t;


enum spritetype {
	SPRITE_NORMAL = 0,
	SPRITE_ANIME  = 5,
	SPRITE_MSG    = 100,
	SPRITE_WP,
	SPRITE_NONE   = -1
};

struct _sprite {
	enum spritetype type;
	
	int no;
	
	int width;
	int height;
	
	cginfo_t *curcg;
	cginfo_t *cg1, *cg2, *cg3;
	
	bool show;
	
	int blendrate;
	
	SDL_Point loc;
	
	SDL_Point cur;

	void (* update)(struct _sprite *sp, SDL_Rect *updatearea);
	
	union {
		struct {
			int interval;
			int startttime;
			int npat;
			unsigned int tick;
		} anime;
		
		struct {
			struct SDL_Surface *canvas;
			SDL_Point dspcur;
		} msg;
	} u;
};

typedef struct _sprite sprite_t;


/* in nt_sprite.c */
sprite_t *nt_sp_new(int no, int cg1, int cg2, int cg3, int type);
sprite_t *nt_sp_msg_new(int no, int x, int y, int width, int height);
void nt_sp_free(sprite_t *sp);
void nt_sp_set_show(sprite_t *sp, bool show);
// void nt_sp_set_cg(sprite_t *sp, int no);
void nt_sp_set_loc(sprite_t *sp, int x, int y);

/* in nt_sprite_update.c */
void nt_sp_update_clipped();
void nt_sp_update_all(bool syncscreen);
void nt_sp_updateme(sprite_t *sp);
void nt_sp_updateme_part(sprite_t *sp, int x, int y, int w, int h);
void nt_sp_add_updatelist(sprite_t *sp);
void nt_sp_remove_updatelist(sprite_t *sp);
void nt_sp_draw_wall(sprite_t *sp, SDL_Rect *r);
void nt_sp_clear_updatelist(void);

/* in nt_sprite_draw.c */
void nt_sp_draw(sprite_t *sp, SDL_Rect *r);
void nt_sp_draw_scg(sprite_t *sp, SDL_Rect *r);

/* in nt_sprite_eupdate.c */
void nt_sp_eupdate(int type, int time, int cancel);


#endif /* __SPRITE_H__ */
