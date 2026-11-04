#ifndef RR_SUB_NOTICE_H
#define RR_SUB_NOTICE_H
#include "rr_race_ui.h"
typedef struct RRSubNotice {char text[128];double timer;bool shown;} RRSubNotice;
/* Browser HUD.sub uses textContent: preserve plain text, never parse markup. */
static inline void rr_sub_notice_set(RRSubNotice *n,const char *text,double duration)
{size_t bytes=strlen(text);if(bytes>=sizeof(n->text))bytes=sizeof(n->text)-1;memmove(n->text,text,bytes);n->text[bytes]=0;n->timer=duration;n->shown=true;}
static inline void rr_sub_notice_checkpoint(RRSubNotice *n,int passed,int total,double time)
{char clock[32];rr_ui_time(clock,time);snprintf(n->text,sizeof(n->text),"CHECKPOINT %d/%d \xc2\xb7 %s",passed,total,clock);n->timer=1.4;n->shown=true;}
static inline void rr_sub_notice_lap(RRSubNotice *n,double time,bool best)
{char clock[32];rr_ui_time(clock,time);snprintf(n->text,sizeof(n->text),"%s%s",best?"\xe2\x98\x85 BEST LAP ":"LAP ",clock);n->timer=2.4;n->shown=true;}
static inline void rr_sub_notice_spin(RRSubNotice *n)
{snprintf(n->text,sizeof(n->text),"SPIN \xc2\xb7 1.5\xc3\x97 SPEED");n->timer=.9;n->shown=true;}
static inline void rr_sub_notice_tick(RRSubNotice *n,double dt)
{n->timer-=dt;if(n->timer<=0)n->shown=false;}
#endif
