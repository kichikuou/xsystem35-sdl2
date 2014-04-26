#ifndef __MUSIC_BGM_H__
#define __MUSIC_BGM_H__

extern int musbgm_init();
extern int musbgm_play(int no, int time, int vol);
extern int musbgm_stop(int no, int time);
extern int musbgm_fade(int no, int time, int vol);
extern int musbgm_getpos(int no);
extern int musbgm_getlen(int no);
extern int musbgm_isplaying(int no);
extern int musbgm_stopall(int time);

#endif /* __MUSIC_BGM_H__ */
