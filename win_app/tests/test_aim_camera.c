#include "rr_aim_camera.h"
#include "rr_camera_motion.h"
#include "rr_chase_camera.h"
#include <stdio.h>
typedef struct AimCameraCase{int reset,phase,requested,autopilot,mode,alive,looking;double dt,cy,cp,ay,ap,speed,roll;RRVec3 position;RRCameraMotion before,expected;double pose[9];}AimCameraCase;
#include "aim_camera_fixture.h"
int main(void){int failures=0;double worst=0;RRCameraMotion m={0};
 for(size_t i=0;i<sizeof(camera_cases)/sizeof(camera_cases[0]);i++){
  const AimCameraCase *c=&camera_cases[i];if(c->reset)m=c->before;bool on=rr_aim_camera_active(c->requested!=0,(RRSessionPhase)c->phase,c->autopilot!=0);
  rr_camera_motion_aim_step(&m,on?c->ay:c->cy,on?c->ap:c->cp,on,c->looking!=0,c->dt);
  RRCar car={0};car.pos=c->position;car.speed=c->speed;car.roll=c->roll;car.fwd=rr_v3(-sin(c->cy)*cos(c->cp),sin(c->cp),-cos(c->cy)*cos(c->cp));
  double orbit=rr_camera_orbit(m.look_yaw,m.look_pitch);RRFirstPerson p=rr_first_person_active(c->mode,c->alive!=0,m.look_yaw,m.look_pitch)?rr_first_person_pose(&car,orbit):rr_chase_pose_aim(&car,c->mode,c->alive!=0,m.yaw,m.pitch,m.look_yaw,m.look_pitch,on);
  double got[]={m.yaw,m.pitch,m.look_yaw,m.look_pitch,p.position.x,p.position.y,p.position.z,p.forward.x,p.forward.y,p.forward.z,p.up.x,p.up.y,p.up.z},want[]={c->expected.yaw,c->expected.pitch,c->expected.look_yaw,c->expected.look_pitch,c->pose[0],c->pose[1],c->pose[2],c->pose[3],c->pose[4],c->pose[5],c->pose[6],c->pose[7],c->pose[8]};
  for(int j=0;j<13;j++){double e=fabs(got[j]-want[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-10){if(failures<8)fprintf(stderr,"aim camera row%zu col%d error%.17g\n",i,j,e);failures++;}}
 }
 for(int phase=0;phase<=6;phase++)for(int r=0;r<2;r++)for(int a=0;a<2;a++)if(rr_aim_camera_active(r!=0,(RRSessionPhase)phase,a!=0)!=(r&&phase==RR_SESSION_ACTIVE&&!a))failures++;
 printf("Aim camera:2160 actual chaseCamera sequential states/poses,28 phase/request/autopilot policies; max error %.12g, %d failures; no GPU\n",worst,failures);return failures?1:0;
}
