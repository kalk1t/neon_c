#include "rr_input_mix.h"
#include <stdio.h>
typedef struct SlideCase{int scenario,start,reset,stun;double speed,yaw,dt;RRInput input;double expected[20];}SlideCase;
typedef struct SlideInputCase{unsigned keys;bool captured;RRInputPad pad;RRInput expected;}SlideInputCase;
#include "slide_fixture.h"
int main(void){
 int failures=0;double worst=0;RRCar car={0};
 for(size_t i=0;i<sizeof(slide_cases)/sizeof(slide_cases[0]);i++){
  const SlideCase *s=&slide_cases[i];if(s->start)rr_car_reset(&car,rr_v3(0,400,0),s->yaw,s->speed);if(s->reset)rr_car_place(&car,rr_v3(0,400,0),s->yaw,0);if(s->stun)car.stun=.7;
  rr_car_step(&car,s->input,s->dt);
  double got[]={car.pos.x,car.pos.y,car.pos.z,car.vel.x,car.vel.y,car.vel.z,car.yaw,car.pitch,car.roll,car.speed,car.side,car.pod_pitch[0],car.pod_roll[0],car.pod_pitch[1],car.pod_roll[1],car.pod_pitch[2],car.pod_roll[2],car.pod_pitch[3],car.pod_roll[3],rr_car_display_speed(&car)};
  for(int k=0;k<20;k++){double e=fabs(got[k]-s->expected[k]);if(e>worst)worst=e;if(!isfinite(got[k])||e>1e-8){if(failures<8)fprintf(stderr,"slide row%zu col%d got%.17g expected%.17g\n",i,k,got[k],s->expected[k]);failures++;}}
 }
 for(size_t i=0;i<sizeof(slide_inputs)/sizeof(slide_inputs[0]);i++){const SlideInputCase *s=&slide_inputs[i];RRInput v=rr_input_mix_capture(s->keys,s->pad,s->captured),w=s->expected;
  if(fabs(v.throttle-w.throttle)>1e-12||fabs(v.brake-w.brake)>1e-12||fabs(v.steer-w.steer)>1e-12||fabs(v.climb-w.climb)>1e-12||v.boost!=w.boost||v.strafe!=w.strafe)failures++;
 }
 // Exact zero, stationary turn direction, reset and total side-speed display.
 rr_car_reset(&car,rr_v3(0,400,0),0,.01);for(int i=0;i<120;i++){double y=car.yaw;rr_car_step(&car,(RRInput){.steer=1},1./60);if(car.speed!=0||car.yaw>=y)failures++;}
 car.side=25;car.speed=0;if(rr_car_display_speed(&car)!=25)failures++;rr_car_place(&car,rr_v3(0,400,0),0,0);if(car.side!=0)failures++;
 printf("v0.2.13:2400 actual-source flight/pod states,16384 capture/key/controller states; max error %.12g, %d failures; no GPU\n",worst,failures);return failures?1:0;
}
