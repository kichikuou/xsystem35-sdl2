/*
 * sact.c: SACT
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
/* $Id: SACT.c,v 1.10 2004/10/31 04:18:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "imput.h"
#include "xsystem35.h"
#include "gametitle.h"
#include "message.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "sactcg.h"
#include "sactstring.h"
#include "sactsound.h"
#include "sactbgm.h"
#include "sactcrypto.h"
#include "sactchart.h"
#include "ngraph.h"
#include "surface.h"
#include "sactamask.h"

/*
  MTコマンドで設定された文字列によって、バージョン間の違いを吸収

  Version 1.0 : エスカレイヤー
          1.1 : Rance5D
          1.2(前期): 妻みぐい２
          1.2(後期): SACT開発キット, シェル・クレイル, NightDemon
*/ 

/*
  妻みぐい２キー説明

  メッセージスキップ(既読、未読関係なくスキップ) -> Ctrl
  自動メッセージ送り -> Aキー
    Aキーを押すことで、自動的にメッセージがすすんで行きます。(自動メッセージ
    送りがONになると、メッセージウィンドの右下にある入力待ちカーソル(>>>>)が
    消えます。解除する場合は、セリフもしくは音声終了まで、Aキーを押し続けて
    ください。Aキーを離したときに、入力待ちカーソルが表示されれば解除された
    ことになります。
  バックログ -> ホイール
*/


/*
   wNum の範囲など、引数のチェックは各サブシステム上でする。


実装確認事項
  SACT.CreateSprite はその呼ばれた瞬間のCGが使用される
    ->SACT.DrawまでにCGが変更されても、Create時のCGを使用
  OutputMessageはメッセージが表示終るまでもどってこない。
  QuakeScreenは終了までもどってこない。(キー抜けあり)
  自動改行はしない
  OutputMessage がきて始めて１文字づつ描画を行う。NewLineでは書かない。
  ~KEY 2: は直前の全てのSP_MOVEが終了するまでスイッチスプライト等は反応しない
  drag中にスイッチスプライトは反応しない
  アニメーションスプライトはつねに動作
  スイッチスプライトは、ボタンが押下状態でスプライト内に入って来たときも、
  cg3に変化する。このときボタンを離しても SpriteKeyWaitを抜けない。
  また、ボタンを押したままスプライト外に出ても、出た後はcg1に変化する
  GETA/BもフォーカスインでCG2にボタン押下でCG3に
  SWPUTはボタン押下で抜ける
*/


// SACT 関連の情報
sact_t sactprv;
extern char *xsys35_sact01;

/**
 * SACT.Init (1.0~)
 *   SACT全体の初期化
 */
void Init() {
	int p1 = getCaliValue(); /* ISys3x */
	
	// ゲームタイトルによるバージョン設定
	if (0 == strcmp(nact->game_title_name, GT_ESUKA)) {
		sact.version = 100;
	} else if (0 == strcmp(nact->game_title_name, GT_RANCE5D)){
		sact.version = 110;
	} else {
		sact.version = 120;
	}
	
	NOTICE("SACT version = %d\n", sact.version);
	
	// 初期座標原点
	sact.origin.x = 0;
	sact.origin.y = 0;
	
	// 各サブシステム初期化
	sp_init();
	sstr_init();
	ssel_init();
	stimer_init();
	ssnd_init();
	
	if (nact->files.sact01) {
		smask_init(nact->files.sact01);
	}
	
	// create depth map
	sact.dmap = sf_create_pixel(sf0->width, sf0->height, 16);
	
	// その他 System35 のデフォルト動作の変更
	nact->ags.font->antialiase_on = TRUE;
	sys_setHankakuMode(2); // 全角半角変換無し
	ags_autorepeat(FALSE); // key auto repeat off
	
	if (sact.version >= 120) {
		sact.logging = TRUE;
	} else {
		sact.logging = FALSE;
	}
	
	DEBUG_COMMAND("SACT.Init %d:\n", p1);
}

/**
 * SACT.CreateSprite (1.0~)
 *   スプライト作成
 *   @param wNum: スプライト番号
 *   @param wNumCG1: 通常表示するＣＧ番号
 *   @param wNumCG2: マウスカーソルを重ねたときのＣＧ番号
 *   @param wNumCG3: クリックしたときのＣＧ番号
 *   @param wType: スプライトのタイプ
 */
void CreateSprite() {
	int wNum    = getCaliValue();
	int wNumCG1 = getCaliValue();
	int wNumCG2 = getCaliValue();
	int wNumCG3 = getCaliValue();
	int wType   = getCaliValue();
	
	sp_new(wNum, wNumCG1, wNumCG2, wNumCG3, wType);
	
	DEBUG_COMMAND_YET("SACT.CreateSprite %d,%d,%d,%d,%d:\n", wNum, wNumCG1, wNumCG2, wNumCG3, wType);
}

/**
 * SACT.CreateTextSprite (1.0~)
 *   メッセージを表示するスプライトの作成
 *   @param wNum: スプライト番号
 *   @param wX: 表示位置のＸ座標
 *   @param wY: 表示位置のＹ座標
 *   @param wWidth: スプライトの幅
 *   @param wHeight: スプライトの高さ
 */
void CreateTextSprite() {
	int wNum = getCaliValue();
	int wX   = getCaliValue();
	int wY   = getCaliValue();
	int wWidth  = getCaliValue();
	int wHeight = getCaliValue();
	
	sp_new_msg(wNum, wX, wY, wWidth, wHeight);
	
	DEBUG_COMMAND_YET("SACT.CreateTextSprite %d,%d,%d,%d,%d:\n", wNum, wX, wY, wWidth, wHeight);
}

/**
 * SACT.SetWallPaper (1.0~)
 *   壁紙(画面背景)として表示するＣＧの設定
 *   @param wNum: 壁紙(背景)として表示するＣＧの番号
 */
void SetWallPaper() {
	int wNum = getCaliValue();
	
	sp_set_wall_paper(wNum);
	
	DEBUG_COMMAND_YET("SACT.SetWallPaper %d:\n", wNum);
}

/**
 * SACT.Clear (1.0~)
 *   全スプライト削除(~SP_CLR)
 */
void Clear() {
	sp_free_all();
	
	DEBUG_COMMAND_YET("SACT.Clear:\n");
}

/**
 * SACT.Delete (1.0~)
 *   スプライトの削除
 *   @param wNum: 削除するスプライト番号
 */
void Delete() {
	int wNum = getCaliValue();
	
	sp_free(wNum);
	
	DEBUG_COMMAND_YET("SACT.Delete %d:\n", wNum);
}

/**
 * SACT.SpriteDeleteCount (1.0~)
 *   wNum番からwCount個の範囲のスプライトの削除
 *   @param wNum: 先頭スプライト番号
 *   @param wCount: 範囲
 */
void SpriteDeleteCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_free(i);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteDeleteCount %d,%d:\n", wNum, wCount);
}

/**
 * SACT.Draw (1.0~)
 *   現在設定されているスプライト状態を画面に反映(~SP_UPDATE)
 */
void Draw() {
	sp_update_all(TRUE);

	DEBUG_COMMAND_YET("SACT.Draw:\n");
}

/**
 * SCAT.DrawEffect (1.0~)
 *   効果指定付き画面更新
 *   @param wType: エフェクトタイプ
 *   @param wEffectTime: エフェクトの時間(1/100秒単位)
 *   @param wEffectKey: キー抜け設定 (1.1~) (1で有効)
 */
void DrawEffect() {
	int wType       = getCaliValue();
	int wEffectTime = getCaliValue();
	int wEffectkey = 1;
	
	if (sact.version >= 110) {
		wEffectkey = getCaliValue();
	}
	
	sp_eupdate(wType, wEffectTime, wEffectkey);
	
	DEBUG_COMMAND_YET("SACT.DrawEffect %d,%d,%d:\n", wType, wEffectTime, wEffectkey);
}

/**
 * SCAT.DrawEffectAlphaMap (1.1~)
 *   αマスクつき画面更新
 *   @param nIndexAlphaMap: マスクα番号
 *   @param wEffectTime: エフェクトの時間(1/100秒単位)
 *   @param wEffectKey: キー抜け設定
 */
void DrawEffectAlphaMap() {
	int nIndexAlphaMap = getCaliValue();
	int wEffectTime = getCaliValue();
	int wEffectKey  = getCaliValue();
	
	sp_eupdate_amap(nIndexAlphaMap, wEffectTime, wEffectKey);
	
	DEBUG_COMMAND_YET("SACT.DrawEffectAlphaMap %d,%d,%d:\n", nIndexAlphaMap, wEffectTime, wEffectKey);
}

