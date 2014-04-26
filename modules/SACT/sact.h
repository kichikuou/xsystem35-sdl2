/*
 * sact.h: SACT
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: sact.h,v 1.3 2003/07/14 16:22:51 chikama Exp $ */

#ifndef __SACT_H__
#define __SACT_H__

#include "config.h"

#include <glib.h>
#include "portab.h"
#include "graphics.h"
#include "surface.h"
#include "sacttimer.h"
#include "variable.h"


// スプライトの最大数
#define SPRITEMAX 21845

// CGの最大数
#define CGMAX 63336

// メッセージの最大長さ
#define MSGBUFMAX 257*10

// 選択肢の最大要素数
#define SEL_ELEMENT_MAX 20


// キーウェイトの種類
#define KEYWAIT_NONE 0
#define KEYWAIT_SIMPLE 1
#define KEYWAIT_SPRITE 2
#define KEYWAIT_MESSAGE 3
#define KEYWAIT_SELECT 4
#define KEYWAIT_BACKLOG 5

// 文字列置換用
typedef struct {
	char *src; // 置き換え元文字列
	char *dst; // 置き換え文字列
} strexchange_t;

// SACTEFAM を使ったマスク
typedef struct {
	int fd;       // SACTEFAM.KLD のファイルディスクプリタ
	char *mapadr; // mmap された最初のアドレス
	off_t size;   // mmap した大きさ
	int datanum;  // SACTEFAM.KLD 中のマスクファイルの数
	int *no;      // シナリオ側での番号
	int *offset;  // データへのオフセット
} SACTEFAM_t;


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

// スプライトのタイプ
enum spritetype {
	SPRITE_NORMAL = 0,
	SPRITE_SWITCH = 1,
	SPRITE_GETA   = 2,
	SPRITE_GETB   = 3,
	SPRITE_PUT    = 4,
	SPRITE_ANIME  = 5,
	SPRITE_SWPUT  = 6,
	SPRITE_MSG    =100,
	SPRITE_WP,
	SPRITE_NONE   =-1
};

// (前方参照用)
struct _sprite;

// スプライトに関する各種情報
struct _sprite {
	// スプライトのタイプ
	enum spritetype type;
	
	// スプライト番号
	int no;
	
	// それぞれの状態の時に鳴らすサウンド番号
	int numsound1, numsound2, numsound3;
	
	// 初期 sprite の大きさ(cg1の大きさ)
	MyDimension cursize;
	
	// それぞれの状態で表示する CG
	cginfo_t *cg1, *cg2, *cg3;
	
	// update するときに表示するcg
	cginfo_t *curcg;
	
	// スプライトを表示するか
	boolean show;
	boolean show_save; // Zkey hide save用
	
	// 表示する際のブレンド率 0:全く見えない, 255: 通常表示
	int blendrate; 
	
	// スプライトが Freeze されているか(0:No 1-3: その番号)
	int freezed_state;
	
	// 表示位置 (SetPos)
	MyPoint loc;
	
	// 現在のスプライトの表示位置
	MyPoint cur;
	
	// event callback
	int (* eventcb)(struct _sprite *sp, agsevent_t *e);  // for key/mouse
	int (* teventcb)(struct _sprite *sp, agsevent_t *e); // for timer
        // sprite削除時の callback
	void (* remove)(struct _sprite *sp);
	// spriteを再描画するときの callback
	int  (* update)(struct _sprite *sp);
	
	boolean focused; // forcusを得ているか
	boolean pressed; // このsprite上でマウスが押されているか
	
	GSList *expsp; // 説明スプライトのリスト
	
	// move command 用パラメータ
	struct {
		MyPoint to;     // 移動先
		int time;       // 移動完了時間
		int speed;      // 移動速度
		int starttime;  // 移動開始時刻
		int endtime;    // 移動終了予定時刻
		boolean moving; // 移動中かどうか
	} move;
	
	// SACT.Numeral用パラメータ
	struct {
		int cg[10];
		MyPoint pos;
		int span;
	} numeral;
	
	// スプライトの種類毎の情報
	union {
		// スイッチスプライト
		struct {
			
		} sw;
		
		// ゲットスプライト
		struct {
			boolean dragging;  // ドラッグ中
			MyPoint dragstart; // ドラッグ開始位置
		} get;

		// プットスプライト
		struct {
			
		} put;

		// アニメーションスプライト
		struct {
			int interval;      // １コマの間隔(10msec)
			int starttime;     // 開始時刻
			int npat;          // アニメコマ数(1/2/3)
			unsigned int tick; // カウンタ
		} anime;
		
		// メッセージスプライト
		struct {
			GSList    *buf;       // 表示する文字のリスト
			surface_t *canvas;    // 文字を描画するsurface
			MyPoint    dspcur;    // 現在の表示位置
		} msg;
	} u;
};
typedef struct _sprite sprite_t;

// SACT全体の情報
struct _sact {
	// SACTのバージョン
	int version;
	
	// スプライト全体
	sprite_t *sp[SPRITEMAX];
	
	GSList *sp_zhide;  // Zキーで消すスプライトのリスト
	GSList *sp_quake;  // Quakeで揺らすスプライトのリスト
	
	GSList *updatelist; // 再描画するスプライトのリスト
	
	cginfo_t *cg[CGMAX]; // cgまたはCG_xxで作った CG
	
	// 座標系の原点
	MyPoint origin;
	
	// 文字列 push/pop/replce 用
	GSList *strstack;
	GSList *strreplace;
	char   *strreplacesrc;
	char   *strreplacedst; 
	
	// メッセージスプライト用メッセージバッファ
	char msgbuf[MSGBUFMAX];
	char msgbuf2[MSGBUFMAX];
	
	// 選択ウィンド
	struct {
		char *elem[SEL_ELEMENT_MAX]; // 選択肢文字列
		int spno; // 背景スプライト番号
		int font_size; // 選択肢文字サイズ
		int font_type; // 選択肢フォント
		int frame_dot; // 枠スプライトの外側からのピクセル数
		int linespace; // 選択肢の行間
		int movecursor; // 初期選択
		int align; // 行そろえ
		void (* cbmove)(agsevent_t *);
		void (* cbrelease)(agsevent_t *);
		surface_t   *charcanvas;
	} sel;
	
	// event listener
	GSList *eventlisteners;
	GSList *teventlisteners;
	GSList *teventremovelist;
	
	// MOVEするスプライトのリスト
	GSList *movelist;
	int movestarttime; // 一斉に移動を開始するための開始時間
	int movecurtime;
	
	MyRectangle updaterect; // 更新が必要なspriteの領域の和
	
	// sact timer
	stimer_t timer[65536];
	
	// DnDに関するもの
	sprite_t *draggedsp;  // drag中のスプライト
	boolean dropped;      // スプライトがドロップされたかどうか
	
	// keywaitの種類
	int waittype;
	int waitkey;
	int sp_result_sw;
	int sp_result_get;
	int sp_result_put;
	
	// wait skip level
	//  0 通常キー待ち
	//  1 既読のみスキップ
	//  2 未読もスキップ
	int waitskiplv;
	
	// 範囲外をクリックしたときの音
	int numsoundob;
	
	// depth map
	surface_t *dmap;
	
	// SACTEFAM.KLD
	SACTEFAM_t am;
	
	boolean zhiding;  // Zkeyによる隠し中
	int     zofftime;
	boolean zdooff;
	
	// バックログ
	boolean logging;
	GList  *log;
};
typedef struct _sact sact_t;

// shortcut
#define sact sactprv
extern sact_t sact;

#endif /* __SACT_H__ */
