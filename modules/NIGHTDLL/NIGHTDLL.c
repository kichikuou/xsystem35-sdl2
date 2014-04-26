#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"
#include "night.h"
#include "nt_sound.h"
#include "nt_graph.h"
#include "nt_scenario.h"
#include "nt_msg.h"
#include "nt_event.h"

#include "sactstring.h"

// NIGHTDLL用データ
night_t nightprv;


void Init(void) { /* 0 */
	int *var = getCaliVariable();
	int p1 = getCaliValue();  /* ISys3xCG */
	int p2 = getCaliValue();  /* ISys3xDIB */
	int p3 = getCaliValue();  /* ISys3xMsgString */
	int p4 = getCaliValue();  /* ISys3xStringTable */
	int p5 = getCaliValue();  /* ISys3xSystem */
	int p6 = getCaliValue();  /* ITimer */
	int p7 = getCaliValue();  /* IUI */
	int p8 = getCaliValue();  /* IWinMsg */
	int p9 = getCaliValue();  /* ISys3x */
	int p10 = getCaliValue(); /* ISys3xInputDevice */
	
	*var = 1;
	
	DEBUG_COMMAND_YET("NIGHTDLL.Init %p:\n", var);
}

void InitGame() { /* 1 */
	nact->ags.font->antialiase_on = TRUE;
	sys_setHankakuMode(2);
	
	nact->msgout = ntmsg_add;
	nact->ags.eventcb = ntev_callback;
	nact->callback = ntev_main;
	
	nt_gr_init();
	ntmsg_init();
	sstr_init();

	DEBUG_COMMAND_YET("NIGHTDLL.InitGame:\n");
}	

