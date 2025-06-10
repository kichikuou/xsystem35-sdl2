#ifndef __NT_MSG_H__
#define __NT_MSG_H__

void ntmsg_init();
void ntmsg_set_frame(int type);
void ntmsg_set_place(int type);
void ntmsg_newline();
void ntmsg_add(const char *msg);
int  ntmsg_ana(void);
void ntmsg_update(sprite_t *sp, SDL_Rect *r);

#endif /* __NT_MSG_H__ */
