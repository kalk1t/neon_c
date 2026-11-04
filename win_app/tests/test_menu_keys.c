#include "rr_menu_keys.h"
#include "rr_menu_policy.h"
#include <stdio.h>
typedef struct SourceMenuKey{bool input;int code;bool pressed,prevented;}SourceMenuKey;
#include "menu_keys_fixture.h"
static int failures,checks;
#define CHECK(v) do{checks++;if(!(v)){if(failures<8)fprintf(stderr,"menu keyboard line%d\n",__LINE__);failures++;}}while(0)
static RRMenuKeyFrame frame(unsigned mask){RRMenuKeyFrame f={0};f.active=f.focused=true;f.enabled=mask;f.pointer_target=-1;return f;}
int main(void){RRMenuKeys k;RRMouseSettings settings={1,false};unsigned all=(1u<<RR_MENU_FOCUS_COUNT)-1;RRMenuKeyResult r;
 CHECK(source_sens_min==.3&&source_sens_max==2.5&&source_sens_step==.05);
 rr_menu_keys_reset(&k);RRMenuKeyFrame f=frame(all);f.tab=true;for(int i=0;i<12;i++){rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==i%RR_MENU_FOCUS_COUNT&&k.keyboard);}
 f.shift=true;for(int i=0;i<12;i++){rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==(RR_MENU_FOCUS_COUNT-2-i%RR_MENU_FOCUS_COUNT+RR_MENU_FOCUS_COUNT)%RR_MENU_FOCUS_COUNT);}
 // Source-supported web subset has no native QUIT; failed scene skips start buttons.
 rr_menu_keys_reset(&k);f=frame(15);f.tab=true;for(int i=0;i<8;i++){rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==i%4);}
 rr_menu_keys_reset(&k);f=frame((1u<<2)|(1u<<3));f.tab=true;for(int i=0;i<8;i++){rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==2+i%2);}
 rr_menu_keys_reset(&k);f=frame(0);f.tab=true;rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==-1);
 // Tab to TIME ATTACK then Enter must request the time trial, not the old global race shortcut.
 rr_menu_keys_reset(&k);f=frame(all);f.tab=true;rr_menu_keys_step(&k,f,&settings);rr_menu_keys_step(&k,f,&settings);f.tab=false;f.enter=true;r=rr_menu_keys_step(&k,f,&settings);CHECK(r.activate==RR_MENU_FOCUS_ATTACK);
 RRMenuPolicy policy={0};CHECK(rr_menu_request(&policy,true,false,false,false)==RR_MENU_NO_ACTION);CHECK(rr_menu_request(&policy,true,true,r.activate==0,r.activate==1)==RR_MENU_START_TIME_ATTACK);CHECK(rr_menu_request(&policy,true,true,true,false)==RR_MENU_NO_ACTION);
 for(int slot=2;slot<=3;slot++){k.focus=slot;r=rr_menu_keys_step(&k,f,&settings);CHECK(r.activate==-1&&rr_menu_keys_input_focus(&k));}
 k.focus=RR_MENU_FOCUS_QUIT;r=rr_menu_keys_step(&k,f,&settings);CHECK(r.activate==RR_MENU_FOCUS_QUIT);rr_menu_keys_reset(&k);r=rr_menu_keys_step(&k,f,&settings);CHECK(r.activate==RR_MENU_FOCUS_RACE);
 // Every source range step, both signs and Home/End endpoints; action keys never start a race.
 for(int i=0;i<45;i++)for(int key=0;key<6;key++){
  rr_menu_keys_reset(&k);k.focus=RR_MENU_FOCUS_SENS;settings.sensitivity=source_sens_min+i*source_sens_step;f=frame(all);f.left=key==0;f.down=key==1;f.right=key==2;f.up=key==3;f.home=key==4;f.end=key==5;
  r=rr_menu_keys_step(&k,f,&settings);int expected=key<2?(i>0?i-1:0):key<4?(i<44?i+1:44):key==4?0:44;CHECK(fabs(settings.sensitivity-(source_sens_min+expected*source_sens_step))<1e-12);CHECK(r.activate==-1&&r.sensitivity_changed==(expected!=i)&&k.keyboard);
 }
 // Owned checkbox Space release, repeat/pulse, Tab/pointer/blur/screen cancellation.
 rr_menu_keys_reset(&k);k.focus=3;settings.invert=false;f=frame(all);f.space_pressed=f.space_held=true;r=rr_menu_keys_step(&k,f,&settings);CHECK(!r.invert_changed&&!settings.invert);f.space_pressed=false;rr_menu_keys_step(&k,f,&settings);f.space_held=false;f.space_released=true;r=rr_menu_keys_step(&k,f,&settings);CHECK(r.invert_changed&&settings.invert&&r.activate==-1);r=rr_menu_keys_step(&k,f,&settings);CHECK(!r.invert_changed&&settings.invert);
 f=frame(all);f.space_pressed=f.space_released=true;r=rr_menu_keys_step(&k,f,&settings);CHECK(r.invert_changed&&!settings.invert);
 for(int cancel=0;cancel<4;cancel++){
  k.focus=3;f=frame(all);f.space_pressed=f.space_held=true;rr_menu_keys_step(&k,f,&settings);f.space_pressed=false;
  if(cancel==0)f.tab=true;if(cancel==1){f.pointer_pressed=true;f.pointer_target=2;}if(cancel==2)f.focused=false;if(cancel==3)f.active=false;rr_menu_keys_step(&k,f,&settings);f=frame(all);f.space_released=true;r=rr_menu_keys_step(&k,f,&settings);CHECK(!r.invert_changed&&!settings.invert);
 }
 // BUTTON Space default is canceled by actual browser Input; no native activation.
 for(int slot=0;slot<2;slot++){k.focus=slot;f=frame(all);f.space_pressed=f.space_released=true;r=rr_menu_keys_step(&k,f,&settings);CHECK(r.activate==-1);}
 // Pointer focuses the source subset without a keyboard cue; Tab continues from it.
 f=frame(all);f.pointer_pressed=true;f.pointer_target=2;rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==2&&!k.keyboard);f.pointer_pressed=false;f.tab=true;rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==3&&rr_menu_keys_visible(&k,3,true)&&!rr_menu_keys_visible(&k,3,false));
 f.active=false;rr_menu_keys_step(&k,f,&settings);CHECK(k.focus==-1&&!rr_menu_keys_input_focus(&k));
 for(size_t i=0;i<sizeof(source_menu_keys)/sizeof(source_menu_keys[0]);i++){const SourceMenuKey *s=&source_menu_keys[i];CHECK(s->pressed!=s->input);CHECK(s->prevented==(!s->input&&s->code>=1&&s->code<=5));}
 printf("Enabled menu keyboard:30 actual Input keydown constraints,45-step range,DOM subset order,Tab/Enter/time-attack,owned checkbox/cancellation; %d checks, %d failures; no GPU/UA/OS\n",checks,failures);return failures?1:0;
}