/**
 * SCAT.QuakeScreen (1.0~)
 *   画面揺らし
 *   @param wType: 0=縦横, 1:回転
 *   @param wParam1: wType=0のときx方向の振幅
 *                   wType=1のとき振幅
 *   @param wParam2: wType=0のときy方向の振幅
 *                   wType=1のとき回転数
 *   @param wCount: 時間(1/100秒)
 *   @param nfKeyEnable: キー抜け (1で有効) (1.1~) 
 */
void QuakeScreen() {
	int wType   = getCaliValue();
	int wParam1 = getCaliValue();
	int wParam2 = getCaliValue();
	int wCount  = getCaliValue();
	int nfKeyEnable = 1;
	
	if (sact.version >= 110) {
		nfKeyEnable = getCaliValue();
	}
	
	sp_quake_screen(wType, wParam1, wParam2, wCount, nfKeyEnable);
	
	DEBUG_COMMAND_YET("SACT.QuakeScreen %d,%d,%d,%d,%d:\n", wType, wParam1, wParam2, wCount, nfKeyEnable);
}

/**
 * SACT.SetOrigin (1.0~)
 *   基準座標変更
 *   @param wX: 原点にするＸ座標の位置
 *   @param wY: 原点にするＹ座標の位置
 */
void SetOrigin() {
	int wX = getCaliValue();
	int wY = getCaliValue();
	
	sact.origin.x = wX;
	sact.origin.y = wY;
	
	DEBUG_COMMAND_YET("SACT.SetOrigin %d,%d:\n", wX, wY);
}

/**
 * SACT.SetShow (1.0~)
 *   スプライトの表示状態の変更
 *   @param wNum: 先頭のスプライト番号
 *   @param wCount: 表示する個数
 *   @param wShow: 0:非表示, 1:表示
 */
void SetShow() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int wShow  = getCaliValue();
	
	sp_set_show(wNum, wCount, wShow);

	DEBUG_COMMAND_YET("SACT.SetShow %d,%d,%d:\n", wNum, wCount, wShow);
}

/**
 * SACT.SetBlendRate (1.1~)
 *   スプライトの表示状態の変更
 *   @param wNum: 先頭のスプライト番号
 *   @param wCount: 表示する個数
 *   @param nBlendRate: ブレンド率
 */
void SetBlendRate() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int nBlendRate  = getCaliValue();
	
	sp_set_blendrate(wNum, wCount, nBlendRate);
	
	DEBUG_COMMAND_YET("SACT.SetBlendRate %d,%d,%d:\n", wNum, wCount, nBlendRate);
}

/**
 * SACT.SetPos (1.0~)
 *   スプライトの表示位置を設定(変更)
 *   @param wNum: スプライト番号
 *   @param wX: 表示Ｘ座標
 *   @param wY: 表示Ｙ座標
 */
void SetPos() {
	int wNum = getCaliValue();
	int wX  = getCaliValue();
	int wY  = getCaliValue();
	
	sp_set_pos(wNum, wX, wY);
	
	DEBUG_COMMAND_YET("SACT.SetPos %d,%d,%d:\n", wNum, wX, wY);
}

/**
 * SACT.SetMove (1.0~)
 *   スプライトの移動 (すぐに処理を戻す)
 *   @param wNum: スプライト番号
 *   @param wX: 表示Ｘ座標
 *   @param wY: 表示Ｙ座標
 */
void SetMove() {
	int wNum = getCaliValue();
	int wX   = getCaliValue();
	int wY   = getCaliValue();
	
	sp_set_move(wNum, wX, wY);
	
	DEBUG_COMMAND_YET("SACT.SetMove %d,%d,%d:\n", wNum, wX, wY);
}

/**
 * SACT.SetMoveTime (1.0~)
 *   SetMoveによるスプライト移動の時間の設定
 *   @param wNum: スプライト番号
 *   @param wTime: 移動を完了するまでの時間(1/100秒単位)
 */
void SetMoveTime() {
	int wNum  = getCaliValue();
	int wTime = getCaliValue();
	
	sp_set_movetime(wNum, wTime);
	
	DEBUG_COMMAND_YET("SACT.SetMoveTime %d,%d:\n", wNum, wTime);
}

/**
 * SACT.SetMoveSpeed (1.0~)
 *   SetMoveによるスプライト移動の速度を設定
 *   @param wNum: スプライト番号
 *   @param wSpeed: 移動速度(デフォルトを100%とした%指定)
 */
void SetMoveSpeed() {
	int wNum   = getCaliValue();
	int wSpeed = getCaliValue();
	
	sp_set_movespeed(wNum, wSpeed);
	
	DEBUG_COMMAND_YET("SACT.SetMoveSpeed %d,%d:\n", wNum, wSpeed);
}

/**
 * SACT.SetMoveSpeedCount (1.0~)
 *   複数のスプライトに対するスプライト移動の速度の設定
 *   @param wNum: 先頭スプライト番号
 *   @param wCount: 範囲
 *   @param wSpeed: 移動速度(デフォルトを100%とした%指定)
 */
void SetMoveSpeedCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int wSpeed = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_set_movespeed(i, wSpeed);
	}
	
	DEBUG_COMMAND_YET("SACT.SetMoveSpeedCount %d,%d,%d:\n", wNum, wCount, wSpeed);
}

/**
 * SACT.SetSpriteAnimeTimeInterval (1.1~)
 *   複数のスプライトに対するアニメーションスプライトの間隔
 *   @param wNum: 先頭スプライト番号
 *   @param wCount: 範囲
 *   @param nTime: 間隔 
 */
void SetSpriteAnimeTimeInterval() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int nTime  = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_set_animeinterval(i, nTime);
	}
	
	DEBUG_COMMAND_YET("SACT.SetSpriteAnimeTimeInterval %d,%d,%d:\n", wNum, wCount, nTime);
}

/**
 * SACT.AddZKeyHideSprite (1.0~)
 *   キー入力待ちでZキーが押されたときに表示OFFになるスプライトの登録
 *   @param wNum: スプライト番号
 */
void AddZKeyHideSprite() {
	int wNum = getCaliValue();
	
	sp_add_zkey_hidesprite(wNum);
	
	DEBUG_COMMAND_YET("SACT.AddZKeyHideSprite %d:\n", wNum);
}

/**
 * SACT.ClearZKeyHideSprite (1.0~)
 *   AddZKeyHideSpriteで登録したスプライト番号を全てクリア
 */
void ClearZKeyHideSprite() {
	sp_clear_zkey_hidesprite_all();
	
	DEBUG_COMMAND_YET("SACT.ClearZKeyHideSprite:\n");
}

/**
 * SACT.SpriteFreeze (1.0~)
 *   スプライトスイッチをwIndexの状態で固定し、~KEY 2:などで反応しない
 *   ようにする
 *   @param wNum: スプライト番号
 *   @param wIndex: 固定する状態番号(1-3)
 */
void SpriteFreeze() {
	int wNum   = getCaliValue();
	int wIndex = getCaliValue();
	
	sp_freeze_sprite(wNum, wIndex);
	
	DEBUG_COMMAND_YET("SACT.SpriteFreeze %d,%d:\n", wNum, wIndex);
}

/**
 * SACT.SpriteThaw (1.0~)
 *   Freezeしたスプライト状態を解除
 *   @param wNum: スプライト番号
 */
void SpriteThaw() {
	int wNum = getCaliValue();
	
	sp_thaw_sprite(wNum);

	DEBUG_COMMAND_YET("SACT.SpriteThaw %d:\n", wNum);
}

/**
 * SACT.SpriteFreezeCount (1.0~)
 *   複数のスプライトをFreezeする
 *   @param wNum: 先頭スプライト番号
 *   @param wCount: 範囲
 *   @param wIndex: 固定する状態番号
 */
void SpriteFreezeCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int wIndex = getCaliValue();
	int i;

	for (i = wNum; i < (wNum + wCount); i++) {
		sp_freeze_sprite(i, wIndex);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteFreezeCount %d,%d,%d:\n", wNum, wCount, wIndex);
}

/**
 * SACT.SpriteThawCount (1.0~)
 *    複数のFreezeスプライト状態の解除
 *    @param wNum: 先頭スプライト番号
 *    @param wCount: 範囲
 */
void SpriteThawCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_thaw_sprite(i);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteThawCount %d,%d:\n", wNum, wCount);
}

/**
 * SACT.QuakeSpriteAdd (1.0~)
 *   QuakeSpriteで揺らすスプライトを追加
 *   @param wNum: スプライト番号
 */
void QuakeSpriteAdd() {
	int wNum = getCaliValue();
	
	sp_add_quakesprite(wNum);
	
	DEBUG_COMMAND_YET("SACT.QuakeSpriteAdd %d:\n", wNum);
}

/**
 * SACT.QuakeSpriteReset (1.0~)
 *   QuakeSpriteAddの設定を全て解除
 */
void QuakeSpriteReset() {
	sp_clear_quakesprite_all();
	DEBUG_COMMAND_YET("SACT.QuakeSpriteReset:\n");
}

