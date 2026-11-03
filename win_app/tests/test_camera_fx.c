#include "rr_camera_fx.h"
#include "rr_camera_motion.h"
#include "rr_chase_camera.h"
#include <stdio.h>
typedef struct FXCase {int reset,restart,mode,was,boosting,event;double dt,time_ms,race_time,speed,nitro,car_yaw,car_pitch,roll;RRCameraFX initial;RRCameraMotion initial_motion;double initial_fov;double expected[18];} FXCase;
#include "camera_fx_fixture.h"
int main(void){RRCameraFX fx={0};RRCameraMotion motion={0};double fov=0,worst=0;int failures=0;
    for(size_t i=0;i<sizeof(fx_cases)/sizeof(fx_cases[0]);i++){
        const FXCase *c=&fx_cases[i];if(c->reset){fx=c->initial;motion=c->initial_motion;fov=c->initial_fov;}if(c->restart)rr_camera_fx_restart(&fx);
        bool event=rr_camera_fx_ignite(&fx,c->was!=0,c->boosting!=0,c->race_time);
        RRCar car={0};car.pos=rr_v3(3,100,4);car.speed=c->speed;car.nitro=c->nitro;car.roll=c->roll;car.yaw=c->car_yaw;car.pitch=c->car_pitch;
        car.fwd=rr_v3(-sin(car.yaw)*cos(car.pitch),sin(car.pitch),-cos(car.yaw)*cos(car.pitch));
        rr_camera_motion_step(&motion,car.yaw,car.pitch,false,c->dt);double orbit=rr_camera_orbit(motion.look_yaw,motion.look_pitch);
        RRFirstPerson p=rr_first_person_active(c->mode,true,motion.look_yaw,motion.look_pitch)?rr_first_person_pose(&car,orbit):rr_chase_pose(&car,c->mode,true,motion.yaw,motion.pitch,motion.look_yaw,motion.look_pitch);
        double sf=fmin(1.4,fmax(0,car.speed)/RR_CAR_MAX);fov+=(68+15*fmin(1,sf)+9*car.nitro-fov)*(1-exp(-4*c->dt));
        RRCameraPulse pulse=rr_camera_fx_step(&fx,c->nitro,c->dt,c->time_ms);p=rr_camera_fx_apply(p,pulse);
        double got[]={fx.last_nitro,fx.kick,fx.shake,fov,fov+pulse.fov_add,motion.yaw,motion.pitch,motion.look_yaw,motion.look_pitch,p.position.x,p.position.y,p.position.z,p.forward.x,p.forward.y,p.forward.z,p.up.x,p.up.y,p.up.z};
        if(event!=(c->event!=0)){fprintf(stderr,"FX event row %zu\n",i);failures++;}
        for(int j=0;j<18;j++){double e=fabs(got[j]-c->expected[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-10){if(failures<8)fprintf(stderr,"FX row %zu component %d error %.17g\n",i,j,e);failures++;}}
    }
    printf("Actual browser Nitro/camera FX: %zu sequential source frames, max error %.17g, %d failures\n",sizeof(fx_cases)/sizeof(fx_cases[0]),worst,failures);return failures?1:0;
}
