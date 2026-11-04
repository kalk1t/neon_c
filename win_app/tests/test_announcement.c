#include "rr_announcement.h"
#include "rr_hud_message.h"
#include <stdio.h>
#include <string.h>
typedef struct NoticeCase {int sequence,op;double now,dt;const char *text;double duration;int warn,visible,shown,rendered;double opacity;const char *expected_text;int expected_warn;} NoticeCase;
typedef struct GeometryCase {double w,h,z;int warn;double y,height;uint32_t color;} GeometryCase;
#include "announcement_fixture.h"
static int failures;static double worst;
#define CHECK(v) do{if(!(v)){if(failures<5)fprintf(stderr,"line%d: %s\n",__LINE__,#v);failures++;}}while(0)
int main(void){RRNoticeFade fade={0};RRHUDMessage message={0};int sequence=-1;bool visible=false;
for(size_t i=0;i<sizeof(notice_cases)/sizeof(notice_cases[0]);i++){
 const NoticeCase *c=&notice_cases[i];if(c->sequence!=sequence){fade=(RRNoticeFade){0};message=(RRHUDMessage){0};visible=false;sequence=c->sequence;}
 if(c->op==0)rr_hud_message_set(&message,c->text,c->duration,c->warn!=0);else if(c->op==1)rr_hud_message_tick(&message,c->dt);else if(c->op==2)visible=c->visible!=0;
 CHECK(message.shown==(c->shown!=0));CHECK(visible==(c->rendered!=0));CHECK(message.warn==(c->expected_warn!=0));CHECK(strcmp(message.text,c->expected_text)==0);
 double opacity=rr_notice_observe(&fade,message.shown,visible,c->now),error=fabs(opacity-c->opacity);if(error>worst)worst=error;
 if(error>2e-9){if(failures<5)fprintf(stderr,"state%zu opacity%.17g expected%.17g\n",i,opacity,c->opacity);failures++;}CHECK(opacity>=0&&opacity<=1);CHECK(fade.duration>=0&&fade.duration<=.25);
}
for(size_t i=0;i<sizeof(geometry_cases)/sizeof(geometry_cases[0]);i++){
 const GeometryCase *c=&geometry_cases[i];RRAnnouncementLayout a=rr_announcement_layout(c->w,c->h,c->z,c->warn!=0);
 CHECK(a.x==0&&a.width==c->w&&a.cx==c->w/2);CHECK(fabs(a.y-c->y)<1e-12&&fabs(a.height-c->height)<1e-12);CHECK(a.color==c->color);CHECK(a.font==84&&a.tracking==6);CHECK(a.near_glow==(c->warn?UINT32_C(0xff2bd6ff):UINT32_C(0x19e6ffff)));CHECK(a.far_glow==UINT32_C(0xff2bd6ff));
 CHECK(fabs(a.baseline-(c->y+(105-84*1.254)/2+84*1.011))<1e-12);
}
RRAnnouncementLayout compact=rr_announcement_layout(1280,720,.8,false);CHECK(fabs(compact.y-172.8)<1e-12&&compact.height==84);CHECK(fabs(compact.font-67.2)<1e-12);
/* Wall-clock sampling can finish while simulation does not advance. */
fade=(RRNoticeFade){0};rr_notice_observe(&fade,false,true,0);rr_notice_observe(&fade,true,true,.01);CHECK(rr_notice_observe(&fade,true,true,5)==1);CHECK(rr_notice_observe(&fade,false,true,5)==1);CHECK(rr_notice_observe(&fade,false,true,6)==0);
/* Repeated true does not restart a transition. Hidden parent snaps to current
 * target on re-render, rather than replaying a cancelled transition. */
fade=(RRNoticeFade){0};rr_notice_observe(&fade,false,true,0);rr_notice_observe(&fade,true,true,1);double start=fade.started;rr_notice_observe(&fade,true,true,1.1);CHECK(fade.started==start);rr_notice_observe(&fade,true,false,1.15);CHECK(rr_notice_observe(&fade,true,true,1.2)==1);
printf("Announcement CSS: %zu actual-source DOM/model states across24 sequences, two reused boxes; worst opacity error%.12g, %d failures; no graphics\n",sizeof(notice_cases)/sizeof(notice_cases[0]),worst,failures);return failures?1:0;}

