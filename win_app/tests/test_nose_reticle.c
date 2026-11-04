#include "rr_reticle.h"
#include "rr_math.h"
#include <stdio.h>
typedef struct NoseCase{RRVec3 eye,target,up,cross,aim;double fov,zoom;int captured,alive,cross_shown,aim_shown,nose;double cross_xy[2],aim_xy[2];}NoseCase;
#include "nose_reticle_fixture.h"
static Vector3 v(RRVec3 p){return (Vector3){(float)p.x,(float)p.y,(float)p.z};}
int main(void){int failures=0;double worst=0;
 for(size_t i=0;i<sizeof(nose_cases)/sizeof(nose_cases[0]);i++){
  const NoseCase *c=&nose_cases[i];Camera3D camera={v(c->eye),v(c->target),v(c->up),(float)c->fov,CAMERA_PERSPECTIVE};RRHUDPoint cross=rr_reticle_project(v(c->cross),camera,1280,720,.3,22000),aim=rr_reticle_project(v(c->aim),camera,1280,720,.3,22000);
  bool visible=rr_reticle_aim_visible(aim.shown,c->captured!=0,c->alive!=0);if(cross.shown!=(c->cross_shown!=0)||visible!=(c->aim_shown!=0)||visible!=(c->nose!=0))failures++;
  RRHeadingReticle h=rr_reticle_heading(visible,c->zoom);if(h.ticks==visible||fabs(h.outer-(visible?4:22)*c->zoom)>1e-12||fabs(h.inner-(visible?2:20)*c->zoom)>1e-12||fabs(h.opacity-(visible?.8:.5))>1e-12)failures++;
  const RRHUDPoint points[]={cross,aim};const double *expected[]={c->cross_xy,c->aim_xy};for(int j=0;j<2;j++)if(points[j].shown){double e=fmax(fabs(points[j].position.x-expected[j][0]),fabs(points[j].position.y-expected[j][1]));if(e>worst)worst=e;if(e>.05){if(failures<8)fprintf(stderr,"nose projection row%zu point%d error%.12g\n",i,j,e);failures++;}}
 }
 printf("One-ring reticles:372 actual HUD projection/visibility/class states,source8px hollow nose/2px border/ticks-hidden; max float pixel error %.12g, %d failures; no GPU\n",worst,failures);return failures?1:0;
}
