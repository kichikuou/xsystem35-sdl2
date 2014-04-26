#ifndef __NT_SOUND_H__
#define __NT_SOUND_H__

#include "portab.h"

extern void nt_voice_set(int no);
extern void nt_cd_play(int no);
extern void nt_cd_stop(int msec);
extern void nt_cd_mute(boolean mute);
extern void nt_snd_setwave(int ch, int no);
extern void nt_snd_setloop(int ch, int num);
extern void nt_snd_setvol(int ch, int vol);
extern void nt_snd_waitend(int ch, boolean waitend);
extern void nt_snd_play(int ch);
extern void nt_snd_stop(int ch, int msec);
extern void nt_snd_stopall(int msec);

#endif /* __NT_SOUND_H__ */
