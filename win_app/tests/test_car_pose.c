#include "rr_car_pose.h"
#include "rr_weapon_current.h"
#include <stdio.h>
#include "car_pose_reference.h"
static unsigned failures;static size_t current;static double max_error;
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"pose%zu line%d: %s\n",current,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>max_error)max_error=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-8);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
int main(void){RRCar car={0};RRCarPose pose={0};unsigned actions[6]={0};
 for(current=0;current<sizeof(pose_frames)/sizeof(*pose_frames);current++){
  const PoseFrame *f=pose_frames+current;if(f->first){rr_car_reset(&car,rr_v3(3,200,5),f->yaw,f->speed);CHECK(rr_car_pose_init(&pose,&car));}
  near(car.speed,f->before);actions[f->action]++;
  if(f->action==0){double before=car.speed;rr_car_step_current(&car,f->input,f->dt);CHECK(rr_car_pose_after_step(&pose,&car,before));}
  else if(f->action==1){rr_car_place(&car,rr_v3(3+f->index*.3,200+f->index*.2,5-f->index*.5),f->yaw+.1*f->index,35);CHECK(rr_car_pose_sync(&pose,&car));}
  else if(f->action==2){car.pos.x+=4;car.vel.z-=3;car.stun=.7;car.speed*=.8;}
  else if(f->action==3)rr_car_dodge_current(&car,f->index%2?-1:1);
  else if(f->action==4){car.t+=f->dt;CHECK(rr_car_pose_idle(&pose,car.t));}
  else{car.yaw+=.3;car.pitch=-.2;car.roll=.15;car.stun=.4;CHECK(rr_car_pose_sync(&pose,&car));}
  const State *s=&f->state;vector(car.pos,s->pos);vector(car.vel,s->vel);vector(car.fwd,s->fwd);double a[]={car.yaw,car.pitch,car.roll,car.speed,car.boost,car.t,car.stun,car.roll_time,car.surge,car.max_multiplier,car.thrust,car.nitro,car.kick,car.sputter},b[]={s->yaw,s->pitch,s->roll,s->speed,s->boost,s->t,s->stun,s->roll_time,s->surge,s->mul,s->thrust,s->nitro,s->kick,s->sputter};for(int i=0;i<14;i++)near(a[i],b[i]);CHECK(car.roll_direction==s->roll_direction&&car.boosting==s->boosting);
  double matrix[16];CHECK(rr_car_pose_matrix(&pose,matrix));for(int i=0;i<16;i++){near(pose.root[i],f->root[i]);near(matrix[i],f->matrix[i]);}near(pose.bob,f->bob);near(pose.lift,f->lift);vector(rr_weapon_current_muzzle(f->model,f->side,matrix),f->muzzle);RRFireBinding binding;CHECK(rr_car_pose_binding(&pose,f->model,0xff26d4,&binding));for(int i=0;i<16;i++)near(binding.model_world[i],f->matrix[i]);
 }
 RRCarPose empty={0};double matrix[16];CHECK(!rr_car_pose_matrix(&empty,matrix)&&!rr_car_pose_idle(&empty,0));CHECK(!rr_car_pose_binding(&pose,4,0,NULL));car.pos.x=NAN;CHECK(!rr_car_pose_sync(&pose,&car));CHECK(!rr_car_pose_after_step(&pose,&car,NAN));
 printf("CAR_POSE_SUMMARY frames=%zu actions=%u,%u,%u,%u,%u,%u pose_bytes=%zu max_error=%.17g failures=%u; no graphics/world\n",sizeof(pose_frames)/sizeof(*pose_frames),actions[0],actions[1],actions[2],actions[3],actions[4],actions[5],sizeof(pose),max_error,failures);return failures?1:0;
}
