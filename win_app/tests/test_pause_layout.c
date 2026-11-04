#include "rr_race_ui.h"
#include "rr_pause_style.h"
#include <stdio.h>
typedef struct PauseCase {double w,h,x,y,width,height;RRUIRect actions[3];} PauseCase;
typedef struct GradientCase {double u,alpha;uint32_t rgba;} GradientCase;
#include "pause_layout_fixture.h"
static int failures;
int main(void){double worst=0;
for(size_t i=0;i<sizeof(pause_cases)/sizeof(pause_cases[0]);i++){
 const PauseCase *c=&pause_cases[i];RRUIOverlayLayout l=rr_ui_pause_layout((float)c->w,(float)c->h);double e=fabs(l.panel.x-c->x);if(e>worst)worst=e;if(e>1.0/64||fabs(l.panel.y-c->y)>.001||l.panel.w!=c->width||l.panel.h!=c->height)failures++;
 for(int b=0;b<3;b++){RRUIRect r=l.actions[b],v=c->actions[b];if(fabs(r.x-l.panel.x-v.x)>.001||fabs(r.y-l.panel.y-v.y)>.001||r.w!=v.w||r.h!=v.h)failures++;if(!rr_ui_hit(r,r.x+24,r.y+30,14)||rr_ui_hit(r,r.x,r.y,14))failures++;}
}
double previous=1;for(size_t i=0;i<sizeof(gradient_cases)/sizeof(gradient_cases[0]);i++){
 const GradientCase *c=&gradient_cases[i];double alpha=rr_pause_backdrop_alpha(c->u);if(!isfinite(alpha)||fabs(alpha-c->alpha)>1e-14||rr_pause_backdrop_rgba(c->u)!=c->rgba||alpha>previous+1e-14)failures++;previous=alpha;
}
if(rr_pause_panel_rgba()!=UINT32_C(0x04081eb8)||rr_pause_backdrop_alpha(NAN)!=0)failures++;
/* Desktop bounds/containment and native overload notice clearance. */
const float sizes[][2]={{960,720},{1180,900},{1181,721},{1280,720},{1600,1000},{1920,1080},{3840,2160}};
for(size_t n=0;n<sizeof(sizes)/sizeof(sizes[0]);n++){
 RRUIOverlayLayout l=rr_ui_pause_layout(sizes[n][0],sizes[n][1]);if(l.panel.x<0||l.panel.y<30||l.panel.x+l.panel.w>sizes[n][0]||l.panel.y+l.panel.h>sizes[n][1])failures++;
 for(int i=0;i<3;i++){RRUIRect b=l.actions[i];if(b.x<l.panel.x||b.x+b.w>l.panel.x+l.panel.w||b.y<l.panel.y+70||b.y+b.h>l.panel.y+309)failures++;}
}
printf("Pause layout: %zu verified Chrome card states, %zu CSS gradient samples, seven native viewport/control checks; worst left-edge error%.17g px, %d failures\n",sizeof(pause_cases)/sizeof(pause_cases[0]),sizeof(gradient_cases)/sizeof(gradient_cases[0]),worst,failures);return failures?1:0;}
