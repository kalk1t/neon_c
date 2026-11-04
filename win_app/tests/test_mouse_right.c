#include "rr_mouse_right.h"
#include "rr_mouse_aim.h"
#include <stdio.h>
typedef struct RightCase{int scenario,before,release,focused,eligible,pressed,physical,expected,captured;}RightCase;
#include "mouse_right_reference.h"
static int checks,failures,old_mismatches;
#define CHECK(x) do{checks++;if(!(x)){if(failures<8)fprintf(stderr,"mouse right line%d\n",__LINE__);failures++;}}while(0)
int main(void){
 for(size_t i=0;i<sizeof(right_cases)/sizeof(right_cases[0]);i++){
  const RightCase *c=&right_cases[i];RRMouseRight s={c->before!=0};if(c->release)rr_mouse_right_release(&s);
  bool actual=rr_mouse_right_step(&s,c->focused!=0,c->eligible!=0,c->pressed!=0,c->physical!=0);CHECK(actual==(c->expected!=0)&&s.held==actual);
  if(c->captured&&c->focused&&(c->physical!=c->expected))old_mismatches++;
 }
 CHECK(old_mismatches>0);
 for(int was=0;was<2;was++)for(int now=0;now<2;now++)for(int logical=0;logical<2;logical++){
  RRMouseRight transition={logical!=0};rr_mouse_right_capture_change(&transition,was!=0,now!=0);
  CHECK(transition.held==((was&&!now)?false:logical!=0));
 }

 for(int phase=RR_SESSION_MENU;phase<=RR_SESSION_ERROR;phase++){
  bool visible=phase==RR_SESSION_ACTIVE||phase==RR_SESSION_COUNTDOWN||phase==RR_SESSION_FINISHED;
  CHECK(rr_mouse_right_canvas((RRSessionPhase)phase,0,0,960,720)==visible);
  CHECK(rr_mouse_right_canvas((RRSessionPhase)phase,959.5,719.5,960,720)==visible);
  CHECK(!rr_mouse_right_canvas((RRSessionPhase)phase,-1,1,960,720));CHECK(!rr_mouse_right_canvas((RRSessionPhase)phase,960,1,960,720));
  CHECK(!rr_mouse_right_canvas((RRSessionPhase)phase,1,720,960,720));CHECK(!rr_mouse_right_canvas((RRSessionPhase)phase,NAN,1,960,720));
 }

 // A canvas press can survive acquisition/countdown; losing capture releases
 // immediately, recapture/held polling cannot rearm, new press can.
 RRMouseRight s={0};CHECK(rr_mouse_right_step(&s,true,true,true,true));CHECK(rr_mouse_right_step(&s,true,true,false,true));
 RRMouseAim aim={0};RRMouseSettings settings={1,false};RRInput controls=rr_mouse_aim_step(&aim,settings,0,0,(RRInput){0},10,5,s.held);
 CHECK(aim.looking&&fabs(aim.look_yaw+.0336)<1e-12&&aim.yaw==0);(void)controls;
 rr_mouse_right_capture_change(&s,true,false);CHECK(!s.held);CHECK(!rr_mouse_right_step(&s,true,true,false,true));
 double look=aim.look_yaw;controls=rr_mouse_aim_step(&aim,settings,0,0,(RRInput){0},10,5,s.held);
 CHECK(!aim.looking&&aim.look_yaw==look&&fabs(aim.yaw+.021)<1e-12&&controls.steer>0);
 CHECK(rr_mouse_right_step(&s,true,true,true,true));CHECK(!rr_mouse_right_step(&s,true,true,true,false));
 CHECK(!rr_mouse_right_step(&s,true,false,true,true));CHECK(!rr_mouse_right_step(&s,true,true,false,true));CHECK(rr_mouse_right_step(&s,true,true,true,true));
 CHECK(!rr_mouse_right_step(&s,false,true,false,true));CHECK(!rr_mouse_right_step(&s,true,true,false,true));
 CHECK(rr_mouse_right_step(&s,true,true,true,true));CHECK(!rr_mouse_right_step(&s,true,true,false,false));
 printf("RMB lifecycle:128 actual-source exhaustive transitions plus aim/free-look coupling, capture-loss/held-recapture/canvas/overlay/blur/coalesced-release; checks=%d old_raw_hold_mismatches=%d failures=%d; no physical input/graphics/per-event delta-filter qualification\n",checks,old_mismatches,failures);return failures?1:0;
}
