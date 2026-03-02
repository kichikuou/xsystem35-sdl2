#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "night.h"
#include "nt_sound.h"
#include "nt_graph.h"
#include "nt_scenario.h"
#include "nt_msg.h"
#include "nt_event.h"
#include "sactcg.h"

#include "sactstring.h"

// NIGHTDLL用データ
night_t nightprv;


static void Init(void) { /* 0 */
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
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.Init %p:", var);
}

static void NIGHTDLL_reset(void) {
	nt_sstr_reset();
	nt_sp_clear_updatelist();
	for (int i = 0; i < SPRITEMAX; i++) {
		if (night.sp[i])
			nt_sp_free(night.sp[i]);
	}
	nt_scg_freeall();
	memset(&night, 0, sizeof(night));
}

static void InitGame() { /* 1 */
	ags_setAntialiasedStringMode(true);
	sys_setHankakuMode(2);
	
	nact->msgout = ntmsg_add;
	nact->ags.eventcb = ntev_callback;
	nact->callback = ntev_main;
	
	nt_gr_init();
	ntmsg_init();
	nt_sstr_init();

	TRACE_UNIMPLEMENTED("NIGHTDLL.InitGame:");
}	

// メッセージの枠の表示
static void SetMsgFrame() { /* 2 */
	int p1 = getCaliValue(); // 0=枠消去, 1=枠あり, 2=中央
	
	ntmsg_set_frame(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetMsgFram %d:", p1);
}

// メッセージの表示位置の設定
static void SetMsgPlaceMethod(void) { /* 3 */
	int p1 = getCaliValue(); // 0=メッセージ枠内, 1=中央, 
	                         // 2=メッセージ枠+顔つき 
	ntmsg_set_place(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetMsgPlaceMethod %d:", p1);
}

// 未実装?
static void SetMsgDrawEffect(void) { /* 4 */
	int p1 = getCaliValue(); // 0, 1, 2, 3 (実際にはどれも機能しない?)
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetMsgDrawEffect %d:", p1);
}

// 未実装?
static void SetMsgClearEffect(void) { /* 5 */
	int p1 = getCaliValue(); // 0, 1, 2, 4 (実際にはどれも機能しない?)
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetMsgClearEffect %d:", p1);
}

// 壁紙CGの設定
static void SetWallPaper(void) { /* 6 */
	int p1 = getCaliValue(); // 壁紙CG番号
	
	nt_gr_set_wallpaper(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetWallPaper %d:", p1);
}

// 背景CGの設定
static void SetScenery(void) { /* 7 */
	int p1 = getCaliValue(); // 背景GC番号
	
	nt_gr_set_scenery(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetScenery %d:", p1);
}

// 顔CGの設定
static void SetFace(void) { /* 8 */
	int p1 = getCaliValue(); // 顔CG番号
	
	nt_gr_set_face(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetFace %d:", p1);
}

// 立ち絵左の設定
static void SetSpriteL(void) { /* 9 */
	int p1 = getCaliValue(); // 左人物スプライト番号
	
	nt_gr_set_spL(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetSpriteL %d:", p1);
}

// 立ち絵中央の設定
static void SetSpriteM(void) { /* 10 */
	int p1 = getCaliValue(); // 中央人物スプライト番号
	
	nt_gr_set_spM(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetSpriteM %d:", p1);
}

// 立ち絵右の設定
static void SetSpriteR(void) { /* 11 */
	int p1 = getCaliValue(); // 右人物スプライト番号
	
	nt_gr_set_spR(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetSpriteR %d:", p1);
}

// 立ち絵左の設定（季節違い?)
static void SetSpriteSeasonL(void) { /* 12 */
	int p1 = getCaliValue(); // 左人物スプライト番号
	
	nt_gr_set_spsL(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetSpriteSeasonL %d:", p1);
}

// 立ち絵中央の設定（季節違い?)
static void SetSpriteSeasonM(void) { /* 13 */
	int p1 = getCaliValue(); // 中央人物スプライト番号
	
	nt_gr_set_spM(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetSpriteSeasonM %d:", p1);
}

// 立ち絵右の設定（季節違い?)
static void SetSpriteSeasonR(void) { /* 14 */
	int p1 = getCaliValue(); // 右人物スプライト番号
	
	nt_gr_set_spsR(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetSpriteSeasonR %d:", p1);
}

// 改行
static void StartNewLine(void) { /* 15 */
	ntmsg_newline();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.StartNewLine:");
}

// メッセージフォントサイズの設定
static void SetFontSize(void) { /* 16 */
	int p1 = getCaliValue(); // メッセージフォントサイズ
	
	night.fontsize = p1;
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetFontSize %d:", p1);
}

// フォントの種類の設定
static void SetFont(void) { /* 17 */
	int p1 = getCaliValue(); // 0: ゴシック, 1: 明朝
	
	night.fonttype = p1;

	TRACE_UNIMPLEMENTED("NIGHTDLL.SetFont %d:", p1);
}

// 選択肢モード ON
static void SetSelMode(void) { /* 18 */
	int p1 = getCaliValue(); // 0, 1(ほとんど0)
	
	night.selmode = p1;
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetSelMode %d:", p1);
}

// キー入力待ち後、改ページ
static void AnalyzeMessage(void) { /* 19 */
	int *var = getCaliVariable(); // 入力されたキー

	*var = ntmsg_ana();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.AnalyzeMessage %p:", var);
}

// ~DRAWの効果時間
static void SetDrawTime(void) { /* 20 */
	int p1 = getCaliValue(); // 効果時間 (未使用？)
	
	nt_gr_set_drawtime(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetDrawTime %d:", p1);
}

// 効果つき画面更新
static void Draw(void) { /* 21 */
	int p1 = getCaliValue(); // 効果番号
	
	nt_gr_draw(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.Draw %d:", p1);
}

// 音声データを再生
static void SetVoice(void) { /* 22 */
	int p1 = getCaliValue(); // ファイル番号
	
	nt_voice_set(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetVoice %d:", p1);
}

// 未使用
static void WaitKey(void) { /* 23 */
	int p1 = getCaliValue(); // 

	TRACE_UNIMPLEMENTED("NIGHTDLL.WaitKey %d:", p1);
}

static void AddFeeling(void) { /* 24 */
	int p1 = getCaliValue(); // person(1:新開,2:星川,3:百瀬,4:いずみ,5:鏡花,6:真言美,7:マコト	 
	int p2 = getCaliValue(); // val
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.AddFeeling %d:", p1);

}

static void SubFeeling(void) { /* 25 */
	int p1 = getCaliValue(); // person
	int p2 = getCaliValue(); // val
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SubFeeling %d:", p1);
}

static void CallEvent(void) { /* 26 */
	int p1 = getCaliValue(); // 1, 2 (Event Number)
	
	nt_sco_callevent(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.CallEvent %d:", p1);
}

static void ScreenCG(void) { /* 27 */
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

	TRACE_UNIMPLEMENTED("NIGHTDLL.ScreenCG %d,%d,%d:", x, y, no);
}

static void RunGameMain(void) { /* 28 */
	int *p1 = getCaliVariable(); // result
	int p2 = getCaliValue();     // month
	int p3 = getCaliValue();     // day
	int p4 = getCaliValue();     // day of week
	int p5 = getCaliValue();     // 0=はじめから,1=途中から
	
	night.Month = p2;
	night.Day   = p3;
	night.DayOfWeek = p4;
	
	*p1 = nt_sco_main(p5);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.RunGameMain %p,%d,%d,%d,%d:", p1, p2, p3, p4, p5);
}

static void CheckNewGame(void) { /* 29 */
	int *p1 = getCaliVariable();
	
	*p1 = 0;
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.CheckNewGame %p:", p1);
}

static void SaveStartData(void) { /* 30 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.SaveStartData:");
}

static void PrintExitSystem(void) { /* 31 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.PrintExitSystem:");
}

static void SetCalendar(void) { /* 32 */
	int p1 = getCaliValue(); // 0, 1, 2

	TRACE_UNIMPLEMENTED("NIGHTDLL.SetCalendar %d:", p1);
}

static void SetDate(void) { /* 33 */
	int p1 = getCaliValue(); // month
	int p2 = getCaliValue(); // day
	int p3 = getCaliValue(); // day of week
	
	night.Month = p1;
	night.Day   = p2;
	night.DayOfWeek = p3;
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetDate %d,%d,%d:", p1, p2, p3);
}

static void GetDate(void) { /* 34 */
	int *p1 = getCaliVariable(); // month
	int *p2 = getCaliVariable(); // day
	int *p3 = getCaliVariable(); // day of weeek

	*p1 = night.Month;
	*p2 = night.Day;
	*p3 = night.DayOfWeek;
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.GetDate %p,%p,%p:", p1, p2, p3);
}

static void SelectGameLevel(void) { /* 35 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.SelectGameLevel:");
}

static void RunEventDungeon(void) { /* 36 */
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();

	*p1 = 1;
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.RunEventDungeon %p,%d:", p1, p2);
}

static void RunEventBattle(void) { /* 37 */
	int p1 = getCaliValue(); // 実際には機能しない？
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.RunEventBattle %d:", p1);
}

// CD再生開始
static void CDPlay(void) { /* 38 */
	int p1 = getCaliValue(); // no
	
	nt_cd_play(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.CDPlay %d:", p1);
}

// CD再生停止
static void CDStop(void) { /* 39 */
	int p1 = getCaliValue(); // time(msec)
	
	nt_cd_stop(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.CDStop %d:", p1);
}

// CDのmute
static void CDMute(void) { /* 40 */
	int p1 = getCaliValue(); // 0: mute off, 1: mute on
	
	nt_cd_mute(p1 != 0);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.CDMute %d:", p1);
}

// ch に効果音番号をセット
static void SoundEffectSetWave(void) { /* 41 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // linkno
	
	nt_snd_setwave(p1, p2);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SoundEffectSetWave %d,%d:", p1, p2);
}

// ch にくり返し数をセット
static void SoundEffectSetLoop(void) { /* 42 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // numloop
	
	nt_snd_setloop(p1, p2);
       
	TRACE_UNIMPLEMENTED("NIGHTDLL.SoundEffectSetLoop %d,%d:", p1, p2);
}

// ch に音量をセット
static void SoundEffectSetVolume(void) { /* 43 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // vol
	
	nt_snd_setvol(p1, p2);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SoundEffectSetVolue %d,%d:", p1, p2);
}

// 効果音の再生を終るまで待つか待たないか？
static void SoundEffectSetSyncFlag(void) { /* 44 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // 0: 終るまで待たない, 1: 待つ
	
	nt_snd_waitend(p1, p2 != 0);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SoundEffectSetSyncFlag %d,%d:", p1, p2);
}

// ch の効果音を再生
static void SoundEffectPlay(void) { /* 45 */
	int p1 = getCaliValue(); // ch
	
	nt_snd_play(p1);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SoundEffectPlay %d:", p1);
}

// ch の効果音を停止
static void SoundEffectStop(void) { /* 46 */
	int p1 = getCaliValue(); // ch
	int p2 = getCaliValue(); // time (止まるまでの時間)
	
	nt_snd_stop(p1, p2);
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SoundEffectStop %d,%d:", p1,p2);
}

// 全てのチャンネルの再生を停止
static void SoundEffectStopAll(void) { /* 47 */
	int p1 = getCaliValue(); // time (止まるまでの時間)
	
	nt_snd_stopall(p1);

	TRACE_UNIMPLEMENTED("NIGHTDLL.SoundEffectStopAll %d:", p1);
}

static void RunSoundMode(void) { /* 48 */
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.RunSoundMode:");
}

static void RunMapEditor(void) { /* 49 */
	int *p1 = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.RunMapEditor %p:", p1);
}

static void VisualListClear(void) { /* 50 */

	TRACE_UNIMPLEMENTED("NIGHTDLL.VisualListClear:");
}

static void VisualListAdd(void) { /* 51 */
	int p1 = getCaliValue();

	TRACE_UNIMPLEMENTED("NIGHTDLL.VisualListAdd %d:", p1);
}

static void GetLocalCountCG(void) { /* 52 */
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.GetLocalCountCG %p,%d:", p1, p2);
}

static void PlayMemory(void) { /* 53 */
	int *p1 = getCaliVariable(); // 回想ページ
	int *p2 = getCaliVariable(); // 回想RESULT
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.PlayMemory %p,%p:", p1, p2);
}

static void GetEventFlagTotal(void) { /* 54 */
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.GetEventFlagTotal %p,%d:", p1, p2);
}

static void SetPlayerName(void) { /* 55 */
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SetPlayerName %d:", p1);
}

static void GetPlayerName(void) { /* 56 */
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.GetPlayerName %d:", p1);
}

static void SaveGame(void) { /* 57 */
	int *p1 = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.SaveGame %p:", p1);
}

static void LoadGame(void) { /* 58 */
	int *p1 = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.LoadGame %p:", p1);
}

static void ExistSaveData(void) { /* 59 */
	int *p1 = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("NIGHTDLL.ExistSaveData %p:", p1);
}

static void ExistStartData(void) { /* 60 */
	int *p1 = getCaliVariable();

	TRACE_UNIMPLEMENTED("NIGHTDLL.ExistStartData %p:", p1);
}

static void SetAreaData(void) { /* 61 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();

	TRACE_UNIMPLEMENTED("NIGHTDLL.SetAreaData %d,%d,%d:", p1, p2, p3);
}

static void RunBattleTest(void) { /* 62 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.RunBattleTest:");
}

static void RunTrainingTest(void) { /* 63 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.RunTrainingTest:");
}

static void TestEventCall(void) { /* 64 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.TestEventCall:");
}

static void Test(void) { /* 65 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.Test:");
}

static void DebugScenario(void) { /* 66 */
	TRACE_UNIMPLEMENTED("NIGHTDLL.DebugScenario:");
}

static void GetDLLTime(void) { /* 67 */
	int *p1 = getCaliVariable();
	int *p2 = getCaliVariable();
	int *p3 = getCaliVariable();
	int *p4 = getCaliVariable();
	int *p5 = getCaliVariable();
	int *p6 = getCaliVariable();
	int *p7 = getCaliVariable();

	TRACE_UNIMPLEMENTED("NIGHTDLL.GetDLLTime %p,%p,%p,%p,%p,%p,%p:", p1, p2, p3, p4, p5, p6, p7);
}

static const ModuleFunc functions[] = {
	{"AddFeeling", AddFeeling},
	{"AnalyzeMessage", AnalyzeMessage},
	{"CDMute", CDMute},
	{"CDPlay", CDPlay},
	{"CDStop", CDStop},
	{"CallEvent", CallEvent},
	{"CheckNewGame", CheckNewGame},
	{"DebugScenario", DebugScenario},
	{"Draw", Draw},
	{"ExistSaveData", ExistSaveData},
	{"ExistStartData", ExistStartData},
	{"GetDLLTime", GetDLLTime},
	{"GetDate", GetDate},
	{"GetEventFlagTotal", GetEventFlagTotal},
	{"GetLocalCountCG", GetLocalCountCG},
	{"GetPlayerName", GetPlayerName},
	{"Init", Init},
	{"InitGame", InitGame},
	{"LoadGame", LoadGame},
	{"PlayMemory", PlayMemory},
	{"PrintExitSystem", PrintExitSystem},
	{"RunBattleTest", RunBattleTest},
	{"RunEventBattle", RunEventBattle},
	{"RunEventDungeon", RunEventDungeon},
	{"RunGameMain", RunGameMain},
	{"RunMapEditor", RunMapEditor},
	{"RunSoundMode", RunSoundMode},
	{"RunTrainingTest", RunTrainingTest},
	{"SaveGame", SaveGame},
	{"SaveStartData", SaveStartData},
	{"ScreenCG", ScreenCG},
	{"SelectGameLevel", SelectGameLevel},
	{"SetAreaData", SetAreaData},
	{"SetCalendar", SetCalendar},
	{"SetDate", SetDate},
	{"SetDrawTime", SetDrawTime},
	{"SetFace", SetFace},
	{"SetFont", SetFont},
	{"SetFontSize", SetFontSize},
	{"SetMsgClearEffect", SetMsgClearEffect},
	{"SetMsgDrawEffect", SetMsgDrawEffect},
	{"SetMsgFrame", SetMsgFrame},
	{"SetMsgPlaceMethod", SetMsgPlaceMethod},
	{"SetPlayerName", SetPlayerName},
	{"SetScenery", SetScenery},
	{"SetSelMode", SetSelMode},
	{"SetSpriteL", SetSpriteL},
	{"SetSpriteM", SetSpriteM},
	{"SetSpriteR", SetSpriteR},
	{"SetSpriteSeasonL", SetSpriteSeasonL},
	{"SetSpriteSeasonM", SetSpriteSeasonM},
	{"SetSpriteSeasonR", SetSpriteSeasonR},
	{"SetVoice", SetVoice},
	{"SetWallPaper", SetWallPaper},
	{"SoundEffectPlay", SoundEffectPlay},
	{"SoundEffectSetLoop", SoundEffectSetLoop},
	{"SoundEffectSetSyncFlag", SoundEffectSetSyncFlag},
	{"SoundEffectSetVolume", SoundEffectSetVolume},
	{"SoundEffectSetWave", SoundEffectSetWave},
	{"SoundEffectStop", SoundEffectStop},
	{"SoundEffectStopAll", SoundEffectStopAll},
	{"StartNewLine", StartNewLine},
	{"SubFeeling", SubFeeling},
	{"Test", Test},
	{"TestEventCall", TestEventCall},
	{"VisualListAdd", VisualListAdd},
	{"VisualListClear", VisualListClear},
	{"WaitKey", WaitKey},
};

const Module module_NIGHTDLL = {"NIGHTDLL", functions, sizeof(functions) / sizeof(ModuleFunc), NIGHTDLL_reset};
