#include "rr_reticle.h"
#include "rlgl.h"
#include "rr_math.h"
#include <stdio.h>
typedef struct ProjectionCase {RRVec3 eye,target,up,point;double fov,near_plane,far_plane;int width,height,display_width,display_height,shown;double expected[2];} ProjectionCase;
typedef struct AimVisibleCase {int projected,captured,alive,shown;} AimVisibleCase;
typedef struct ReticleBoxCase {int aim;double width,height,border;} ReticleBoxCase;
#include "reticle_fixture.h"
#include "reticle_box_fixture.h"
static Vector3 v(RRVec3 p){return (Vector3){(float)p.x,(float)p.y,(float)p.z};}
int main(void){int failures=0;double worst=0,raylib_error=0;
for(size_t i=0;i<sizeof(projection_cases)/sizeof(projection_cases[0]);i++){
const ProjectionCase *c=&projection_cases[i];Camera3D camera={v(c->eye),v(c->target),v(c->up),(float)c->fov,CAMERA_PERSPECTIVE};
RRHUDPoint p=rr_reticle_project(v(c->point),camera,c->width,c->height,c->near_plane,c->far_plane);
if(p.shown!=(c->shown!=0))failures++;
if(c->shown){rlSetClipPlanes(c->near_plane,c->far_plane);Vector2 actual=GetWorldToScreenEx(v(c->point),camera,c->width,c->height);double e=fmax(fabs(p.position.x-actual.x),fabs(p.position.y-actual.y));if(e>raylib_error)raylib_error=e;if(e>.001)failures++;
p.position.x*=(float)c->display_width/c->width;p.position.y*=(float)c->display_height/c->height;
double ex=fabs(p.position.x-c->expected[0]),ey=fabs(p.position.y-c->expected[1]);e=fmax(ex,ey);if(e>worst)worst=e;if(e>.02){if(failures<6)fprintf(stderr,"Reticle projection row%zu error%.17g\n",i,e);failures++;}}
}
for(size_t i=0;i<sizeof(aim_visible_cases)/sizeof(aim_visible_cases[0]);i++){
const AimVisibleCase *c=&aim_visible_cases[i];if(rr_reticle_aim_visible(c->projected!=0,c->captured!=0,c->alive!=0)!=(c->shown!=0))failures++;
}
for(int i=0;i<3;i++)for(size_t j=0;j<sizeof(reticle_boxes)/sizeof(reticle_boxes[0]);j++){
double z=(double[]){.55,.8,1}[i];const ReticleBoxCase *c=&reticle_boxes[j];RRReticleShape s=rr_reticle_shape(z);double outer=c->aim?s.aim_outer:s.cross_outer,inner=c->aim?s.aim_inner:s.cross_inner;
if(fabs(2*outer-c->width*z)>1e-12||fabs(2*outer-c->height*z)>1e-12||fabs(outer-inner-c->border*z)>1e-12)failures++;}
Camera3D camera={{0,0,0},{0,0,-1},{0,1,0},68,CAMERA_PERSPECTIVE};if(rr_reticle_project((Vector3){0},camera,960,720,.1,2500).shown)failures++;
printf("Actual HUD.screen/Three reticles: %zu projection cases/%zu source visibility states; worst%.17g pixels, raylib%.17g, %d failures; no window/GPU\n",sizeof(projection_cases)/sizeof(projection_cases[0]),sizeof(aim_visible_cases)/sizeof(aim_visible_cases[0]),worst,raylib_error,failures);return failures?1:0;}
