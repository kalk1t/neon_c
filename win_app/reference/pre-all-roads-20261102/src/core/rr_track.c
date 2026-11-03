/* Centripetal Catmull-Rom evaluation follows Three.js (MIT); see licenses. */
#include <stdbool.h>
#include "rr_track.h"
#include "skyline_data.h"

static double cubic(double x0,double x1,double x2,double x3,double d0,double d1,double d2,double t)
{
    double m1=((x1-x0)/d0-(x2-x0)/(d0+d1)+(x2-x1)/d1)*d1;
    double m2=((x2-x1)/d1-(x3-x1)/(d1+d2)+(x3-x2)/d2)*d1;
    double c2=-3*x1+3*x2-2*m1-m2;
    double c3=2*x1-2*x2+m1+m2;
    return x1+m1*t+c2*t*t+c3*t*t*t;
}

RRVec3 rr_track_point(double t)
{
    t=rr_clamp(t,0,1);
    double p=RR_SKYLINE_CONTROL_COUNT*t;
    int i=(int)floor(p);
    double w=p-i;
    RRVec3 p0=rr_skyline_control[(i+RR_SKYLINE_CONTROL_COUNT-1)%RR_SKYLINE_CONTROL_COUNT];
    RRVec3 p1=rr_skyline_control[i%RR_SKYLINE_CONTROL_COUNT];
    RRVec3 p2=rr_skyline_control[(i+1)%RR_SKYLINE_CONTROL_COUNT];
    RRVec3 p3=rr_skyline_control[(i+2)%RR_SKYLINE_CONTROL_COUNT];
    RRVec3 a=rr_sub(p0,p1),b=rr_sub(p1,p2),c=rr_sub(p2,p3);
    double d0=pow(rr_dot(a,a),0.25),d1=pow(rr_dot(b,b),0.25),d2=pow(rr_dot(c,c),0.25);
    if(d1<1e-4) d1=1;
    if(d0<1e-4) d0=d1;
    if(d2<1e-4) d2=d1;
    return rr_v3(cubic(p0.x,p1.x,p2.x,p3.x,d0,d1,d2,w),
                 cubic(p0.y,p1.y,p2.y,p3.y,d0,d1,d2,w),
                 cubic(p0.z,p1.z,p2.z,p3.z,d0,d1,d2,w));
}

RRVec3 rr_track_tangent(double t)
{
    RRVec3 a=rr_track_point(fmax(0,t-0.0001)),b=rr_track_point(fmin(1,t+0.0001));
    return rr_normalize(rr_sub(b,a));
}

static double parameter_at(const RRTrack *track,double u)
{
    double distance=rr_clamp(u,0,1)*track->length;
    int lo=0,hi=RR_TRACK_ARC_DIVISIONS;
    while(lo<=hi){
        int mid=(lo+hi)/2;
        if(track->arcs[mid]<distance) lo=mid+1;
        else if(track->arcs[mid]>distance) hi=mid-1;
        else return (double)mid/RR_TRACK_ARC_DIVISIONS;
    }
    int i=hi;
    if(i<0) return 0;
    if(i>=RR_TRACK_ARC_DIVISIONS) return 1;
    double fraction=(distance-track->arcs[i])/(track->arcs[i+1]-track->arcs[i]);
    return (i+fraction)/RR_TRACK_ARC_DIVISIONS;
}

RRVec3 rr_track_point_at(const RRTrack *track,double u){return rr_track_point(parameter_at(track,u));}
RRVec3 rr_track_tangent_at(const RRTrack *track,double u){return rr_track_tangent(parameter_at(track,u));}

bool rr_track_init(RRTrack *track)
{
    track->arcs[0]=0;
    RRVec3 last=rr_track_point(0);
    for(int i=1;i<=RR_TRACK_ARC_DIVISIONS;i++){
        RRVec3 p=rr_track_point((double)i/RR_TRACK_ARC_DIVISIONS);
        track->arcs[i]=track->arcs[i-1]+rr_length(rr_sub(p,last)); last=p;
    }
    track->length=track->arcs[RR_TRACK_ARC_DIVISIONS];
    track->sample_count=(int)ceil(track->length/4);
    if(track->sample_count<=0 || track->sample_count>RR_TRACK_MAX_SAMPLES) return false;
    for(int i=0;i<track->sample_count;i++){
        double u=(double)i/track->sample_count;
        track->samples[i]=rr_track_point_at(track,u);
        track->tangents[i]=rr_track_tangent_at(track,u);
    }
    for(int i=0;i<RR_TRACK_GATES;i++){
        double t=rr_skyline_checkpoints[i]/RR_SKYLINE_CONTROL_COUNT;
        RRVec3 tangent=rr_track_tangent(t);
        track->gates[i].pos=rr_track_point(t);
        track->gates[i].yaw=atan2(-tangent.x,-tangent.z);
        track->gates[i].forward=rr_v3(-sin(track->gates[i].yaw),0,-cos(track->gates[i].yaw));
    }
    return true;
}

int rr_track_nearest(const RRTrack *track, RRVec3 pos)
{
    int best=0; double distance=HUGE_VAL;
    for(int i=0;i<track->sample_count;i++){
        RRVec3 delta=rr_sub(track->samples[i],pos); double d=rr_dot(delta,delta);
        if(d<distance){distance=d;best=i;}
    }
    return best;
}

int rr_track_nearest_local(const RRTrack *track,RRVec3 pos,int hint)
{
    int best=hint;double distance=HUGE_VAL;
    for(int k=-30;k<=30;k++){
        int i=(hint+k+track->sample_count)%track->sample_count;
        RRVec3 delta=rr_sub(track->samples[i],pos);double d=rr_dot(delta,delta);
        if(d<distance){distance=d;best=i;}
    }
    return best;
}

bool rr_gate_crossed(const RRGate *gate,RRVec3 previous,RRVec3 current,double radius)
{
    double a=rr_dot(rr_sub(previous,gate->pos),gate->forward);
    double b=rr_dot(rr_sub(current,gate->pos),gate->forward);
    if(a>=0 || b<0 || b==a)return false;
    RRVec3 crossing=rr_mix(previous,current,-a/(b-a));
    return rr_length(rr_sub(crossing,gate->pos))<=radius;
}
