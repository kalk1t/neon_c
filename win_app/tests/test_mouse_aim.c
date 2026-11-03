#include "rr_mouse_aim.h"
#include <stdio.h>
typedef struct AimCase {int reset,look,invert;double sensitivity,cy,cp,dx,dy,steer,climb;double initial[4],expected[9];} AimCase;
#include "mouse_aim_fixture.h"
int main(void){RRMouseAim a={0};int failures=0;double worst=0;
for(size_t i=0;i<sizeof(aim_cases)/sizeof(aim_cases[0]);i++){
const AimCase *c=&aim_cases[i];if(c->reset){a.yaw=c->initial[0];a.pitch=c->initial[1];a.look_yaw=c->initial[2];a.look_pitch=c->initial[3];}
RRInput in={0};in.steer=c->steer;in.climb=c->climb;
in=rr_mouse_aim_step(&a,(RRMouseSettings){c->sensitivity,c->invert!=0},c->cy,c->cp,in,c->dx,c->dy,c->look!=0);
double got[]={a.yaw,a.pitch,a.look_yaw,a.look_pitch,in.steer,in.climb,a.direction.x,a.direction.y,a.direction.z};
if(a.looking!=(c->look!=0))failures++;
for(int j=0;j<9;j++){double e=fabs(got[j]-c->expected[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-11){if(failures<5)fprintf(stderr,"Aim row %zu column %d\n",i,j);failures++;}}
}
for(int i=0;i<=44;i++)if(fabs(rr_mouse_sensitivity_at((double)i/44)-(.3+i*.05))>1e-12)failures++;
if(fabs(rr_mouse_sensitivity_at(-1)-.3)>1e-12||fabs(rr_mouse_sensitivity_at(2)-2.5)>1e-12)failures++;
printf("Actual browser updateAim: %zu states, worst %.17g, %d failures; 45 slider steps checked\n",sizeof(aim_cases)/sizeof(aim_cases[0]),worst,failures);return failures?1:0;}