// メッセージの枠の表示
void SetMsgFrame() { /* 2 */
	int p1 = getCaliValue(); // 0=枠消去, 1=枠あり, 2=中央
	
	ntmsg_set_frame(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetMsgFram %d:\n", p1);
}

// メッセージの表示位置の設定
void SetMsgPlaceMethod(void) { /* 3 */
	int p1 = getCaliValue(); // 0=メッセージ枠内, 1=中央, 
	                         // 2=メッセージ枠+顔つき 
	ntmsg_set_place(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetMsgPlaceMethod %d:\n", p1);
}

// 未実装?
void SetMsgDrawEffect(void) { /* 4 */
	int p1 = getCaliValue(); // 0, 1, 2, 3 (実際にはどれも機能しない?)
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetMsgDrawEffect %d:\n", p1);
}

// 未実装?
void SetMsgClearEffect(void) { /* 5 */
	int p1 = getCaliValue(); // 0, 1, 2, 4 (実際にはどれも機能しない?)
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetMsgClearEffect %d:\n", p1);
}

// 壁紙CGの設定
void SetWallPaper(void) { /* 6 */
	int p1 = getCaliValue(); // 壁紙CG番号
	
	nt_gr_set_wallpaper(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetWallPaper %d:\n", p1);
}

// 背景CGの設定
void SetScenery(void) { /* 7 */
	int p1 = getCaliValue(); // 背景GC番号
	
	nt_gr_set_scenery(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetScenery %d:\n", p1);
}

// 顔CGの設定
void SetFace(void) { /* 8 */
	int p1 = getCaliValue(); // 顔CG番号
	
	nt_gr_set_face(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetFace %d:\n", p1);
}

// 立ち絵左の設定
void SetSpriteL(void) { /* 9 */
	int p1 = getCaliValue(); // 左人物スプライト番号
	
	nt_gr_set_spL(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetSpriteL %d:\n", p1);
}

// 立ち絵中央の設定
void SetSpriteM(void) { /* 10 */
	int p1 = getCaliValue(); // 中央人物スプライト番号
	
	nt_gr_set_spM(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetSpriteM %d:\n", p1);
}

// 立ち絵右の設定
void SetSpriteR(void) { /* 11 */
	int p1 = getCaliValue(); // 右人物スプライト番号
	
	nt_gr_set_spR(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetSpriteR %d:\n", p1);
}

// 立ち絵左の設定（季節違い?)
void SetSpriteSeasonL(void) { /* 12 */
	int p1 = getCaliValue(); // 左人物スプライト番号
	
	nt_gr_set_spsL(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetSpriteSeasonL %d:\n", p1);
}

// 立ち絵中央の設定（季節違い?)
void SetSpriteSeasonM(void) { /* 13 */
	int p1 = getCaliValue(); // 中央人物スプライト番号
	
	nt_gr_set_spM(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetSpriteSeasonM %d:\n", p1);
}

// 立ち絵右の設定（季節違い?)
void SetSpriteSeasonR(void) { /* 14 */
	int p1 = getCaliValue(); // 右人物スプライト番号
	
	nt_gr_set_spsR(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetSpriteSeasonR %d:\n", p1);
}

// 改行
void StartNewLine(void) { /* 15 */
	ntmsg_newline();
	
	DEBUG_COMMAND_YET("NIGHTDLL.StartNewLine:\n");
}

// メッセージフォントサイズの設定
void SetFontSize(void) { /* 16 */
	int p1 = getCaliValue(); // メッセージフォントサイズ
	
	night.fontsize = p1;
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetFontSize %d:\n", p1);
}

// フォントの種類の設定
void SetFont(void) { /* 17 */
	int p1 = getCaliValue(); // 0: ゴシック, 1: 明朝
	
	night.fonttype = p1;

	DEBUG_COMMAND_YET("NIGHTDLL.SetFont %d:\n", p1);
}

// 選択肢モード ON
void SetSelMode(void) { /* 18 */
	int p1 = getCaliValue(); // 0, 1(ほとんど0)
	
	night.selmode = p1;
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetSelMode %d:\n", p1);
}

// キー入力待ち後、改ページ
void AnalyzeMessage(void) { /* 19 */
	int *var = getCaliVariable(); // 入力されたキー

	*var = ntmsg_ana();
	
	DEBUG_COMMAND_YET("NIGHTDLL.AnalyzeMessage %p:\n", var);
}

// ~DRAWの効果時間
void SetDrawTime(void) { /* 20 */
	int p1 = getCaliValue(); // 効果時間 (未使用？)
	
	nt_gr_set_drawtime(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetDrawTime %d:\n", p1);
}

// 効果つき画面更新
void Draw(void) { /* 21 */
	int p1 = getCaliValue(); // 効果番号
	
	nt_gr_draw(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.Draw %d:\n", p1);
}

// 音声データを再生
void SetVoice(void) { /* 22 */
	int p1 = getCaliValue(); // ファイル番号
	
	nt_voice_set(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetVoice %d:\n", p1);
}

// 未使用
void WaitKey(void) { /* 23 */
	int p1 = getCaliValue(); // 

	DEBUG_COMMAND_YET("NIGHTDLL.WaitKey %d:\n", p1);
}

void AddFeeling(void) { /* 24 */
	int p1 = getCaliValue(); // person(1:新開,2:星川,3:百瀬,4:いずみ,5:鏡花,6:真言美,7:マコト	 
	int p2 = getCaliValue(); // val
	
	DEBUG_COMMAND_YET("NIGHTDLL.AddFeeling %d:\n", p1);

}

void SubFeeling(void) { /* 25 */
	int p1 = getCaliValue(); // person
	int p2 = getCaliValue(); // val
	
	DEBUG_COMMAND_YET("NIGHTDLL.SubFeeling %d:\n", p1);
}

void CallEvent(void) { /* 26 */
	int p1 = getCaliValue(); // 1, 2 (Event Number)
	
	nt_sco_callevent(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.CallEvent %d:\n", p1);
}

void ScreenCG(void) { /* 27 */
	/*
	    CG を読み込んで、surface0 と BlendScreen で重ね合わせ

	   x : 表示位置 X
	   y : 表示位置 Y
	   no: 読み込むCG番号
	*/
	int p1 = getCaliValue(); /* ISys3xDIB */
	int p2 = getCaliValue(); /* ISys3xCG  */
	int x  = getCaliValue();
	int y  = getCaliValue();
	int no = getCaliValue();
	
	nt_gr_screencg(no, x, y);

	DEBUG_COMMAND_YET("NIGHTDLL.ScreenCG %d,%d,%d:\n", x, y, no);
}

void RunGameMain(void) { /* 28 */
	int *p1 = getCaliVariable(); // result
	int p2 = getCaliValue();     // month
	int p3 = getCaliValue();     // day
	int p4 = getCaliValue();     // day of week
	int p5 = getCaliValue();     // 0=はじめから,1=途中から
	
	night.Month = p2;
	night.Day   = p3;
	night.DayOfWeek = p4;
	
	*p1 = nt_sco_main(p5);
	
	DEBUG_COMMAND_YET("NIGHTDLL.RunGameMain %p,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5);
}

void CheckNewGame(void) { /* 29 */
	int *p1 = getCaliVariable();
	
	*p1 = 0;
	
	DEBUG_COMMAND_YET("NIGHTDLL.CheckNewGame %p:\n", p1);
}

void SaveStartData(void) { /* 30 */
	DEBUG_COMMAND_YET("NIGHTDLL.SaveStartData:\n");
}

void PrintExitSystem(void) { /* 31 */
	DEBUG_COMMAND_YET("NIGHTDLL.PrintExitSystem:\n");
}

void SetCalendar(void) { /* 32 */
	int p1 = getCaliValue(); // 0, 1, 2

	DEBUG_COMMAND_YET("NIGHTDLL.SetCalendar %d:\n", p1);
}

void SetDate(void) { /* 33 */
	int p1 = getCaliValue(); // month
	int p2 = getCaliValue(); // day
	int p3 = getCaliValue(); // day of week
	
	night.Month = p1;
	night.Day   = p2;
	night.DayOfWeek = p3;
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetDate %d,%d,%d:\n", p1, p2, p3);
}

void GetDate(void) { /* 34 */
	int *p1 = getCaliVariable(); // month
	int *p2 = getCaliVariable(); // day
	int *p3 = getCaliVariable(); // day of weeek

	*p1 = night.Month;
	*p2 = night.Day;
	*p3 = night.DayOfWeek;
	
	DEBUG_COMMAND_YET("NIGHTDLL.GetDate %p,%p,%p:\n", p1, p2, p3);
}

void SelectGameLevel(void) { /* 35 */
	DEBUG_COMMAND_YET("NIGHTDLL.SelectGameLevel:\n");
}

void RunEventDungeon(void) { /* 36 */
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();

	*p1 = 1;
	
	DEBUG_COMMAND_YET("NIGHTDLL.RunEventDungeon %p,%d:\n", p1, p2);
}

void RunEventBattle(void) { /* 37 */
	int p1 = getCaliValue(); // 実際には機能しない？
	
	DEBUG_COMMAND_YET("NIGHTDLL.RunEventBattle %d:\n", p1);
}

// CD再生開始
void CDPlay(void) { /* 38 */
	int p1 = getCaliValue(); // no
	
	nt_cd_play(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.CDPlay %d:\n", p1);
}

// CD再生停止
void CDStop(void) { /* 39 */
	int p1 = getCaliValue(); // time(msec)
	
	nt_cd_stop(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.CDStop %d:\n", p1);
}

// CDのmute
void CDMute(void) { /* 40 */
	int p1 = getCaliValue(); // 0: mute off, 1: mute on
	
	nt_cd_mute(p1 == 0 ? FALSE : TRUE);
	
	DEBUG_COMMAND_YET("NIGHTDLL.CDMute %d:\n", p1);
}

// ch に効果音番号をセット
void SoundEffectSetWave(void) { /* 41 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // linkno
	
	nt_snd_setwave(p1, p2);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SoundEffectSetWave %d,%d:\n", p1, p2);
}

// ch にくり返し数をセット
void SoundEffectSetLoop(void) { /* 42 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // numloop
	
	nt_snd_setloop(p1, p2);
       
	DEBUG_COMMAND_YET("NIGHTDLL.SoundEffectSetLoop %d,%d:\n", p1, p2);
}

// ch に音量をセット
void SoundEffectSetVolume(void) { /* 43 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // vol
	
	nt_snd_setvol(p1, p2);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SoundEffectSetVolue %d,%d:\n", p1, p2);
}

// 効果音の再生を終るまで待つか待たないか？
void SoundEffectSetSyncFlag(void) { /* 44 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // 0: 終るまで待たない, 1: 待つ
	
	nt_snd_waitend(p1, p2 == 0 ? FALSE : TRUE);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SoundEffectSetSyncFlag %d,%d:\n", p1, p2);
}

// ch の効果音を再生
void SoundEffectPlay(void) { /* 45 */
	int p1 = getCaliValue(); // ch
	
	nt_snd_play(p1);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SoundEffectPlay %d:\n", p1);
}

// ch の効果音を停止
void SoundEffectStop(void) { /* 46 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // time (止まるまでの時間)
	
	nt_snd_stop(p1, p2);
	
	DEBUG_COMMAND_YET("NIGHTDLL.SoundEffectStop %d,%d:\n", p1,p2);
}

// 全てのチャンネルの再生を停止
void SoundEffectStopAll(void) { /* 47 */
	int p1 = getCaliValue(); // time (止まるまでの時間)
	
	nt_snd_stopall(p1);

	DEBUG_COMMAND_YET("NIGHTDLL.SoundEffectStopAll %d:\n", p1);
}

void RunSoundMode(void) { /* 48 */
	
	DEBUG_COMMAND_YET("NIGHTDLL.RunSoundMode:\n");
}

void RunMapEditor(void) { /* 49 */
	int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("NIGHTDLL.RunMapEditor %p:\n", p1);
}

void VisualListClear(void) { /* 50 */

	DEBUG_COMMAND_YET("NIGHTDLL.VisualListClear:\n");
}

void VisualListAdd(void) { /* 51 */
	int p1 = getCaliValue();

	DEBUG_COMMAND_YET("NIGHTDLL.VisualListAdd %d:\n", p1);
}

void GetLocalCountCG(void) { /* 52 */
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("NIGHTDLL.GetLocalCountCG %p,%d:\n", p1, p2);
}

void PlayMemory(void) { /* 53 */
	int *p1 = getCaliVariable(); // 回想ページ
	int *p2 = getCaliVariable(); // 回想RESULT
	
	DEBUG_COMMAND_YET("NIGHTDLL.PlayMemory %p,%p:\n", p1, p2);
}

void GetEventFlagTotal(void) { /* 54 */
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();
	
	DEBUG_COMMAND_YET("NIGHTDLL.GetEventFlagTotal %p,%d:\n", p1, p2);
}

void SetPlayerName(void) { /* 55 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("NIGHTDLL.SetPlayerName %d:\n", p1);
}

void GetPlayerName(void) { /* 56 */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("NIGHTDLL.GetPlayerName %d:\n", p1);
}

void SaveGame(void) { /* 57 */
	int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("NIGHTDLL.SaveGame %p:\n", p1);
}

void LoadGame(void) { /* 58 */
	int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("NIGHTDLL.LoadGame %p:\n", p1);
}

void ExistSaveData(void) { /* 59 */
	int *p1 = getCaliVariable();
	
	DEBUG_COMMAND_YET("NIGHTDLL.ExistSaveData %p:\n", p1);
}

void ExistStartData(void) { /* 60 */
	int *p1 = getCaliVariable();

	DEBUG_COMMAND_YET("NIGHTDLL.ExistStartData %p:\n", p1);
}

void SetAreaData(void) { /* 61 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();

	DEBUG_COMMAND_YET("NIGHTDLL.SetAreaData %d,%d,%d:\n", p1, p2, p3);
}

void RunBattleTest(void) { /* 62 */
	DEBUG_COMMAND_YET("NIGHTDLL.RunBattleTest:\n");
}

void RunTrainingTest(void) { /* 63 */
	DEBUG_COMMAND_YET("NIGHTDLL.RunTrainingTest:\n");
}

void TestEventCall(void) { /* 64 */
	DEBUG_COMMAND_YET("NIGHTDLL.TestEventCall:\n");
}

void Test(void) { /* 65 */
	DEBUG_COMMAND_YET("NIGHTDLL.Test:\n");
}

void DebugScenario(void) { /* 66 */
	DEBUG_COMMAND_YET("NIGHTDLL.DebugScenario:\n");
}

void GetDLLTime(void) { /* 67 */
	int *p1 = getCaliVariable();
	int *p2 = getCaliVariable();
	int *p3 = getCaliVariable();
	int *p4 = getCaliVariable();
	int *p5 = getCaliVariable();
	int *p6 = getCaliVariable();
	int *p7 = getCaliVariable();

	DEBUG_COMMAND_YET("NIGHTDLL.GetDLLTime %p,%p,%p,%p,%p,%p,%p:\n", p1, p2, p3, p4, p5, p6, p7);
}

