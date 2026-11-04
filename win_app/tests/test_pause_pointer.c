#include "rr_showcase_input.h"
#include "rr_pause_ui.h"
#include "rr_mouse_right.h"
#include "rr_mouse_aim.h"
#include <stdio.h>
typedef struct PointerCase{int sequence,frame,button;double x,y,wheel;unsigned pressed,released,held;int inside,control;double yaw,pitch,dist;int dragging;}PointerCase;
#include "pause_pointer_reference.h"
static int checks,failures,old_mismatches;static double worst;
#define CHECK(x) do{checks++;if(!(x)){if(failures<8)fprintf(stderr,"pause pointer line%d\n",__LINE__);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(e<1e-12);}
int main(void){RRSession session;RRCar car;rr_session_init(&session);RRShowcase s={0},legacy={0};int sequence=-1;double stamp=0;
 for(size_t i=0;i<sizeof(pointer_cases)/sizeof(pointer_cases[0]);i++){
  const PointerCase *c=&pointer_cases[i];if(sequence!=c->sequence){sequence=c->sequence;rr_session_start(&session,&car,RR_RACE_TWO_LAPS);rr_showcase_init(&s);CHECK(rr_ui_pause_showcase(&session,&stamp,1000,&s));legacy=s;}
  RRShowcaseInputFrame f={c->x,c->y,c->wheel,c->pressed,c->released,c->held,true,true,c->inside!=0,c->control!=0};rr_showcase_input_step(&s,f);
  near(s.yaw,c->yaw);near(s.pitch,c->pitch);near(s.dist,c->dist);CHECK(s.dragging==(c->dragging!=0));
  // Previous host sampled only LEFT, with no explicit any-button pointerup.
  if((c->pressed&1u)&&c->inside)rr_showcase_down(&legacy,c->x,c->y,c->control!=0);
  if(!(c->held&1u))rr_showcase_up(&legacy);else rr_showcase_move(&legacy,c->x,c->y);
  if(c->inside&&c->wheel!=0)rr_showcase_wheel(&legacy,c->wheel);
  if(fabs(legacy.yaw-c->yaw)>1e-12||fabs(legacy.pitch-c->pitch)>1e-12||legacy.dragging!=(c->dragging!=0))old_mismatches++;
 }
 CHECK(old_mismatches>0);
 // Native cancellation guards retained across blur/phase exit/missing release;
 // invalid coordinates cannot poison the camera. Coalesced release wins.
 for(int guard=0;guard<5;guard++){
  rr_showcase_init(&s);rr_showcase_down(&s,1,2,false);RRShowcaseInputFrame f={10,20,0,0,0,2,true,true,true,false};
  if(guard==0)f.focused=false;if(guard==1)f.active=false;if(guard==2)f.held=0;if(guard==3)f.x=NAN;if(guard==4){f.pressed=2;f.released=2;f.held=2;}
  rr_showcase_input_step(&s,f);CHECK(!s.dragging&&isfinite(s.yaw)&&isfinite(s.pitch));
 }
 // Pause-overlay RMB gesture rotates the showcase without arming flight free
 // look. Holding it on resume cannot rearm the logical flight button.
 RRMouseRight right={0};rr_showcase_init(&s);RRShowcaseInputFrame f={10,20,0,2,0,2,true,true,true,false};
 CHECK(!rr_mouse_right_step(&right,true,false,true,true));rr_showcase_input_step(&s,f);CHECK(s.dragging);
 f.pressed=0;f.x=40;f.y=50;rr_showcase_input_step(&s,f);CHECK(fabs(s.yaw-.36)<1e-12&&fabs(s.pitch-.29)<1e-12);
 f.active=false;rr_showcase_input_step(&s,f);CHECK(!s.dragging);rr_mouse_right_capture_change(&right,false,true);CHECK(!rr_mouse_right_step(&right,true,true,false,true));
 RRMouseAim aim={0};RRInput ctl=rr_mouse_aim_step(&aim,(RRMouseSettings){1,false},0,0,(RRInput){0},10,0,right.held);CHECK(!aim.looking&&aim.yaw<0&&ctl.steer>0);
 rr_session_dispose(&session);printf("Pause pointer:216 actual-source callback states across18 standard-button/control/wheel sequences, cancellation/coalescing guards and flight-RMB isolation; checks=%d old_primary_only_mismatches=%d max_error=%.12g failures=%d; no hardware/DOM hit-test/graphics qualification\n",checks,old_mismatches,worst,failures);return failures?1:0;
}
