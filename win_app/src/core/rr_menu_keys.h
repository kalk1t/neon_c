#ifndef RR_MENU_KEYS_H
#define RR_MENU_KEYS_H
#include "rr_mouse_aim.h"
enum {RR_MENU_FOCUS_NONE=-1,RR_MENU_FOCUS_RACE,RR_MENU_FOCUS_ATTACK,RR_MENU_FOCUS_SENS,RR_MENU_FOCUS_INVERT,RR_MENU_FOCUS_UNITS,RR_MENU_FOCUS_QUIT,RR_MENU_FOCUS_COUNT};
typedef struct RRMenuKeys {int focus,space_owner;bool keyboard;} RRMenuKeys;
typedef struct RRMenuKeyFrame {
 bool active,focused,tab,shift,enter,space_pressed,space_released,space_held;
 bool left,right,up,down,home,end,pointer_pressed;int pointer_target;unsigned enabled;
} RRMenuKeyFrame;
typedef struct RRMenuKeyResult {int activate;bool sensitivity_changed,invert_changed;int units_command;} RRMenuKeyResult;
static inline void rr_menu_keys_reset(RRMenuKeys *k){*k=(RRMenuKeys){RR_MENU_FOCUS_NONE,RR_MENU_FOCUS_NONE,false};}
static inline bool rr_menu_key_enabled(unsigned mask,int slot){return slot>=0&&slot<RR_MENU_FOCUS_COUNT&&(mask&(1u<<slot));}
static inline bool rr_menu_keys_input_focus(const RRMenuKeys *k){return k->focus==RR_MENU_FOCUS_SENS||k->focus==RR_MENU_FOCUS_INVERT||k->focus==RR_MENU_FOCUS_UNITS;}
static inline bool rr_menu_keys_visible(const RRMenuKeys *k,int slot,bool focused){return focused&&k->keyboard&&k->focus==slot;}
/* Enabled offline subset follows browser DOM order. Platform supplies one frame
 * of key edges/repeats and pointer focus. No persistence/account/UI calls. */
static inline RRMenuKeyResult rr_menu_keys_step(RRMenuKeys *k,RRMenuKeyFrame f,RRMouseSettings *settings)
{
 RRMenuKeyResult result={RR_MENU_FOCUS_NONE,false,false};
 if(!f.active){rr_menu_keys_reset(k);return result;}
 if(!rr_menu_key_enabled(f.enabled,k->focus))k->focus=RR_MENU_FOCUS_NONE;
 if(!f.focused){k->space_owner=RR_MENU_FOCUS_NONE;return result;}
 if(f.pointer_pressed){k->focus=rr_menu_key_enabled(f.enabled,f.pointer_target)?f.pointer_target:RR_MENU_FOCUS_NONE;k->keyboard=false;k->space_owner=RR_MENU_FOCUS_NONE;}
 if(f.tab){
  int at=k->focus;if(at==RR_MENU_FOCUS_NONE)at=f.shift?0:-1;
  k->focus=RR_MENU_FOCUS_NONE;
  for(int i=0;i<RR_MENU_FOCUS_COUNT;i++){at=(at+(f.shift?RR_MENU_FOCUS_COUNT-1:1))%RR_MENU_FOCUS_COUNT;if(rr_menu_key_enabled(f.enabled,at)){k->focus=at;break;}}
  k->keyboard=true;k->space_owner=RR_MENU_FOCUS_NONE;
 }
 if(k->focus==RR_MENU_FOCUS_SENS){
  int index=(int)floor(rr_clamp((settings->sensitivity-.3)/.05,0,44)+.5),next=index;
  if(f.home)next=0;else if(f.end)next=44;else next+=(int)f.right+(int)f.up-(int)f.left-(int)f.down;
  next=(int)rr_clamp(next,0,44);
  if(f.home||f.end||f.left||f.right||f.up||f.down){k->keyboard=true;settings->sensitivity=.3+next*.05;result.sensitivity_changed=next!=index;}
 }
 if(k->focus==RR_MENU_FOCUS_UNITS){
  if(f.home||f.left||f.up)result.units_command=-1;
  else if(f.end||f.right||f.down)result.units_command=1;
  else if(f.enter||f.space_pressed)result.units_command=3;
  if(result.units_command)k->keyboard=true;
 }
 // Browser Input prevents Space default on BUTTON, but bypasses INPUT.
 // Checkbox Space is an owned press/release; Enter on either INPUT is inert.
 if(f.space_pressed&&k->focus==RR_MENU_FOCUS_INVERT){k->space_owner=k->focus;k->keyboard=true;}
 if(f.space_released){if(k->space_owner==RR_MENU_FOCUS_INVERT&&k->focus==k->space_owner){settings->invert=!settings->invert;result.invert_changed=true;}k->space_owner=RR_MENU_FOCUS_NONE;}
 if(!f.space_held&&!f.space_pressed)k->space_owner=RR_MENU_FOCUS_NONE;
 if(f.enter){
  int target=k->focus==RR_MENU_FOCUS_NONE?RR_MENU_FOCUS_RACE:k->focus;
  if(rr_menu_key_enabled(f.enabled,target)&&(target==RR_MENU_FOCUS_RACE||target==RR_MENU_FOCUS_ATTACK||target==RR_MENU_FOCUS_QUIT))result.activate=target;
 }
 return result;
}
#endif
