#include "rr_ai_current.h"
#include <limits.h>
bool rr_ai_current_prepare(RRAICorners *c,const RRTrack *t){
 if(!c)return false;c->track=NULL;c->count=0;
 if(!t||t->sample_count<3||t->sample_count>RR_TRACK_MAX_SAMPLES||!isfinite(t->length)||t->length<=0)return false;
 int n=t->sample_count;double step=t->length/n;
 for(int i=0;i<n;i++){
  RRVec3 a=t->tangents[(i-3+n)%n],b=t->tangents[(i+3)%n];double dot=rr_dot(a,b);
  if(!isfinite(dot))return false;
  double k=acos(rr_clamp(dot,-1,1))/(6*step);c->speed[i]=(float)(k>1e-6?(1.15*.8)/k:INFINITY);
 }
 c->track=t;c->count=n;c->length=t->length;return true;
}
double rr_ai_current_corner_limit(const RRAICorners *c,int sample,double speed){
 if(!c||!c->track||c->count<3||c->count>RR_TRACK_MAX_SAMPLES||c->track->sample_count!=c->count||c->track->length!=c->length||sample<0||sample>=c->count||!isfinite(speed))return NAN;
 double sp=fmax(0,speed),reach=sp*sp/(2*45)+40,step=c->length/c->count,limit=INFINITY;
 /* Repeated laps cannot lower min(sqrt(vc^2+90*d)): same vc, greater d.
  * One lap therefore exactly bounds work, even for a large supplied speed. */
 double d=0;for(int k=0;k<c->count&&d<reach;k++,d+=step){
  double vc=c->speed[(sample+k)%c->count];
  if(vc<INFINITY)limit=fmin(limit,sqrt(vc*vc+2*45*d));
 }
 return limit;
}
bool rr_ai_current_controls(RRAIDriver *r,const RRCar *car,const RRTrack *t,const RRAICorners *c,const RRAIPeer *peers,size_t count,RRVec3 target,double time,double dt,const RRVec3 *coin,RRInput *out){
 if(!r||!car||!t||!out||(count&&!peers)||count>RR_AI_RIVALS||r->sample<0||r->sample>=t->sample_count||!isfinite(car->speed)||!isfinite(time)||!isfinite(dt)||dt<0)return false;
 double limit=rr_ai_current_corner_limit(c,r->sample,car->speed);if(isnan(limit)||c->track!=t)return false;
 double look=14+floor(fmax(0,car->speed)/14+.5);if(look>INT_MAX-r->sample)return false;
 int i=(r->sample+(int)look)%t->sample_count;RRVec3 tangent=t->tangents[i],right=rr_normalize(rr_v3(-tangent.z,0,tangent.x));
 for(size_t j=0;j<count;j++){
  if(!peers[j].alive)continue;RRVec3 relative=rr_sub(peers[j].pos,car->pos);double ahead=rr_dot(relative,car->fwd);
  if(ahead>0&&ahead<32){double lateral=rr_dot(relative,right);if(fabs(lateral)<5.5)r->dodge=lateral>0?-7:7;}
 }
 r->dodge*=exp(-1.2*dt);double gate_distance=rr_length(rr_sub(car->pos,target)),lane_k=rr_clamp((gate_distance-60)/160,0,1);
 double offset=(r->lane+sin(time*.25+r->seed)*3+r->dodge)*lane_k;RRVec3 aim=rr_add(t->samples[i],rr_scale(right,offset));aim.y+=r->lane_y*lane_k;if(coin)aim=rr_mix(aim,*coin,.85);
 double near=rr_clamp(1-gate_distance/fmax(60,fmax(0,car->speed)*1.5),0,1);if(near>0)aim=rr_mix(aim,target,near);
 RRVec3 relative=rr_sub(aim,car->pos);double dy=atan2(-relative.x,-relative.z)-car->yaw;dy=atan2(sin(dy),cos(dy));
 double angle=atan2(relative.y,hypot(relative.x,relative.z)),sp=rr_length(car->vel),going=sp>1?asin(rr_clamp(car->vel.y/sp,-1,1)):car->pitch;
 double pitch=angle+(angle-going)*.8,over=car->speed-limit;
 *out=(RRInput){.throttle=over> -4?0:1,.brake=fabs(dy)>.55||over>6?1:0,.steer=rr_clamp(-dy*2.6,-1,1),.climb=rr_clamp((pitch/.62)*1.4,-1,1),.boost=over< -25&&fabs(dy)<.07&&car->boost>.35&&sin(time*.7+r->seed)>.2-r->skill*.6};return true;
}
double rr_ai_current_top(int id){return id>=1&&id<=9?120*pow(1.02,id-1):NAN;}
double rr_ai_current_speed_factor(double player,double rival,int samples,int id){
 if(samples<=0||!isfinite(player)||!isfinite(rival))return NAN;
 double top=rr_ai_current_top(id);if(isnan(top))return NAN;
 double gap=(player-rival)/(samples*.15);return (top/RR_CURRENT_CAR_MAX)*(1+rr_clamp(gap*.05,0,.07));
}
