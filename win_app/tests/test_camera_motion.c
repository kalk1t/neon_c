#include "rr_camera_motion.h"
#include "rr_chase_camera.h"
#include <stdio.h>
typedef struct MotionCase {int reset;bool looking;double dt,car_yaw,car_pitch;RRCameraMotion before,expected;double pose[9];} MotionCase;
#include "camera_motion_fixture.h"
int main(void){int failures=0;double worst=0;RRCameraMotion m={0};
    for(size_t i=0;i<sizeof(motion_cases)/sizeof(motion_cases[0]);i++){
        const MotionCase *c=&motion_cases[i];if(c->reset)m=c->before;
        rr_camera_motion_step(&m,c->car_yaw,c->car_pitch,c->looking,c->dt);
        RRCar car={0};car.pos=rr_v3(3,100,4);car.speed=95;car.roll=.4;car.fwd=rr_v3(-sin(c->car_yaw)*cos(c->car_pitch),sin(c->car_pitch),-cos(c->car_yaw)*cos(c->car_pitch));
        RRFirstPerson p=rr_chase_pose(&car,0,true,m.yaw,m.pitch,m.look_yaw,m.look_pitch);
        double got[]={m.yaw,m.pitch,m.look_yaw,m.look_pitch,p.position.x,p.position.y,p.position.z,p.forward.x,p.forward.y,p.forward.z,p.up.x,p.up.y,p.up.z};
        double want[]={c->expected.yaw,c->expected.pitch,c->expected.look_yaw,c->expected.look_pitch,c->pose[0],c->pose[1],c->pose[2],c->pose[3],c->pose[4],c->pose[5],c->pose[6],c->pose[7],c->pose[8]};
        for(int j=0;j<13;j++){double e=fabs(got[j]-want[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-10){if(failures<8)fprintf(stderr,"camera motion row %zu value %d error %.17g\n",i,j,e);failures++;}}
    }
    printf("Actual v0.2.6 chaseCamera timing: %zu sequential states and resulting poses, max error %.17g, %d failures\n",sizeof(motion_cases)/sizeof(motion_cases[0]),worst,failures);return failures?1:0;
}
