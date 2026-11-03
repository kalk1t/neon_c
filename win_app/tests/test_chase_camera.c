#include "rr_chase_camera.h"
#include <stdio.h>
typedef struct ChaseCase {int mode;bool alive;double speed,roll,yaw,pitch,look_yaw,look_pitch;RRVec3 car_fwd;double expected[9];} ChaseCase;
#include "chase_camera_fixture.h"
int main(void){int failures=0;double worst=0;
    for(size_t i=0;i<sizeof(chase_cases)/sizeof(chase_cases[0]);i++){
        const ChaseCase *c=&chase_cases[i];RRCar car={0};car.pos=rr_v3(3,100,4);car.fwd=c->car_fwd;car.speed=c->speed;car.roll=c->roll;
        RRFirstPerson p=rr_chase_pose(&car,c->mode,c->alive,c->yaw,c->pitch,c->look_yaw,c->look_pitch);
        double got[]={p.position.x,p.position.y,p.position.z,p.forward.x,p.forward.y,p.forward.z,p.up.x,p.up.y,p.up.z};
        for(int j=0;j<9;j++){double e=fabs(got[j]-c->expected[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-10){if(failures<8)fprintf(stderr,"chase row %zu component %d error %.17g\n",i,j,e);failures++;}}
    }
    printf("Actual v0.2.6 chaseCamera: %zu geometry/bank/speed/degenerate cases, max error %.17g, %d failures\n",sizeof(chase_cases)/sizeof(chase_cases[0]),worst,failures);return failures?1:0;
}
