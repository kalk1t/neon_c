#ifndef RR_COCKPIT_PLAN_H
#define RR_COCKPIT_PLAN_H
#include "rr_ui_layout.h"
#include "rr_session.h"
#include "rr_view_controls.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
typedef struct RRCKBox { double x,y,w,h; } RRCKBox;
typedef struct RRCKLayout { RRCKBox control[2],panel[4];double degrees,pivot_x,pivot_y,font[4];int controls;bool race_column; } RRCKLayout;
typedef struct RRCKFit { double x,y,w,h,font; } RRCKFit;
typedef struct RRCKReadout {char speed[16],boost[16],time[32],lap[32],heat[32];const char *charge;double angle;bool hot;} RRCKReadout;
/* v3 measured DOM styles; return packed RGBA for renderer-independent tests. */
static inline uint32_t rr_cockpit_boost_rgba(const RRCKReadout *v)
{return strcmp(v->charge,"empty")==0?UINT32_C(0xc4ccd7ff):UINT32_C(0xf6f9ffff);}
static inline uint32_t rr_cockpit_heat_rgba(const RRCKReadout *v)
{return v->hot?UINT32_C(0xffb15aff):UINT32_C(0xff5adeff);}
static inline RRCKLayout rr_cockpit_layout(int id)
{
    if(id==1)return (RRCKLayout){{{.18,.59,.104,.26},{.716,.59,.108,.26}},{{.416,.463,.168,.085},{.654,.49,.102,.069},{.65,.563,.12,.027},{.263,.506,.1,.065}},8,.5,.9,{1.05,1.1,1.05,1.4},2,true};
    if(id==2)return (RRCKLayout){{{.215,.645,.57,.45},{0}},{{.435,.537,.13,.086},{.617,.536,.08,.095},{.768,.628,.104,.103},{.43,.492,.14,.03}},16,.5,.39,{1.1,1.2,1.1,1.15},1,false};
    return (RRCKLayout){{{.25,.61,.5,.486},{0}},{{.438,.517,.124,.079},{.264,.544,.073,.09},{.67,.54,.073,.1},{.432,.611,.14,.029}},16,.5,.58,{1,1.2,1.1,1.15},1,false};
}
static inline RRCKFit rr_cockpit_fit(double W,double H)
{
    double ratio=1672.0/941.0,w=fmax(W,H*ratio),h=w/ratio;return (RRCKFit){(W-w)*.5,H-.8*h,w,h,fmax(7,w/100)};
}
static inline RRUIRect rr_cockpit_box(RRCKFit fit,RRCKBox b)
{return (RRUIRect){(float)(fit.x+b.x*fit.w),(float)(fit.y+b.y*fit.h),(float)(b.w*fit.w),(float)(b.h*fit.h)};}
static inline void rr_cockpit_readout(RRCKReadout *out,int id,double speed,double boost,double time,double heat,bool hot,int lap,int laps,double steer)
{
    int kmh=(int)floor(rr_clamp(speed,0,1e6)+.5),percent=(int)floor(rr_clamp(boost,0,1)*100+.5);
    snprintf(out->speed,sizeof(out->speed),"%d",kmh);snprintf(out->boost,sizeof(out->boost),"%d%%",percent);out->charge=percent==0?"empty":percent==100?"full":"partial";
    uint64_t ticks=(uint64_t)fmax(0,floor(rr_clamp(time,0,1e12)*100));snprintf(out->time,sizeof(out->time),"%02llu:%02llu.%02llu",(unsigned long long)(ticks/6000),(unsigned long long)(ticks/100%60),(unsigned long long)(ticks%100));
    if(laps==0)snprintf(out->lap,sizeof(out->lap),"FREE ROAM");else if(laps<0)snprintf(out->lap,sizeof(out->lap),"LAP %d",lap);else snprintf(out->lap,sizeof(out->lap),"LAP %d/%d",lap,laps);
    out->hot=hot;if(hot)snprintf(out->heat,sizeof(out->heat),"OVERHEAT");else if(heat>.6)snprintf(out->heat,sizeof(out->heat),"HEAT %d%%",(int)floor(heat*100+.5));else snprintf(out->heat,sizeof(out->heat),"READY");
    out->angle=floor(steer*50+.5)/50*rr_cockpit_layout(id).degrees;
}
static inline double rr_cockpit_steer(double previous,double target,double dt,bool playing)
{return previous+((playing?target:0)-previous)*fmin(1,fmax(0,dt)*8);}
static inline bool rr_cockpit_visible(bool requested,RRSessionPhase phase,double look_yaw,double look_pitch,bool ready)
{
    bool flight=rr_view_flight_phase(phase);
    double orbit=fmin(1,(fabs(atan2(sin(look_yaw),cos(look_yaw)))+fabs(look_pitch))*4);return requested&&ready&&flight&&orbit<.05;
}
typedef struct RRCKSeat {RRVec3 position,forward;double extra_fov;} RRCKSeat;
static inline RRCKSeat rr_cockpit_seat(const RRCar *car)
{
    RRVec3 f=rr_normalize(car->fwd),right=rr_normalize(rr_v3(-f.z,0,f.x));
    RRVec3 up=rr_v3(-right.z*f.y,right.z*f.x-right.x*f.z,right.x*f.y);
    RRCKSeat seat={rr_add(rr_add(car->pos,rr_scale(f,.6)),rr_v3(0,1.5,0)),rr_add(rr_scale(f,cos(.2)),rr_scale(up,-sin(.2))),8};return seat;
}
#endif
