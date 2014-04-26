#ifndef __NT_MSG_H__
#define __NT_MSG_H__

extern void ntmsg_init();
extern void ntmsg_set_frame(int type);
extern void ntmsg_set_place(int type);
extern void ntmsg_newline();
extern void ntmsg_add(char *msg);
extern int  ntmsg_ana(void);
extern int  ntmsg_update(sprite_t *sp, MyRectangle *r);

#endif /* __NT_MSG_H__ */
