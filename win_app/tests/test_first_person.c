#include "rr_first_person.h"
#include <stdio.h>
typedef struct SeatCase {int mode;bool alive;double look_yaw,look_pitch,roll;RRVec3 fwd;bool active;double expected[9];} SeatCase;
#include "first_person_fixture.h"
int main(void){int fails=0;double worst=0;int pose_count=0;
    for(int i=0;i<3;i++){int m=i;for(int j=0;j<3;j++)m=rr_camera_next(m);if(m!=i)fails++;}
    for(size_t i=0;i<sizeof(seat_cases)/sizeof(seat_cases[0]);i++){
        const SeatCase *c=&seat_cases[i];bool active=rr_first_person_active(c->mode,c->alive,c->look_yaw,c->look_pitch);
        if(active!=c->active){fprintf(stderr,"visibility row %zu\n",i);fails++;}
        if(!active)continue;
        RRCar car={0};car.pos=rr_v3(3,100,4);car.fwd=c->fwd;car.roll=c->roll;
        RRFirstPerson p=rr_first_person_pose(&car,rr_camera_orbit(c->look_yaw,c->look_pitch));
        double a[]={p.position.x,p.position.y,p.position.z,p.forward.x,p.forward.y,p.forward.z,p.up.x,p.up.y,p.up.z};
        for(int j=0;j<9;j++){double e=fabs(a[j]-c->expected[j]);if(e>worst)worst=e;if(!isfinite(a[j])||e>1e-10){fprintf(stderr,"pose row %zu value %d\n",i,j);fails++;}}pose_count++;
    }
    printf("Actual source chaseCamera: %zu visibility cases, %d raw seat poses, max error %.17g, %d failures\n",sizeof(seat_cases)/sizeof(seat_cases[0]),pose_count,worst,fails);return fails?1:0;
}
