#ifndef RR_HUD_MESSAGE_H
#define RR_HUD_MESSAGE_H
#include <stdbool.h>
#include <stdio.h>
typedef struct RRHUDMessage {char text[64];double timer;bool shown,warn;} RRHUDMessage;
/* Source HUD.message replaces content, warn class and timer synchronously. */
static inline void rr_hud_message_set(RRHUDMessage *m,const char *text,double duration,bool warn)
{snprintf(m->text,sizeof(m->text),"%s",text);m->timer=duration;m->shown=true;m->warn=warn;}
static inline void rr_hud_message_lap(RRHUDMessage *m,int lap,int total)
{char text[64];if(lap==total)snprintf(text,sizeof(text),"FINAL LAP");else snprintf(text,sizeof(text),"LAP %d",lap);rr_hud_message_set(m,text,1.6,false);}
/* Current production client is solo; multiplayer place announcements remain gated. */
static inline void rr_hud_message_finish(RRHUDMessage *m){rr_hud_message_set(m,"FINISH",3,false);}
static inline void rr_hud_message_tick(RRHUDMessage *m,double dt)
{m->timer-=dt;if(m->timer<=0)m->shown=false;}
#endif