/**
 * SACT.QuakeSprite (1.0~)
 *   QuakeSpriteAddで設定したスプライトを揺らす
 *   @param wType: 0:縦横(全てのスプライトを同じように揺らす)
 *                 1:縦横(全てのスプライトをバラバラに揺らす)
 *   @param wAmplitudeX: Ｘ方向の振幅
 *   @param wAmplitudeY: Ｙ方向の振幅
 *   @param wCount: 時間(1/100秒)
 *   @param nfKeyEnable: (1.1~): キーキャンセルあり(=1)
 */
void QuakeSprite() {
	int wType       = getCaliValue();
	int wAmplitudeX = getCaliValue();
	int wAmplitudeY = getCaliValue();
	int wCount      = getCaliValue();
	int nfKeyEnable = 0;

	if (sact.version >= 110) {
		nfKeyEnable = getCaliValue();
	}
	
	sp_quake_sprite(wType, wAmplitudeX, wAmplitudeY, wCount, nfKeyEnable);
	
	DEBUG_COMMAND_YET("SACT.QuakeSprite %d,%d,%d,%d:\n", wType, wAmplitudeX, wAmplitudeY, wCount);
}

/**
 * SACT.QuerySpriteIsExist (1.0~)
 *  指定のスプライトが登録されているかどうかを取得
 *  @param wNum: スプライト番号
 *  @param var: 登録状態を返す変数 0: 未登録, 1:既登録
 */
void QuerySpriteIsExist() {
	int wNum = getCaliValue();
	int *var = getCaliVariable();

	sp_query_isexist(wNum, var);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteIsExist %d,%p:\n", wNum, var);
}

/**
 * SACT.QuerySpriteInfo (1.0~)
 *   スプライトの情報を取得
 *   @param wNum: スプライト番号
 *   @param vType: スプライトタイプ。テキストスプライトの場合は100
 *   @param vCG1: ＣＧ１(ない場合は０)
 *   @param vCG2: ＣＧ２(ない場合は０)
 *   @param vCG3: ＣＧ３(ない場合は０)
 */
void QuerySpriteInfo() {
	int wNum   = getCaliValue();
	int *vType = getCaliVariable();
	int *vCG1  = getCaliVariable();
	int *vCG2  = getCaliVariable();
	int *vCG3  = getCaliVariable();
	
	sp_query_info(wNum, vType, vCG1, vCG2, vCG3);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteInfo %d,%p,%p,%p,%p:\n", wNum, vType, vCG1, vCG2, vCG3);
}

/**
 * SACT.QuerySpriteShow (1.0~)
 *   スプライトの表示状態(SP_SHOWの値)を取得
 *   @param wNum: スプライト番号
 *   @param vShow: 0:非表示, 1:表示
 */
void QuerySpriteShow() {
	int wNum = getCaliValue();
	int *vShow = getCaliVariable();

	sp_query_show(wNum, vShow);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteShow %d,%p:\n", wNum, vShow);
}

/**
 * SACT.QuerySpritePos (1.0~)
 *   スプライトの表示位置の取得
 *   @param wNum: スプライト番号
 *   @param vX: Ｘ座標
 *   @param vY: Ｙ座標
 */
void QuerySpritePos() {
	int wNum = getCaliValue();
	int *vX  = getCaliVariable();
	int *vY  = getCaliVariable();
	
	sp_query_pos(wNum, vX, vY);
	
	DEBUG_COMMAND_YET("SACT.QuerySpritePos %d,%p,%p:\n", wNum, vX, vY);
}

/**
 * SACT.QuerySpriteSize (1.0~)
 *   スプライトの大きさの取得
 *   @param wNum: スプライト番号
 *   @param vWidth: スプライトの幅
 *   @param vHeight: スプライトの高さ
 */
void QuerySpriteSize() {
	int wNum     = getCaliValue();
	int *vWidth  = getCaliVariable();
	int *vHeight = getCaliVariable();
	
	sp_query_size(wNum, vWidth, vHeight);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteSize %d,%p,%p:\n", wNum, vWidth, vHeight);
}

/**
 * SACT.QueryTextPos (1.2~)
 *   メッセージスプライトの文字位置
 *   @param nMesSpID:
 *   @param pwX:
 *   @param pwY:
 */
void QueryTextPos() {
	int wNum = getCaliValue();
	int *vX  = getCaliVariable();
	int *vY  = getCaliVariable();
	
	sp_query_textpos(wNum, vX, vY);
	
	DEBUG_COMMAND_YET("SACT.QueryTextPos %d,%p,%p:\n", wNum, vX, vY);
}

/**
 * SCAT.CG_Clear (1.0~)
 *   CG_Createで作成したCGを全て削除
 */
void CG_Clear() {
	scg_freeall();
	
	DEBUG_COMMAND_YET("SACT.CG_Clear:\n");
}

/**
 * SACT.CG_Reset (1.0~)
 *   CG_Createで作成したCGを削除
 *   @param wNumCG: 削除するCG番号
 */
void CG_Reset() {
	int wNumCG = getCaliValue();
	
	scg_free(wNumCG);
	
	DEBUG_COMMAND_YET("SACT.CG_Reset %d:\n", wNumCG);
}

/**
 * SACT.CG_QueryType (1.0~)
 *   CGの状態(CGのタイプ)を取得
 *   @param wNumCG: CG番号
 *   @param vType: CGの種類, 0: 未使用, 1:リンクされている, 2: CG_SETで作成
 *                 3: CG_REVERSEで作成, 4: CG_STRETCHで作成
 */
void CG_QueryType() {
	int wNumCG = getCaliValue();
	int *vType = getCaliVariable();
	
	scg_querytype(wNumCG, vType);
	
	DEBUG_COMMAND_YET("SACT.CG_QueryType %d,%p:\n", wNumCG, vType);
}

/**
 * SACT.CG_QuerySize (1.0~)
 *   CGの大きさを取得
 *   @param wNumCG: CG番号
 *   @param vWidth: 幅
 *   @param vHeight: 高さ
 */
void CG_QuerySize() {
	int wNumCG   = getCaliValue();
	int *vWidth  = getCaliVariable();
	int *vHeight = getCaliVariable();
	
	scg_querysize(wNumCG, vWidth, vHeight);
	
	DEBUG_COMMAND_YET("SACT.CG_QuerySize %d,%p,%p:\n", wNumCG, vWidth, vHeight);
}

/**
 * SACT.CG_QueryBpp (1.0~)
 *   CGのbppを取得
 *   @param wNumCG: CG番号
 *   @param vBpp: CGのbpp
 */
void CG_QueryBpp() {
	int wNumCG = getCaliValue();
	int *vBpp  = getCaliVariable();
	
	scg_querybpp(wNumCG, vBpp);
	
	DEBUG_COMMAND_YET("SACT.CG_QueryBpp %d,%p:\n", wNumCG, vBpp);
}

/**
 * SACT.CG_ExistAlphaMap (1.0~)
 *   CGのαマップ(マスク)があれば１、なければ０
 *   @param wNumCG: CG番号
 *   @param vMask: 0/1
 */
void CG_ExistAlphaMap() {
	int wNumCG = getCaliValue();
	int *vMask = getCaliVariable();
	
	scg_existalphamap(wNumCG, vMask);
	
	DEBUG_COMMAND_YET("SACT.CG_ExistAlphaMap %d,%p:\n", wNumCG, vMask);
}

/**
 * SACT.CG_Create (1.0~)
 *   指定サイズ、色、ブレンド率の四角を表示する
 *   @param wNumCG: CG番号
 *   @param wWidth: 幅
 *   @param wHeight: 高さ
 *   @param wR: RGB値の赤(0-255)
 *   @param wG: RGB値の緑(0-255)
 *   @param wB: RGB値の青(0-255)
 *   @param wBlendRate: ブレンド率(0-255)
 */
void CG_Create() {
	int wNumCG     = getCaliValue();
	int wWidth     = getCaliValue();
	int wHeight    = getCaliValue();
	int wR         = getCaliValue();
	int wG         = getCaliValue();
	int wB         = getCaliValue();
	int wBlendRate = getCaliValue();
	
	scg_create(wNumCG, wWidth, wHeight, wR, wG, wB, wBlendRate);
	
	DEBUG_COMMAND_YET("SACT.CG_Create %d,%d,%d,%d,%d,%d,%d:\n", wNumCG, wWidth, wHeight, wR, wG, wB, wBlendRate);
}

/**
 * SACT,CG_CreateReverse (1.0~)
 *   元になるCGを反転したCGを作成する
 *   @param wNumCG: CG番号
 *   @param wNumSrcCG: コピーの元になるCGの番号
 *   @param wReverseX: X方向の反転スイッチ(0:反転しない、1:反転する)
 *   @param wReverseY: Y方向の反転スイッチ(0:反転しない、1:反転する)
 */
