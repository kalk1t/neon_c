#include "rr_overlay_keys.h"
#include "rr_session.h"
#include "overlay_keys_reference.h"
#include <stdio.h>
static int checks,failures;
#define CHECK(x) do{checks++;if(!(x)){if(failures<8)fprintf(stderr,"overlay keys line%d\n",__LINE__);failures++;}}while(0)
static RROverlayKeyFrame frame(int kind,unsigned enabled){RROverlayKeyFrame f={0};f.kind=kind;f.enabled=enabled;f.focused=true;f.pointer_target=-1;return f;}
int main(void){
 RROverlayKeys k;rr_overlay_keys_reset(&k);CHECK(k.kind==0&&k.focus==-1);
 for(int kind=RR_OVERLAY_PAUSE;kind<=RR_OVERLAY_RESULTS;kind++){
  const int *order=kind==RR_OVERLAY_PAUSE?source_pause_actions:source_results_actions;int count=kind==RR_OVERLAY_PAUSE?3:2;
  unsigned enabled=kind==RR_OVERLAY_PAUSE?7u:6u;
  for(int direction=0;direction<2;direction++){
   rr_overlay_keys_reset(&k);RROverlayKeyFrame f=frame(kind,enabled);f.tab=true;f.shift=direction!=0;
   for(int i=0;i<count*4;i++){CHECK(rr_overlay_keys_step(&k,f)==-1);CHECK(k.focus==order[direction?count-1-i%count:i%count]);CHECK(rr_overlay_keys_visible(&k,k.focus,true)&&!rr_overlay_keys_visible(&k,k.focus,false));}
  }
 }
 // All masks, initial positions and both directions: bounded traversal never selects a disabled button.
 for(unsigned mask=0;mask<8;mask++)for(int at=-1;at<3;at++)for(int reverse=0;reverse<2;reverse++){
  rr_overlay_keys_reset(&k);k.kind=RR_OVERLAY_PAUSE;k.focus=at;RROverlayKeyFrame f=frame(RR_OVERLAY_PAUSE,mask);f.tab=true;f.shift=reverse!=0;
  CHECK(rr_overlay_keys_step(&k,f)==-1);CHECK(mask?rr_overlay_key_enabled(mask,k.focus):k.focus==-1);
 }
 // Pointer selection does not synthesize activation, blank press clears focus; Enter uses that selection only.
 for(int slot=0;slot<3;slot++){
  rr_overlay_keys_reset(&k);RROverlayKeyFrame f=frame(RR_OVERLAY_PAUSE,7);f.pointer_pressed=true;f.pointer_target=slot;
  CHECK(rr_overlay_keys_step(&k,f)==-1&&k.focus==slot&&!k.keyboard);f.pointer_pressed=false;f.enter=true;CHECK(rr_overlay_keys_step(&k,f)==slot);
  f.enter=false;CHECK(rr_overlay_keys_step(&k,f)==-1);f.enter=true;f.focused=false;CHECK(rr_overlay_keys_step(&k,f)==-1);
  f.focused=true;f.pointer_pressed=true;f.pointer_target=-1;CHECK(rr_overlay_keys_step(&k,f)==-1&&k.focus==-1);
 }
 rr_overlay_keys_reset(&k);RROverlayKeyFrame f=frame(RR_OVERLAY_PAUSE,7);f.tab=true;rr_overlay_keys_step(&k,f);f.kind=RR_OVERLAY_RESULTS;f.enabled=6;f.tab=false;f.enter=true;
 CHECK(rr_overlay_keys_step(&k,f)==-1&&k.focus==-1);CHECK(rr_overlay_keys_default_restart(&k,true,true,6));CHECK(!rr_overlay_keys_default_restart(&k,false,true,6));CHECK(!rr_overlay_keys_default_restart(&k,true,true,4));
 f.tab=true;CHECK(rr_overlay_keys_step(&k,f)==RR_OVERLAY_RESTART);f.tab=false;CHECK(!rr_overlay_keys_default_restart(&k,true,true,6));
 f.tab=true;CHECK(rr_overlay_keys_step(&k,f)==RR_OVERLAY_MENU);CHECK(!rr_overlay_keys_default_restart(&k,true,true,6));
 // Error/results page exposes only MENU; removing a focused action invalidates it before Enter.
 f.enabled=4;f.tab=false;k.focus=RR_OVERLAY_RESTART;CHECK(rr_overlay_keys_step(&k,f)==-1&&k.focus==-1);f.tab=true;CHECK(rr_overlay_keys_step(&k,f)==RR_OVERLAY_MENU);
 f.kind=RR_OVERLAY_NONE;CHECK(rr_overlay_keys_step(&k,f)==-1&&k.kind==0&&k.focus==-1);
 // Real solo session lifecycle: dispatcher result controls existing resume/restart/menu functions across four return phases.
 RRSession session;RRCar car;rr_session_init(&session);RRSessionPhase phases[]={RR_SESSION_COUNTDOWN,RR_SESSION_ACTIVE,RR_SESSION_FINISHED,RR_SESSION_RESULTS};
 for(int phase=0;phase<4;phase++)for(int action=0;action<3;action++){
  rr_session_start(&session,&car,RR_RACE_TIME_ATTACK);session.phase=phases[phase];CHECK(rr_session_pause(&session));
  rr_overlay_keys_reset(&k);f=frame(RR_OVERLAY_PAUSE,7);f.tab=true;
  for(int i=0;i<=action;i++)CHECK(rr_overlay_keys_step(&k,f)==-1);
  f.tab=false;f.enter=true;int got=rr_overlay_keys_step(&k,f);CHECK(got==source_pause_actions[action]);
  if(got==RR_OVERLAY_RESUME){CHECK(rr_session_resume(&session));CHECK(session.phase==phases[phase]);}
  else if(got==RR_OVERLAY_RESTART){rr_session_start(&session,&car,RR_RACE_TIME_ATTACK);CHECK(session.phase==RR_SESSION_COUNTDOWN&&session.lap_count==0);}
  else if(got==RR_OVERLAY_MENU){rr_session_menu(&session);CHECK(session.phase==RR_SESSION_MENU);}
 }
 rr_session_dispose(&session);
 printf("Offline overlays: source-enabled action order, all masks/directions, Enter/focus/blur/pointer/reset, result default isolation, 12 real session lifecycle cases; checks=%d failures=%d; no UA/graphics/hardware/account qualification\n",checks,failures);return failures?1:0;
}
