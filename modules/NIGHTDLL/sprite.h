
#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <glib.h>

#include "portab.h"
#include "surface.h"
#include "graphics.h"

#define DEFAULT_UPDATE sp_draw


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
	surface_t *sf;     // CG本体
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
	
	MyDimension cursize;
	
	cginfo_t *curcg;
	cginfo_t *cg1, *cg2, *cg3;
	
	boolean show;
	
	int blendrate;
	
	MyPoint loc;
	
	MyPoint cur;

	int (* update)(struct _sprite *sp, MyRectangle *updatearea);
	
	union {
		struct {
			int interval;
			int startttime;
			int npat;
			unsigned int tick;
		} anime;
		
		struct {
			surface_t *canvas;
			MyPoint dspcur;
		} msg;
	} u;
};

typedef struct _sprite sprite_t;


/* in nt_sprite.c */
extern sprite_t *sp_new(int no, int cg1, int cg2, int cg3, int type);
extern sprite_t *sp_msg_new(int no, int x, int y, int width, int height);
extern void sp_free(sprite_t *sp);
extern void sp_set_show(sprite_t *sp, boolean show);
// extern void sp_set_cg(sprite_t *sp, int no);
extern void sp_set_loc(sprite_t *sp, int x, int y);

/* in nt_sprite_update.c */
extern int sp_update_clipped();
extern int sp_update_all(boolean syncscreen);
extern int sp_updateme(sprite_t *sp);
extern int sp_updateme_part(sprite_t *sp, int x, int y, int w, int h);
extern void sp_add_updatelist(sprite_t *sp);
extern void sp_remove_updatelist(sprite_t *sp);
extern int sp_draw_wall(sprite_t *sp, MyRectangle *r);

/* in nt_sprite_draw.c */
extern int sp_draw(sprite_t *sp, MyRectangle *r);
extern int sp_draw2(sprite_t *sp, cginfo_t *cg, MyRectangle *r);
extern void sp_draw_dmap(gpointer data, gpointer userdata);
extern int sp_draw_scg(sprite_t *sp, MyRectangle *r);

/* in nt_sprite_eupdate.c */
extern int sp_eupdate(int type, int time, int cancel);


#endif /* __SPRITE_H__ */