void CG_CreateReverse() {
	int wNumCG = getCaliValue();
	int wNumSrcCG = getCaliValue();
	int wReverseX = getCaliValue();
	int wReverseY = getCaliValue();

	scg_create_reverse(wNumCG, wNumSrcCG, wReverseX, wReverseY);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateReverse %d,%d,%d,%d:\n", wNumCG, wNumSrcCG, wReverseX, wReverseY);
}

/**
 * SACT.CG_CreateStretch (1.0~)
 *   元になるCGを拡大もしくは縮小したCGを作成する
 *   @param wNumCG: CG番号
 *   @param wWidth: 作成するCGの幅
 *   @param wHeight: 作成するCGの高さ
 *   @param wNumSrcCG: 元になるCGの番号
 */
void CG_CreateStretch() {
	int wNumCG    = getCaliValue();
	int wWidth    = getCaliValue();
	int wHeight   = getCaliValue();
	int wNumSrcCG = getCaliValue();

	scg_create_stretch(wNumCG, wWidth, wHeight, wNumSrcCG);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateStretch %d,%d,%d,%d:\n", wNumCG, wWidth, wHeight, wNumSrcCG);
}

/**
 * SACT.CG_CreateBlend (1.0~)
 *   ２枚のCGをかさねあわせたCGを作成
 *   @param wNumDstCG: CG番号(作成先)
 *   @param wNumBaseCG: 重ね合わせのもととなるCG
 *   @param wX: 重ね合わせる位置のＸ座標
 *   @param wY: 重ね合わせる位置のＹ座標
 *   @param wNumBlendCG: 上に重ね合わせるCG
 *   @param wAlphaMapMode: αマップの作成モード
 */
void CG_CreateBlend() {
	int wNumDstCG  = getCaliValue();
	int wNumBaseCG = getCaliValue();
	int wX = getCaliValue();
	int wY = getCaliValue();
	int wNumBlendCG   = getCaliValue();
	int wAlphaMapMode = getCaliValue();
	
	DEBUG_COMMAND_YET("SACT.CG_CreateBlend %d,%d,%d,%d,%d,%d:\n", wNumDstCG, wNumBaseCG, wX, wY, wNumBlendCG, wAlphaMapMode);
	scg_create_blend(wNumDstCG, wNumBaseCG, wX, wY, wNumBlendCG, wAlphaMapMode);
	
}

/**
 * SACT.CG_CreateText (1.0~)
 *   文字列からCGを作成
 *   @param wNumCG: 作成するCG番号
 *   @param wSize: 文字の高さ(pixel)
 *   @param wR: 文字のR値(0-255)
 *   @param wG: 文字のG値(0-255)
 *   @param wB: 文字のB値(0-255)
 *   @param wText: 描画する文字列変数の番号
 */
void CG_CreateText() {
	int wNumCG = getCaliValue();
	int wSize  = getCaliValue();
	int wR     = getCaliValue();
	int wG     = getCaliValue();
	int wB     = getCaliValue();
	int wText  = getCaliValue();
	
	scg_create_text(wNumCG, wSize, wR, wG, wB, wText);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateText %d,%d,%d,%d,%d,%d:\n", wNumCG, wSize, wR, wG, wB, wText);
}

/**
 * SACT.CG_CreateTextNum (1.0~)
 *   数値からシステムテキストのCGを作成
 *   @param wNumCG: 作成するCG番号
 *   @param wSize: 文字の高さ(pixel)
 *   @param wR: 文字のR値(0-255)
 *   @param wG: 文字のG値(0-255)
 *   @param wB: 文字のB値(0-255)
 *   @param wFigs: 桁数
 *   @param wZeroPadding: 桁数に満たない部分０で埋めるかどうかのフラグ
 *                        0:ゼロ埋めしない 1:ゼロ埋めする
 *   @param wValue: 描画する値
 */
void CG_CreateTextNum() {
	int wNumCG       = getCaliValue();
	int wSize        = getCaliValue();
	int wR           = getCaliValue();
	int wG           = getCaliValue();
	int wB           = getCaliValue();
	int wFigs        = getCaliValue();
	int wZeroPadding = getCaliValue();
	int wValue       = getCaliValue();
	
	scg_create_textnum(wNumCG, wSize, wR, wG, wB, wFigs, wZeroPadding, wValue);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateTextNum %d,%d,%d,%d,%d,%d,%d,%d:\n", wNumCG, wSize, wR, wG, wB, wFigs, wZeroPadding, wValue);
}

/**
 * SACT.CG_Copy (1.0~)
 *   CGを複製
 *   @param wNumDst: 複写先のCG番号
 *   @param wNumSrc: 複写元のCG番号
 */
void CG_Copy() {
	int wNumDst = getCaliValue();
	int wNumSrc = getCaliValue();
	
	scg_copy(wNumDst, wNumSrc);

	DEBUG_COMMAND_YET("SACT.CG_Copy %d,%d:\n", wNumDst, wNumSrc);
}

/**
 * SACT.CG_Cut (1.0~)
 *   元のCGの一部を切りぬいたCGを作成
 *   @param wNumDstCG: CG番号(作成先)
 *   @param wNumSrcCG: CG番号(カット元)
 *   @param wX: カット開始Ｘ座標
 *   @param wY: カット開始Ｙ座標
 *   @param wWidth: カット幅
 *   @param wHeight: カット高さ
 */
