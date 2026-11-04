#include "rr_speed_units.h"
#include "rr_menu_keys.h"
#include <stdio.h>
#include <string.h>
static int failures;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"speed units line%d\n",__LINE__);failures++;}}while(0)
int main(void){
 CHECK(rr_speed_readout(25,RR_SPEED_KMH)==90);CHECK(rr_speed_readout(25,RR_SPEED_MPH)==56);
 CHECK(rr_speed_readout(hypot(30,40),RR_SPEED_KMH)==180);CHECK(rr_speed_readout(hypot(30,40),RR_SPEED_MPH)==112);
 CHECK(rr_speed_readout(-25,RR_SPEED_MPH)==56);CHECK(rr_speed_readout(NAN,RR_SPEED_MPH)==0);CHECK(rr_speed_readout(INFINITY,RR_SPEED_KMH)==0);
 CHECK(!strcmp(rr_speed_label(RR_SPEED_KMH),"km/h")&&!strcmp(rr_speed_label(RR_SPEED_MPH),"mph"));
 for(int i=0;i<20001;i++){double speed=i*.01;CHECK(rr_speed_readout(speed,RR_SPEED_KMH)==(int)floor(speed*3.6+.5));CHECK(rr_speed_readout(speed,RR_SPEED_MPH)==(int)floor(speed*2.23694+.5));}
 RRMenuKeys k;rr_menu_keys_reset(&k);k.focus=RR_MENU_FOCUS_UNITS;RRMouseSettings m={1,false};RRMenuKeyFrame f={0};f.active=f.focused=true;f.enabled=1u<<RR_MENU_FOCUS_UNITS;f.right=true;
 RRMenuKeyResult out=rr_menu_keys_step(&k,f,&m);CHECK(out.activate==-1&&rr_speed_select(RR_SPEED_KMH,out.units_command)==RR_SPEED_MPH);
 f.right=false;f.home=true;out=rr_menu_keys_step(&k,f,&m);CHECK(rr_speed_select(RR_SPEED_MPH,out.units_command)==RR_SPEED_KMH);
 f.home=false;f.enter=true;out=rr_menu_keys_step(&k,f,&m);CHECK(out.activate==-1&&rr_speed_select(RR_SPEED_KMH,out.units_command)==RR_SPEED_MPH);
 f.focused=false;out=rr_menu_keys_step(&k,f,&m);CHECK(out.units_command==0&&out.activate==-1);CHECK(m.sensitivity==1&&!m.invert);
 printf("speed units: browser coefficients/rounding,40002 sweep comparisons,combined speed,keyboard isolation; failures=%d\n",failures);return failures?1:0;
}
