#include "rr_showcase.h"
#include <stdio.h>
typedef struct ShowcaseCase {int sequence,frame;double dt;int event;double x,y,wheel,cx,cy,cz,car_yaw,yaw,pitch,dist;int dragging;double px,py,pz,tx,ty,tz,fov;} ShowcaseCase;
#include "showcase_fixture.h"
static int failures;static double worst;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"showcase line%d\n",__LINE__);failures++;}}while(0)
static void compare(double a,double b){double error=fabs(a-b);if(error>worst)worst=error;CHECK(error<3e-10);}
int main(void)
{
    int sequence=-1;RRShowcase s={0};RRShowcasePose pose={0};
    for(size_t i=0;i<sizeof(showcase_cases)/sizeof(showcase_cases[0]);i++){
        const ShowcaseCase *c=&showcase_cases[i];
        if(c->sequence!=sequence){sequence=c->sequence;rr_showcase_init(&s);pose.position=rr_v3(20+sequence,120-sequence,-30);pose.fov=68+sequence;}
        if(c->event==1||c->event==5)rr_showcase_down(&s,c->x,c->y,c->event==5);
        else if(c->event==2)rr_showcase_move(&s,c->x,c->y);else if(c->event==3)rr_showcase_up(&s);else if(c->event==4)rr_showcase_wheel(&s,c->wheel);
        pose=rr_showcase_pause_step(&s,rr_v3(c->cx,c->cy,c->cz),c->car_yaw,pose.position,pose.fov,c->dt);
        compare(s.yaw,c->yaw);compare(s.pitch,c->pitch);compare(s.dist,c->dist);CHECK(s.dragging==(c->dragging!=0));
        compare(pose.position.x,c->px);compare(pose.position.y,c->py);compare(pose.position.z,c->pz);compare(pose.target.x,c->tx);compare(pose.target.y,c->ty);compare(pose.target.z,c->tz);compare(pose.fov,c->fov);
    }
    /* Native lifecycle safeguard: release cancels future moves across focus,
       minimize or leaving pause, without resetting orbit or zoom. */
    rr_showcase_init(&s);rr_showcase_down(&s,10,20,false);rr_showcase_up(&s);rr_showcase_move(&s,1000,-1000);CHECK(!s.dragging&&s.yaw==.6&&s.pitch==.14&&s.dist==10.5);
    rr_showcase_down(&s,10,20,true);CHECK(!s.dragging);
    printf("Showcase pause: %zu actual-source callback/camera states across12 sequences; worst error %.12g, %d failures; no GPU\n",sizeof(showcase_cases)/sizeof(showcase_cases[0]),worst,failures);return failures?1:0;
}