void CG_Cut() {
	int wNumDstCG = getCaliValue();
	int wNumSrcCG = getCaliValue();
	int wX = getCaliValue();
	int wY = getCaliValue();
	int wWidth  = getCaliValue();
	int wHeight = getCaliValue();
	
	scg_cut(wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
	
	DEBUG_COMMAND_YET("SACT.CG_Cut %d,%d,%d,%d,%d,%d:\n", wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
}

/**
 * SACT.CG_PartCopy (1.0~)
 *   元のCGの一部を切りぬいたCGを作成、CGのサイズ自体はもとのままで、
 *   マスクデータのみを処理して見掛け上のサイズを変化させる
 *   @param wNumDstCG: CG番号(作成先)
 *   @param wNumSrcCG: CG番号(元)
 *   @param wX: 開始Ｘ座標
 *   @param wY: 開始Ｙ座標
 *   @param wWidth: カット幅
 *   @param wHeight: カット高さ
 */
void CG_PartCopy() {
	int wNumDstCG = getCaliValue();
	int wNumSrcCG = getCaliValue();
	int wX = getCaliValue();
	int wY = getCaliValue();
	int wWidth  = getCaliValue();
	int wHeight = getCaliValue();
	
	scg_partcopy(wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
	
	DEBUG_COMMAND_YET("SACT.PartCopy %d,%d,%d,%d,%d,%d:\n", wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
}

/**
 * SACT.WiatKeySimple (1.0~)
 *   通常キー入力待ち
 *   @param vKey: 入力されたキー
 */
void WaitKeySimple() {
	int *vKey = getCaliVariable();

	DEBUG_COMMAND_YET("SACT.WaitKeySimple %d:\n", vKey);

	// とりあえず全更新
	sp_update_all(TRUE);
	
	sact.waittype = KEYWAIT_SIMPLE;
	sact.waitkey = -1;
	
	while(sact.waitkey == -1) {
		sys_keywait(25, TRUE);
	}
	
	sact.waittype = KEYWAIT_NONE;
	
	*vKey = sact.waitkey;
	
}

/**
 * SACT.WaitKeyMessgae (1.0~)
 *   メッセージキー入力待ち
 *   @param wMessageMark1: スプライト番号1(アニメーションスプライト)
 *   @param wMessageMark2: スプライト番号2(アニメーションスプライト)
 *   @param wMessageLength: (1.2~)
 */
void WaitKeyMessage() {
	int wMessageMark1 = getCaliValue();
	int wMessageMark2 = getCaliValue();
	int wMessageLength = 0;
	
	if (sact.version >= 120) {
		wMessageLength = getCaliValue();
	}
	
	smsg_keywait(wMessageMark1, wMessageMark2, wMessageLength);
	
	DEBUG_COMMAND_YET("SACT.WaitKeyMessage %d,%d,%d:\n", wMessageMark1, wMessageMark2, wMessageLength);
}

/**
 * SACT.WaitKeySprite (1.0~)
 *   スプライト処理待ち
 *   @param vOK: 0ならば右クリック 
 *   @param vRND: キー入力結果
 *   @param vRsv1: 予約
 *   @param vRsv2: 予約
 */
void WaitKeySprite() {
	int *vOK = getCaliVariable();
	int *vRND = getCaliVariable();
	int *vRsv1 = getCaliVariable();
	int *vRsv2 = getCaliVariable();
	
	DEBUG_COMMAND("SACT.WaitKeySprite %p,%p,%p,%p:\n", vOK, vRND, vRsv1, vRsv2);
	
	sp_keywait(vOK, vRND, vRsv1, vRsv2, NULL, -1);
	
	DEBUG_COMMAND_YET("SACT.WaitKeySprite %d,%d,%d,%d:\n", *vOK, *vRND, *vRsv1, *vRsv2);
}

/**
 * SACT.PeekKey (1.2~)
 *   ?????
 *   @param nKeyCode:
 *   @param vResult:
 */
void PeekKey() {
	int nKeyCode = getCaliValue();
	int *vResult = getCaliVariable();
	
	WARNING("NOT IMPLEMENTED\n");
	DEBUG_COMMAND_YET("SACT.PeekKey %d,%p:\n", nKeyCode, vResult);
}

/**
 * SACT.WaitKeySKipKeyUp (1.0~)
 *   文字送りキーが押されっぱなしの時、離されるまで待つ
 */
void WaitMsgSkipKeyUp() {
	WARNING("NOT IMPLEMENTED\n");
	DEBUG_COMMAND_YET("SACT.WaitMsgSkipKeyUp:\n");
}

/**
 * SACT.WaiKeySimpleTimeOut (1.0~)
 *   タイムアウトつきキーウェイト
 *   @param vRND: 入力されたキーコード
 *   @param vD03: タイムアウトした場合=1, しない場合=0
 *   @param wTime: タイムアウト時間 (1/100sec)
 */
void WaitKeySimpleTimeOut() {
	int *vRND = getCaliVariable();
	int *vD03 = getCaliVariable();
	int wTime = getCaliValue();

	sact.waittype = KEYWAIT_SIMPLE;
	sact.waitkey = -1;
	
	sys_keywait(wTime * 10, TRUE);
	if (sact.waitkey == -1) {
		*vD03 = 1;
		*vRND = 0;
	} else {
		*vD03 = 0;
		*vRND = sact.waitkey;
	}
	
	sact.waittype = KEYWAIT_NONE;
	
	DEBUG_COMMAND_YET("SACT.WaitKeySimpleTimeOut %p,%p,%d:\n", vRND, vD03, wTime);
}

/**
 * SACT.WaiKeySpriteTimeOut (1.0~)
 *   タイムアウトつきスプライトキーウェイト 
 *   @param vOK: 0 の時右クリック
 *   @param vRND: スイッチスプライトの番号
 *   @param vD01: ゲットスプライトの番号
 *   @param vD02: プットスプライトの番号
 *   @param vD03: タイムアウトした場合=1, しない場合=0
 *   @param wTime: タイムアウト時間 (1/100sec)
 */
void WaitKeySpriteTimeOut() {
	int *vOK = getCaliVariable();
	int *vRND = getCaliVariable();
	int *vD01 = getCaliVariable();
	int *vD02 = getCaliVariable();
	int *vD03 = getCaliVariable();
	int wTime = getCaliValue();
	
	sp_keywait(vOK, vRND, vD01, vD02, vD03, wTime);
	
	DEBUG_COMMAND_YET("SACT.WaitKeySpriteTimeOut %p,%p,%p,%p,%p,%d:\n", vOK, vRND, vD01, vD02, vD03, wTime);
}

/**
 * SACT.QueryMessageSkip (1.0~)
 *   ??????
 *   @param vSkip:
 */
void QueryMessageSkip() {
	int *vSkip = getCaliVariable();

	*vSkip = get_skipMode() ? 1 : 0;
	
	DEBUG_COMMAND_YET("SACT.QueryMessageSkip %p:\n", vSkip);
}

/**
 * SACT.RegistReplaceString (1.0~)
 *   メッセージ中の文字列の置き換え
 *   @param sstr: 変換元文字列番号
 *   @param dstr: 変換先文字列番号
 */
void RegistReplaceString() {
	int sstr = getCaliValue();
	int dstr = getCaliValue();
	
	sstr_regist_replace(sstr, dstr);
	
	DEBUG_COMMAND_YET("SACT.RegistReplaceString %d,%d:\n", sstr, dstr);
}

/**
 * SACT.MessageOutput (1.0~)
 *   @param wMessageSpriteNumber: メッセージを表示するメッセージスプライト番号
 *                                (~MES)
 *   @param wMessageSize: フォントの大きさ (~MES_SIZE|~MES_SET)
 *   @param wMessageColorR: メッセージの色(Red) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorG: メッセージの色(Green) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorB: メッセージの色(Blue) (~MES_SET|~MES_COLOR)
 *   @param wMessageFont: メッセージのフォント(0:ゴシック, 1:明朝)
 *                         (~MES_FONT)
 *   @param wMessageSpeed: メッセージの表示速度(0:ウェイト無し, 1:速い,
 *                          2:中くらい, 3: 遅い) (~MES_SPEED) (msec)
 *   @param wMessageLineSpace: 行間スペース (~MES_SPC_Y)
 *   @param wMessageAlign: 行そろえ (1.1~)
 *   @param vMessageLength: ???     (1.2~)
 */
void MessageOutput() {
	int wMessageSpriteNumber = getCaliValue();
	int wMessageSize   = getCaliValue();
	int wMessageColorR = getCaliValue();
	int wMessageColorG = getCaliValue();
	int wMessageColorB = getCaliValue();
	int wMessageFont   = getCaliValue();
	int wMessageSpeed  = getCaliValue();
	int wMessageLineSpace = getCaliValue();
	int wMessageAlign   = 0;
	int *vMessageLength  = NULL;
	
	if (sact.version >= 110) {
		wMessageAlign  = getCaliValue();
	}
	if (sact.version >= 120) {
		vMessageLength  = getCaliVariable();
	}
	
	smsg_out(wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace, wMessageAlign, 0, 0, 0, vMessageLength);
	
	DEBUG_COMMAND_YET("SACT.MessageOutput %d,%d,%d,%d,%d,%d,%d,%d,%d,%p:\n", wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace, wMessageAlign, vMessageLength);
}

/**
 * SACT.MessageOutputEx (1.1~)
 *   ルビつきメッセージ出力
 *   @param wMessageSpriteNumber: メッセージを表示するメッセージスプライト番号
 *                                (~MES)
 *   @param wMessageSize: フォントの大きさ (~MES_SIZE|~MES_SET)
 *   @param wMessageColorR: メッセージの色(Red) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorG: メッセージの色(Green) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorB: メッセージの色(Blue) (~MES_SET|~MES_COLOR)
 *   @param wMessageFont: メッセージのフォント(0:ゴシック, 1:明朝)
 *                         (~MES_FONT)
 *   @param wMessageSpeed: メッセージの表示速度(0:ウェイト無し, 1:速い,
 *                          2:中くらい, 3: 遅い) (~MES_SPEED)
 *   @param wMessageLineSpace: 行間スペース (~MES_SPC_Y)
 *   @param wMessageAlign: 行そろえ
 *   @param wRubySize:     ルビ文字の大きさ
 *   @param wRubyFont:     ルビ文字のフォントの種類((0:ゴシック, 1:明朝)
 *   @param wRubyLineSpace: ルビ文字とメッセージの行間スペース
 *   @param vLength: ???    (1.2~)
 */
void MessageOutputEx() {
	int wMessageSpriteNumber = getCaliValue();
	int wMessageSize   = getCaliValue();
	int wMessageColorR = getCaliValue();
	int wMessageColorG = getCaliValue();
	int wMessageColorB = getCaliValue();
	int wMessageFont   = getCaliValue();
	int wMessageSpeed  = getCaliValue();
	int wMessageLineSpace = getCaliValue();
	int wMessageAlign  = getCaliValue();
	int wRubySize      = getCaliValue();
	int wRubyFont      = getCaliValue();
	int wRubyLineSpace = getCaliValue();
	int *vLength = NULL;
	
	if (sact.version >= 120) {
		vLength = getCaliVariable();
	}
	
	smsg_out(wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace, wMessageAlign, wRubySize, wRubyFont, wRubyLineSpace, vLength);
	
	DEBUG_COMMAND_YET("SACT.MessageOutputEx %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%p:\n", wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace,wMessageAlign, wRubySize, wRubyFont, wRubyLineSpace, vLength);
}

/**
 * SACT.MessageNewLine (1.0~)
 *   改行(Ｒコマンド相当) (~MES_NEW_LINE)
 *   @param wMessageSpriteNumber: メッセージスプライト番号
 *   @param wMessageSize: フォントの大きさ (~MES_SIZE|~MES_SET)
 */
void MessageNewLine() {
	int wMessageSpriteNumber = getCaliValue();
	int wMessageSize = getCaliValue();
	
	smsg_newline(wMessageSpriteNumber, wMessageSize);
	
	DEBUG_COMMAND_YET("SACT.MessageNewLine %d,%d:\n", wMessageSpriteNumber, wMessageSize);
}

/**
 * SACT.MessageClear (1.0~)
 *   メッセージ領域の消去(Aコマンド相当)
 *   @param wMessageSpriteNumber: メッセージスプライト番号
 */ 
void MessageClear() {
	int wMessageSpriteNumber = getCaliValue();
	
	smsg_clear(wMessageSpriteNumber);
	
	DEBUG_COMMAND_YET("SACT.MessageClear %d:\n", wMessageSpriteNumber);
}

/**
 * SACT.MessageIsEmpty  (1.0~)
 *   メッセージが残っている場合 wResult に 0 を返す?
 *   @param wResult: 結果を返す変数
 */
void MessageIsEmpty() {
	int *wResult = getCaliVariable();

	*wResult = smsg_is_empty();
	
	DEBUG_COMMAND_YET("SACT.MessageIsEmpty %p:\n", wResult);
}

/**
 * SACT.MessagePeek  (1.2+~) (妻みぐい２にはない)
 *   メッセージバッファの内容を取得する
 *   @param vCount: 取得した行数
 *   @param nTopStringNum: バッファを取得する文字列変数の最初
 */
void MessagePeek() {
	int *vCount = getCaliVariable();
	int nTopStringNum = getCaliValue();

	WARNING("NOT IMPLEMENTED\n");
	
	DEBUG_COMMAND_YET("SACT.MessagePeek %p,%d:\n", vCount, nTopStringNum);
}

/**
 * SACT.Log_Stop (1.2~)
 *   ログ採取停止
 */
void Log_Stop() {
	sact.logging = FALSE;
	DEBUG_COMMAND_YET("SACT.Log_Stop:\n");
}

/**
 * SACT.Log_Start (1.2~)
 *   ログ採取開始
 */
void Log_Start() {
	sact.logging = TRUE;
	DEBUG_COMMAND_YET("SACT.Log_Start:\n");
}

/**
 * SACT.MenuClear (1.0~)
 *   SACT内部の選択肢情報をクリア
 */
void MenuClear() {
	ssel_clear();
	
	DEBUG_COMMAND_YET("SACT.MenuClear:\n");
}

/**
 * SACT.MenuAdd (1.0~)
 *   登録文字列をSACT内部選択肢情報に追加
 *   @param nString: 登録する文字列変数番号
 *   @param wI: 登録する位置 (1-)
 */
void MenuAdd() {
	int nString = getCaliValue();
	int wI = getCaliValue();

	ssel_add(nString, wI);
	
	DEBUG_COMMAND_YET("SACT.MenuAdd %d,%d:\n", nString, wI);
}

/**
 * SACT.MenuOpen (1.0~)
 *   SACT内部選択ループ
 *   @param wMenuResult: 選択結果(番号) キャンセルしたら0
 *   @param wNum: 枠,背景とするスプライト番号 (~SP_SEL)
 *   @param wChoiceSize: 選択肢文字サイズ (~SEL_SIZE)
 *   @param wMenuOutSpc: 枠スプライトの外側からのピクセル数(~SP_SETSELSPC)
 *   @param wChoiceLineSpace: 選択肢の行間(1に固定?)
 *   @param wChoiceAutoMoveCursor: オープン時に自動的に移動する選択肢の番号
 *   @param nAlign: 行そろえ (0:左, 1:中央, 2: 右) (1.1~)
 */
void MenuOpen() {
	int *wMenuResult = getCaliVariable();
	int wNum         = getCaliValue();
	int wChoiceSize  = getCaliValue();
	int wMenuOutSpc  = getCaliValue();
	int wChoiceLineSpace = getCaliValue();
	int wChoiceAutoMoveCursor = getCaliValue();
	int nAlign = 0;
	
	if (sact.version >= 110) {
		nAlign = getCaliValue();
	}
	
	*wMenuResult = ssel_select(wNum, wChoiceSize, wMenuOutSpc, wChoiceLineSpace, wChoiceAutoMoveCursor, nAlign);
	
	DEBUG_COMMAND_YET("SACT.MenuOpen %p,%d,%d,%d,%d,%d,%d:\n", wMenuResult, wNum, wChoiceSize, wMenuOutSpc, wChoiceLineSpace, wChoiceAutoMoveCursor, nAlign);
}

/**
 * SACT.PushString (1.0~)
 *   SACT内部に文字列変数をプッシュ
 *   @param nString: 文字列変数番号
 */
void PushString() {
	int nString = getCaliValue();
	
	sstr_push(nString);
	
	DEBUG_COMMAND_YET("SACT.PushString %d:\n", nString);
}

/**
 * SACT.PopString (1.0~)
 *   SACT内部にプッシュした文字列変数をポップ
 *   @param nString: 文字列変数番号
 */
void PopString() {
	int nString = getCaliValue();

	sstr_pop(nString);
	
	DEBUG_COMMAND_YET("SACT.PopString %d:\n", nString);
}

/**
 * SACT.Numeral_XXXX
 *   スプライト毎に、指定の数字(0〜9)に対して対応するCG番号や
 *   表示位置、間隔などを格納・取り出しを行う
 */

/**
 * SACT.Numeral_SetCG (1.0~)
 *   指定の数値に対するCG番号の設定
 *   @param nNum: スプライト番号
 *   @param nIndex: 数字(0-9)
 *   @param nCG: 数字に対応するCG番号
 */
void Numeral_SetCG() {
	int nNum = getCaliValue();
	int nIndex = getCaliValue();
	int nCG = getCaliValue();
	
	sp_num_setcg(nNum, nIndex, nCG);
	
	DEBUG_COMMAND_YET("SACT.Numeral_SetCG %d,%d,%d:\n", nNum, nIndex, nCG);
}

/**
 * SACT.Numeral_GetCG (1.0~)
 *   指定の数値に対するCG番号の設定
 *   @param nNum: スプライト番号
 *   @param nIndex: 数字(0-9)
 *   @param vCG: 設定されているCG番号を返す変数
 */
void Numeral_GetCG() {
	int nNum = getCaliValue();
	int nIndex = getCaliValue();
	int *vCG = getCaliVariable();
	
	sp_num_getcg(nNum, nIndex, vCG);
	
	DEBUG_COMMAND_YET("SACT.Numeral_GetCG %d,%d,%p:\n", nNum, nIndex, vCG);
}

/**
 * SACT.Numeral_SetPos (1.0~)
 *   Numeralの表示位置の設定
 *   @param nNum: スプライト番号
 *   @param nX: 表示Ｘ座標
 *   @param ny: 表示Ｙ座標
 */
void Numeral_SetPos() {
	int nNum = getCaliValue();
	int nX = getCaliValue();
	int nY = getCaliValue();
	
	sp_num_setpos(nNum, nX, nY);
	
	DEBUG_COMMAND_YET("SACT.Numeral_SetPos %d,%d,%d:\n", nNum, nX, nY);
}

/**
 * SACT.Numeral_GetPos (1.0~)
 *   Numeral_SetPosで設定した座標の取り出し
 *   @param nNum: スプライト番号
 *   @param vX: Ｘ座標を格納する変数
 *   @param vY: Ｙ座標を格納する変数
 */
void Numeral_GetPos() {
	int nNum = getCaliValue();
	int *vX = getCaliVariable();
	int *vY = getCaliVariable();
	
	sp_num_getpos(nNum, vX, vY);
	
	DEBUG_COMMAND_YET("SACT.Numeral_GetPos %d,%p,%p:\n", nNum, vX, vY);
}

/**
 * SACT.Numeral_SetSpan (1.0~)
 *   Numeralの間隔(Span)の設定
 *   @param nNum: スプライト番号
 *   @param nSpan: 間隔
 */
void Numeral_SetSpan() {
	int nNum = getCaliValue();
	int nSpan = getCaliValue();
	
	sp_num_setspan(nNum, nSpan);
	
	DEBUG_COMMAND_YET("SACT.Numeral_SetSpan %d,%d:\n", nNum, nSpan);
}

/**
 * SACT.Numeral_GetSpan (1.0~)
 *   Numeral_SetSpanで設定した値の取り出し
 *   @param nNum: スプライト番号
 *   @param vSpan: 値を格納する変数
 */
void Numeral_GetSpan() {
	int nNum = getCaliValue();
	int *vSpan = getCaliVariable();

	sp_num_getspan(nNum, vSpan);
	
	DEBUG_COMMAND_YET("SACT.Numeral_GetSpan %d,%p:\n", nNum, vSpan);
}

/**
 * SACT.ExpSp_Clear (1.0~)
 *   説明スプライト設定クリア
 */
void ExpSp_Clear() {
	DEBUG_COMMAND_YET("SACT.ExpSp_Clear:\n");

	sp_exp_clear();
}

/**
 * SACT.ExpSp_Add (1.0~)
 *   説明スプライト設定追加
 *   @param wNumSP1: スイッチスプライト
 *   @param wNumSP2: 説明スプライト
 */
void ExpSp_Add() {
	int wNumSP1 = getCaliValue();
	int wNumSP2 = getCaliValue();
	
	sp_exp_add(wNumSP1, wNumSP2);
	
	DEBUG_COMMAND_YET("SACT.ExpSp_Add %d,%d:\n", wNumSP1, wNumSP2);
}

/**
 * SACT.ExpSp_Del (1.0~)
 *   説明スプライト削除
 *   @param wNum: スプライト番号
 */
void ExpSp_Del() {
	int wNum = getCaliValue();
	
	sp_exp_del(wNum);
	
	DEBUG_COMMAND_YET("SACT.ExpSp_Del %d:\n", wNum);
}

/**
 * SACT.TimerSet (1.0~)
 *   指定のIDのタイマーをwCount値でリセット
 *   @param wTimerID: タイマーID
 *   @param wCount: リセットする値
 */
void TimerSet() {
	int wTimerID = getCaliValue();
	int wCount = getCaliValue();
	
	stimer_reset(wTimerID, wCount);
	
	DEBUG_COMMAND("SACT.TimerSet %d,%d:\n", wTimerID, wCount);
}

/**
 * SACT.TimerGet (1.0~)
 *   指定のIDのタイマーをRNDに取得
 *   @param wTimerID: タイマーID
 *   @param vRND: 取得する変数
 */
void TimerGet() {
	int wTimerID = getCaliValue();
	int *vRND = getCaliVariable();

	*vRND = stimer_get(wTimerID);
	
	DEBUG_COMMAND("SACT.TimerGet %d,%p:\n", wTimerID, vRND);
}

/**
 * SACT.TimerWait (1.0~)
 *   指定IDのタイマーが指定カウントになるまで待つ
 *   @param wTimerID: タイマーID
 *   @param wCount: 指定カウント
 */
void TimerWait() {
	int wTimerID = getCaliValue();
	int wCount = getCaliValue();

	while(wCount > stimer_get(wTimerID)) {
		sys_keywait(10, FALSE);
	}
	
	DEBUG_COMMAND("SACT.TimerWait %d,%d:\n", wTimerID, wCount);
}

/**
 * SACT.Wait (1.1~)
 *   指定時間、すべての動作を停止
 *   @param nCount: 時間(1/100秒単位)
 */
void Wait() {
	int wCount = getCaliValue();
	
	sys_keywait(wCount*10, FALSE);
	
	DEBUG_COMMAND_YET("SACT.Wait %d:\n", wCount);
}

/**
 * SACT.SoundPlay (1.0~)
 *   サウンド直接再生  (~SOUND_PLAY)
 *   @param wNum: 再生する番号
 */
void SoundPlay() {
	int wNum = getCaliValue();
	
	ssnd_play(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPlay %d:\n", wNum);
}

/**
 * SACT.SoundStop (1.0~)
 *   サウンド再生停止 (~SOUND_STOP)
 *   @param wNum: 停止する番号
 *   @param wFadeTime: 停止するまでの時間 (1/100sec)
 */
void SoundStop() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	
	ssnd_stop(wNum, wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.SoundStop %d,%d:\n", wNum, wFadeTime);
}

/**
 * SACT.SoundStopAll (1.1~)
 *   サウンド再生停止 (~SOUND_STOP)
 *   @param wNum: 停止する番号
 *   @param wFadeTime: 停止するまでの時間 (1/100sec)
 */
void SoundStopAll() {
	int wFadeTime = getCaliValue();
	
	ssnd_stopall(wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.SoundStopAll %d:\n", wFadeTime);
}

/**
 * SACT.SoundWait (1.0~)
 *   ヘッダで指定された時間or再生終了まで待つ  (~SOUND_WAIT)
 *   @param wNum: 指定番号
 */
void SoundWait() {
	int wNum = getCaliValue();
	
	ssnd_wait(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundWait %d:\n", wNum);
}

/**
 * SACT.SoundWaitKey (1.0~)
 *   指定されたサウンドが再生終了するか、キーが押されるまで待つ
 *   @param wNum: 指定番号
 *   @param vKey: キャンセルキー
 */
void SoundWaitKey() {
	int wNum = getCaliValue();
	int *vKey = getCaliVariable();
	
	ssnd_waitkey(wNum, vKey);
	
	DEBUG_COMMAND_YET("SACT.SoundWaitKey %d,%p:\n", wNum, vKey);
}

/**
 * SACT.SoundPrepare (1.0~)
 *   再生の準備をする(~SOUND_PREPARE)
 *   @param wNum: 再生する番号
 */
void SoundPrepare() {
	int wNum = getCaliValue();
	
	ssnd_prepare(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPrepare %d:\n", wNum);
}

/**
 * SACT.SoundPrepareLR (1.0~)
 *   再生の準備をする(左右反転) (~SOUND_PREPARE_LR)
 *   @param wNum: 再生する番号
 */
void SoundPrepareLR() {
	int wNum = getCaliValue();

	ssnd_prepareLRrev(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPrepareLR %d:\n", wNum);
}

/**
 * SACT.SoundPlayLR (1.0~)
 *   左右反転して再生 (~SOUND_PLAY_LR)
 *   @param wNum: 再生する番号
 */
void SoundPlayLR() {
	int wNum = getCaliValue();
	
	ssnd_playLRrev(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPlayLR %d:\n", wNum);
}

/**
 * SACT.SpriteSound (1.0~)
 * サウンド(スプライト指定) (~SP_SOUND)
 *   @param wNumSP: 設定するスプライト番号
 *   @param nCount: 設定する個数
 *   @param wNumWave1: Sound1
 *   @param wNumWave2: Sound2
 *   @param wNumWave3: Sound3
 */
void SpriteSound() {
	int wNumSP = getCaliValue();
	int nCount = getCaliValue();
	int wNumWave1 = getCaliValue();
	int wNumWave2 = getCaliValue();
	int wNumWave3 = getCaliValue();
	int i;
	
	for (i = wNumSP; i < (wNumSP + nCount); i++) {
		sp_sound_set(i, wNumWave1, wNumWave2, wNumWave3);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteSound %d,%d,%d,%d,%d:\n", wNumSP, nCount, wNumWave1, wNumWave2, wNumWave3);
}

/**
 * SACT.SpriteSoundWait (1.0~)
 *   SpriteSoundで設定したすべての音の再生終了まで待つ (~SP_SOUND_WAIT)
 */
void SpriteSoundWait() {
	DEBUG_COMMAND_YET("SACT.SpriteSoundWait:\n");

	sp_sound_wait();
}

/**
 * SACT.SpriteSoundOB (1.0~)
 *   範囲外をクリックしたときの音  (~SPRITE_SOUND_OB)
 *   @param wNumWave: 再生する番号、０でクリア
 */
void SpriteSoundOB() {
	int wNumWave = getCaliValue();
	
	sp_sound_ob(wNumWave);
	
	DEBUG_COMMAND_YET("SACT.SpriteSoundOB %d:\n", wNumWave);
}

/**
 * SACT.MusicCheck (1.0~)
 *   音楽データがあるかどうか (~MUSIC_CHECK)
 *   @param wNum: 番号
 *   @param vRND: 0:ない、1:ある
 */
void MusicCheck() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();
	
	*vRND = smus_check(wNum);
	
	DEBUG_COMMAND_YET("SACT.MusicCheck %d,%p:\n", wNum, vRND);
}

/**
 * SACT.MusicGetLength (1.0~)
 *   音楽データの長さを1/100秒単位で取得 (~MUSIC_GET_LENGTH)
 *   @param wNum: 音楽番号
 *   @param vRND: 取得した長さを格納する変数
 */
void MusicGetLength() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();
	
	*vRND = smus_getlength(wNum);
	
	DEBUG_COMMAND_YET("SACT.MusicGetLength %d,%d:\n", wNum, *vRND);
}

/**
 * SACT.MusicGetPos (1.0~)
 *   音楽データの再生位置を1/100秒単位で取得 (~MUSIC_GET_POS)
 *   @param wNum: 音楽番号
 *   @param vRND: 取得した位置を格納する変数
 */
void MusicGetPos() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();
	
	*vRND = smus_getpos(wNum);
	
	DEBUG_COMMAND_YET("SACT.MusicGetPos %d,%d:\n", wNum, *vRND);
}

/**
 * SACT.MusicPlay (1.0~)
 *   再生 (~MUSIC_PLAY)
 *   @param wNum: 音楽番号
 *   @param wFadeTime: フェードイン時間(1/100秒)
 *   @param wVolume: 音量(0-100)
 */
void MusicPlay() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	int wVolume = getCaliValue();
	
	smus_play(wNum, wFadeTime, wVolume);
	
	DEBUG_COMMAND_YET("SACT.MusicPlay %d,%d,%d:\n", wNum, wFadeTime, wVolume);
}

/**
 * SACT.MusicStop (1.0~)
 *   音楽停止 (~MUSIC_STOP)
 *   @param wNum: 音楽番号
 *   @param wFadeTime: 終了するまでの時間(1/100秒)
 */
void MusicStop() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	
	smus_stop(wNum, wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.MusicStop %d,%d:\n", wNum, wFadeTime);
}

/**
 * SACT.MusicStopAll (1.2~)
 *   すべての音楽を停止
 *   @param wFadeTime: 終了するまでの時間(1/100秒)
 */
void MusicStopAll() {
	int wFadeTime = getCaliValue();
	
	smus_stopall(wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.MusicStopAll %d:\n", wFadeTime);
}

/**
 * SACT.MusicFade (1.0~)
 *   指定のボリュームまでフェード (~MUSIC_FADE)
 *   @param wNum: 音楽番号
 *   @param wFadeTime: フェード時間(1/100秒)
 *   @param wVolume: 音量 (1-100)
 */
void MusicFade() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	int wVolume = getCaliValue();
	
	smus_fade(wNum, wFadeTime, wVolume);
	
	DEBUG_COMMAND_YET("SACT.MusicFade %d,%d,%d:\n", wNum, wFadeTime, wVolume);
}

/**
 * SACT.MusicWait (1.0~)
 *   再生が終了するまで待つ (~MUSIC_WAIT)
 *   @param wNum: 音楽番号
 *   @param nTimeOut: (1.1~)
 */
void MusicWait() {
	int wNum = getCaliValue();
	int nTimeOut = 0;
	
	if (sact.version >= 110) {
		nTimeOut = getCaliValue();
	}
	
	smus_wait(wNum, nTimeOut);
	
	DEBUG_COMMAND_YET("SACT.MusicWait %d,%d:\n", wNum, nTimeOut);
}

/**
 * SACT.MusicWatiPos (1.0~)
 *   指定の音楽がwIndex番のマークで指定された再生位置に来るまで待つ 
 *   (~MUSIC_WAIT_POS)
 *   @param wNum: 音楽番号
 *   @param wIndex: 位置マーク番号
 */
void MusicWaitPos() {
	int wNum = getCaliValue();
	int wIndex = getCaliValue();
	
	smus_waitpos(wNum, wIndex);
	
	DEBUG_COMMAND_YET("SACT.MusicWaitPos %d,%d:\n", wNum, wIndex);
}

/**
 * SACT.SoundGetLinkNum (1.0~)
 *   指定チャンネルのリンク番号を取得 (~SOUND_GET_LINK_NUM)
 *   @param wNum: チャンネル番号
 *   @param vRND: リンク番号(未使用＝０)
 */
void SoundGetLinkNum() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();

	*vRND = ssnd_getlinknum(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundGetLinkNum %d,%p:\n", wNum, vRND);
}

/**
 * SACT.ChartPos (1.0~)
 *   グラフ用チャート作成
 *   @param pos : 結果出力変数
 *   @param pos1: 最小値
 *   @param pos2: 最大値
 *   @param val1: 分率最小値
 *   @param val2: 分率最大値
 *   @param val : 分率
 *
 *     pos = ((pos2-pos1) / (val2-val1)) * (val-val1) + pos1
 */
void ChartPos() {
	int *pos = getCaliVariable();
	int pos1 = getCaliValue();
	int pos2 = getCaliValue();
	int val1 = getCaliValue();
	int val2 = getCaliValue();
	int val  = getCaliValue();
	
	schart_pos(pos, pos1, pos2, val1, val2, val);
	
	DEBUG_COMMAND_YET("SACT.ChartPos %p,%d,%d,%d,%d,%d:\n", pos, pos1, pos2, val1, val2, val);
}

/**
 * SACT.NumToStr (1.0~)
 *   数値 -> 文字列変換
 *   @param strno: 変換済み文字列変数番号
 *   @param fig:   けた数
 *   @param zeropad: 0: ゼロ埋めしない, 1: ゼロ埋めする
 *   @param num: 変換する数値
 */
void NumToStr() {
	int strno   = getCaliValue();
	int fig     = getCaliValue();
	int zeropad = getCaliValue();
	int num     = getCaliValue();
	
	sstr_num2str(strno, fig, zeropad, num);
	
	DEBUG_COMMAND_YET("SACT.NumToStr %d,%d,%d,%d:\n", strno, fig, zeropad, num);
}

/**
 * SACT.Maze_Create (1.0~)
 */
void Maze_Create() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	WARNING("NOT IMPLEMENTED\n");
	
	DEBUG_COMMAND_YET("SACT.Maze_Create %d,%d:\n", p1,p2);
}

/**
 * SACT.Maze_Get (1.0~)
 */
void Maze_Get() {
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	
	WARNING("NOT IMPLEMENTED\n");
	
	DEBUG_COMMAND_YET("SACT.Maze_Get %p,%d,%d:\n", p1,p2,p3);
}

/**
 * SACT.EncryptWORD (1.0~)
 */
void EncryptWORD() {
	int *array = getCaliVariable();
	int num = getCaliValue();
	int key = getCaliValue();

	scryp_encrypt_word(array, num, key);
	
	DEBUG_COMMAND_YET("SACT.EncryptWORD %p,%d,%d:\n", array, num, key);
}

/**
 * SACT.DecryptWORD (1.0~)
 */
void DecryptWORD() {
	int *array = getCaliVariable();
	int num = getCaliValue();
	int key = getCaliValue();

	scryp_encrypt_word(array, num, key);
	
	DEBUG_COMMAND_YET("SACT.DecryptWORD %p,%d,%d:\n", array, num, key);
}

/**
 * SACT.EncryptString (1.0~)
 */
void EncryptString() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	scryp_encrypt_str(p1, p2);
	
	DEBUG_COMMAND_YET("SACT.EncryptString %d,%d:\n", p1,p2);
}

/**
 * SACT.DecryptString (1.0~)
 */
void DecryptString() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	scryp_decrypt_str(p1, p2);
	
	DEBUG_COMMAND_YET("SACT.DecryptString %d,%d:\n", p1,p2);
}

/**
 * SACT.XMenuClear (1.0~)
 *   拡張メニュー初期化
 */
void XMenuClear() {
	spxm_clear();
	
	DEBUG_COMMAND_YET("SACT.XMenuClear:\n");
}

/**
 * SACT.XMenuRegister (1.0~)
 *   現在バッファにある文字列を拡張メニューのアイテムとして登録
 *   @param nRegiNum: 拡張メニューの内部インデックス番号
 *   @param nMenuID: 選択されたときに返す番号(ID)
 */
void XMenuRegister() {
	int nRegiNum = getCaliValue();
	int nMenuID  = getCaliValue();
	
	spxm_register(nRegiNum, nMenuID);
	
	DEBUG_COMMAND_YET("SACT.XMenuRegister %d,%d:\n", nRegiNum, nMenuID);
}

/**
 * SACT.XMenuGetNum (1.0~)
 *   XMenuRegisterで登録されたIDを返す
 *   @param nRegiNum: 内部インデックス番号
 *   @param vMenuID: 登録されているIDを格納する変数
 */
void XMenuGetNum() {
	int nRegiNum = getCaliValue();
	int *vMenuID = getCaliVariable();
	
	*vMenuID = spxm_getnum(nRegiNum);
	
	DEBUG_COMMAND_YET("SACT.XMenuGetNum %d,%p:\n", nRegiNum, vMenuID);
}

/**
 * SACT.XMenuGetText (1.0~)
 *   XMenuRegisterで登録したアイテムを指定の文字列変数にコピーする
 *   @param nRegiNum: 内部インデックス番号
 *   @param strno: コピー先文字列変数番号
 */
void XMenuGetText() {
	int nRegiNum = getCaliValue();
	int strno    = getCaliValue();
	
	spxm_gettext(nRegiNum, strno);
	
	DEBUG_COMMAND_YET("SACT.XMenuGetText %d,%d:\n", nRegiNum, strno);
}

/**
 * SACT.XMenuTitleRegister (1.0~)
 *   現在バッファにある文字列を拡張メニューのタイトルとして登録
 */
void XMenuTitleRegister() {
	spxm_titlereg();
	
	DEBUG_COMMAND_YET("SACT.XMenuTitleRegister:\n");
}

/**
 * SACT.XMenuTitleGet (1.0~)
 *   拡張メニューのタイトルを指定の文字列変数にコピー
 *   @param strno: コピー先文字列変数番号
 */
void XMenuTitleGet() {
	int strno = getCaliValue();
	
	spxm_titleget(strno);
	
	DEBUG_COMMAND_YET("SACT.XMenuTitleGet %d:\n", strno);
}
