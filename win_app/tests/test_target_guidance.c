#include "rr_target_guidance.h"
#include <stdio.h>
typedef struct TargetCase {double nx,ny,nz,w,h,z,d;int on;double x,y,a,rounded;} TargetCase;
#include "target_guidance_fixture.h"
int main(void){int failures=0;double worst=0;
for(size_t i=0;i<sizeof(target_cases)/sizeof(target_cases[0]);i++){
const TargetCase *c=&target_cases[i];RRTargetGuidance t=rr_target_guidance(c->nx,c->ny,c->nz,c->w,c->h,c->z,c->d);
double got[]={t.x,t.y,t.angle,t.distance},want[]={c->x,c->y,c->a,c->rounded};
if(t.on!=(c->on!=0))failures++;
for(int j=0;j<4;j++){double e=fabs(got[j]-want[j]);if(e>worst)worst=e;if(!isfinite(got[j])||e>1e-9)failures++;}
}
printf("Actual browser target guidance: %zu cases, worst %.17g, failures %d\n",sizeof(target_cases)/sizeof(target_cases[0]),worst,failures);return failures?1:0;}
