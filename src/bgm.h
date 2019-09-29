#ifndef __BGM_H__
#define __BGM_H__

extern int musbgm_init();
extern int musbgm_exit();
extern int musbgm_play(int no, int time, int vol);
extern int musbgm_stop(int no, int time);
extern int musbgm_fade(int no, int time, int vol);
extern int musbgm_getpos(int no);
extern int musbgm_getlen(int no);
extern int musbgm_isplaying(int no);
extern int musbgm_stopall(int time);
extern int musbgm_wait(int no, int timeout);

#endif /* __BGM_H__ */
