#ifndef RR_SHOWCASE_H
#define RR_SHOWCASE_H
#include "rr_math.h"
#include <stdbool.h>
/* Pause subset of browser camera.js showcase. Menu swing/shop remain separate. */
typedef struct RRShowcase {double yaw,pitch,dist,x,y;bool dragging;} RRShowcase;
typedef struct RRShowcasePose {RRVec3 position,target;double fov;} RRShowcasePose;
static inline void rr_showcase_init(RRShowcase *s)
{*s=(RRShowcase){.6,.14,10.5,0,0,false};}
static inline void rr_showcase_down(RRShowcase *s,double x,double y,bool button_or_link)
{if(!button_or_link){s->x=x;s->y=y;s->dragging=true;}}
static inline void rr_showcase_move(RRShowcase *s,double x,double y)
{if(s->dragging){s->yaw-=(x-s->x)*.008;s->pitch=rr_clamp(s->pitch+(y-s->y)*.005,-.15,1.2);s->x=x;s->y=y;}}
static inline void rr_showcase_up(RRShowcase *s){s->dragging=false;}
static inline void rr_showcase_wheel(RRShowcase *s,double delta_y)
{s->dist=rr_clamp(s->dist*(1+(delta_y>0?.1:delta_y<0?-.1:0)),6,22);}
static inline RRShowcasePose rr_showcase_pause_step(RRShowcase *s,RRVec3 car,double car_yaw,RRVec3 camera,double fov,double dt)
{
    if(!s->dragging)s->yaw+=dt*.12;
    RRVec3 target=rr_add(car,rr_v3(0,-.1,0));double a=car_yaw+s->yaw,cp=cos(s->pitch);
    RRVec3 want=rr_add(rr_scale(rr_v3(-sin(a)*cp,sin(s->pitch),-cos(a)*cp),s->dist),target);
    double k=1-exp(-5*dt);camera=rr_mix(camera,want,k);
    target=rr_add(target,rr_scale(rr_v3(cos(a),0,-sin(a)),s->dist*.12));
    return (RRShowcasePose){camera,target,fov+(34-fov)*k};
}
#endif
