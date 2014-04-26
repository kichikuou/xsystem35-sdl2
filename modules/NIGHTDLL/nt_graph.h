#ifndef __NT_GRAPH_H__
#define __NT_GRAPH_H__

extern void nt_gr_init();
extern void nt_gr_set_wallpaper(int no);
extern void nt_gr_set_scenery(int no);
extern void nt_gr_set_face(int no);
extern void nt_gr_set_spL(int no);
extern void nt_gr_set_spM(int no);
extern void nt_gr_set_spR(int no);
extern void nt_gr_set_spsL(int no);
extern void nt_gr_set_spsM(int no);
extern void nt_gr_set_spsR(int no);
extern void nt_gr_set_drawtime(int msec);
extern void nt_gr_draw(int effectno);
extern void nt_gr_screencg(int no, int x, int y);

#endif /* __NT_GRAPH_H__ */
