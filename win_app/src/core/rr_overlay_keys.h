#ifndef RR_OVERLAY_KEYS_H
#define RR_OVERLAY_KEYS_H
#include <stdbool.h>
enum {RR_OVERLAY_NONE,RR_OVERLAY_PAUSE,RR_OVERLAY_RESULTS};
enum {RR_OVERLAY_NO_ACTION=-1,RR_OVERLAY_RESUME,RR_OVERLAY_RESTART,RR_OVERLAY_MENU,RR_OVERLAY_ACTIONS};
typedef struct RROverlayKeys {int kind,focus;bool keyboard;} RROverlayKeys;
typedef struct RROverlayKeyFrame {int kind,pointer_target;unsigned enabled;bool focused,tab,shift,enter,pointer_pressed;} RROverlayKeyFrame;
static inline void rr_overlay_keys_reset(RROverlayKeys *k){*k=(RROverlayKeys){RR_OVERLAY_NONE,RR_OVERLAY_NO_ACTION,false};}
static inline bool rr_overlay_key_enabled(unsigned enabled,int at){return at>=0&&at<RR_OVERLAY_ACTIONS&&(enabled&(1u<<at));}
/* Enabled native subset of source buttons, in DOM order. Enter activates once;
 * Space is deliberately absent: source Input cancels its BUTTON default.
 * Browser links/popups/UA focus mechanics are outside this adapter's scope. */
static inline int rr_overlay_keys_step(RROverlayKeys *k,RROverlayKeyFrame f){
 if(f.kind!=k->kind){rr_overlay_keys_reset(k);k->kind=f.kind;}
 if(f.kind!=RR_OVERLAY_PAUSE&&f.kind!=RR_OVERLAY_RESULTS){rr_overlay_keys_reset(k);return RR_OVERLAY_NO_ACTION;}
 if(!rr_overlay_key_enabled(f.enabled,k->focus))k->focus=RR_OVERLAY_NO_ACTION;
 if(!f.focused)return RR_OVERLAY_NO_ACTION;
 if(f.pointer_pressed){k->focus=rr_overlay_key_enabled(f.enabled,f.pointer_target)?f.pointer_target:RR_OVERLAY_NO_ACTION;k->keyboard=false;}
 if(f.tab){int at=k->focus;if(at<0)at=f.shift?0:-1;k->focus=RR_OVERLAY_NO_ACTION;
  for(int i=0;i<RR_OVERLAY_ACTIONS;i++){at=(at+(f.shift?RR_OVERLAY_ACTIONS-1:1))%RR_OVERLAY_ACTIONS;if(rr_overlay_key_enabled(f.enabled,at)){k->focus=at;break;}}
  k->keyboard=true;
 }
 return f.enter&&rr_overlay_key_enabled(f.enabled,k->focus)?k->focus:RR_OVERLAY_NO_ACTION;
}
static inline bool rr_overlay_keys_default_restart(const RROverlayKeys *k,bool focused,bool enter,unsigned enabled)
{return k->kind==RR_OVERLAY_RESULTS&&focused&&enter&&k->focus<0&&rr_overlay_key_enabled(enabled,RR_OVERLAY_RESTART);}
static inline bool rr_overlay_keys_visible(const RROverlayKeys *k,int action,bool focused){return focused&&k->keyboard&&k->focus==action;}
#endif
