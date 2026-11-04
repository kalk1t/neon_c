#include "rr_ai_current.h"
#include <stdio.h>
#include <string.h>
#include "ai_current_reference.h"
static unsigned failures;static size_t current;static double max_control,max_state,max_corner,max_limit;static unsigned corner_differences;
#define CHECK(x) do{if(!(x)){if(failures<15)fprintf(stderr,"case%zu line%d: %s\n",current,__LINE__,#x);failures++;}}while(0)
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
static void near(double a,double b,double tolerance,double *maximum){if(isinf(a)||isinf(b)){CHECK(a==b);return;}double e=fabs(a-b);if(e>*maximum)*maximum=e;CHECK(isfinite(a)&&isfinite(b)&&e<=tolerance);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x,1e-5,&max_state);near(a.y,b.y,1e-5,&max_state);near(a.z,b.z,1e-5,&max_state);}
static void input(RRInput a,RRInput b){near(a.throttle,b.throttle,1e-8,&max_control);near(a.brake,b.brake,1e-8,&max_control);near(a.steer,b.steer,1e-8,&max_control);near(a.climb,b.climb,1e-8,&max_control);CHECK(a.boost==b.boost&&a.strafe==0&&!a.aim_steer);}
static void load(RRCar *c,const CarState *s){rr_car_reset(c,s->pos,s->yaw,s->speed);c->vel=s->vel;c->fwd=s->fwd;c->pitch=s->pitch;c->roll=s->roll;c->boost=s->boost;c->t=s->t;c->stun=s->stun;c->roll_time=s->roll_time;c->surge=s->surge;c->max_multiplier=s->mul;c->thrust=s->thrust;c->nitro=s->nitro;c->kick=s->kick;c->sputter=s->sputter;c->boosting=s->boosting;}
static void car(const RRCar *c,const CarState *s){vector(c->pos,s->pos);vector(c->vel,s->vel);vector(c->fwd,s->fwd);double a[]={c->yaw,c->pitch,c->roll,c->speed,c->boost,c->t,c->stun,c->roll_time,c->surge,c->max_multiplier,c->thrust,c->nitro,c->kick,c->sputter},b[]={s->yaw,s->pitch,s->roll,s->speed,s->boost,s->t,s->stun,s->roll_time,s->surge,s->mul,s->thrust,s->nitro,s->kick,s->sputter};for(size_t i=0;i<COUNT(a);i++)near(a[i],b[i],1e-5,&max_state);CHECK(c->boosting==s->boosting);}
int main(void){static RRTrack track;static RRAICorners corners;RRCar c;int current_road=0;
 for(current=0;current<COUNT(routes);current++){
  const Route *r=routes+current;CHECK(rr_track_init_road(&track,r->road)&&rr_ai_current_prepare(&corners,&track));CHECK(corners.count==r->n);near(track.length,r->length,1e-7,&max_state);
  for(int k=0;k<r->n;k++){double expected=corner_reference[r->corner_offset+(size_t)k];near(corners.speed[k],expected,fmax(1e-5,fabs(expected)*2e-7),&max_corner);if(corners.speed[k]!=expected)corner_differences++;}
  near(rr_ai_current_top(r->road),r->top,1e-12,&max_control);
  for(int k=0;k<5;k++)near(rr_ai_current_speed_factor(r->factors[k][0],r->factors[k][1],r->n,r->road),r->factors[k][2],1e-12,&max_control);
  for(int k=0;k<2;k++)near(rr_ai_current_corner_limit(&corners,(int)r->extra_limits[k][0],r->extra_limits[k][1]),r->extra_limits[k][2],1e-4,&max_limit);
 }
 for(current=0;current<COUNT(probes);current++){
  const Probe *p=probes+current;if(p->road!=current_road){CHECK(rr_track_init_road(&track,p->road)&&rr_ai_current_prepare(&corners,&track));current_road=p->road;}load(&c,&p->car);RRAIDriver driver=p->driver;RRInput out;
  near(rr_ai_current_corner_limit(&corners,driver.sample,c.speed),p->limit,1e-4,&max_limit);CHECK(rr_ai_current_controls(&driver,&c,&track,&corners,p->peers,3,p->target,p->time,p->dt,p->has_coin?&p->coin:NULL,&out));input(out,p->expected);near(driver.dodge,p->dodge,1e-8,&max_control);
 }
 unsigned steps=0;
 for(size_t t=0;t<COUNT(traces);t++){
  const Trace *trace=traces+t;CHECK(rr_track_init_road(&track,trace->road)&&rr_ai_current_prepare(&corners,&track));load(&c,&trace->initial);RRAIDriver driver=trace->driver;
  for(size_t i=0;i<trace->count;i++){
   current=trace->offset+i;const Frame *f=frames+current;CHECK(rr_track_nearest(&track,c.pos)==f->sample);driver.sample=f->sample;RRInput out;
   near(rr_ai_current_corner_limit(&corners,driver.sample,c.speed),f->limit,1e-4,&max_limit);CHECK(rr_ai_current_controls(&driver,&c,&track,&corners,NULL,0,f->target,f->time,f->dt,f->has_coin?&f->coin:NULL,&out));input(out,f->input);near(driver.dodge,f->dodge,1e-8,&max_control);rr_car_step_current(&c,out,f->dt);car(&c,&f->car);steps++;
  }
 }
 // Explicit preparation lifecycle and invalid-cache gates; no hidden stale reuse.
 corners.track=NULL;CHECK(isnan(rr_ai_current_corner_limit(&corners,0,135)));CHECK(!rr_ai_current_prepare(&corners,NULL));CHECK(isnan(rr_ai_current_top(0))&&isnan(rr_ai_current_speed_factor(0,0,0,1)));
 CHECK(rr_track_init_road(&track,1)&&rr_ai_current_prepare(&corners,&track));track.length+=1;CHECK(isnan(rr_ai_current_corner_limit(&corners,0,135)));CHECK(rr_ai_current_prepare(&corners,&track));
 printf("AI_CURRENT_SUMMARY roads=%zu samples=%zu probes=%zu frames=%u factors=45 multilap_limits=18 corner_differences=%u max_corner_error=%.17g max_limit_error=%.17g max_control_error=%.17g max_state_error=%.17g cache_bytes=%zu failures=%u; offline,no live rivals/graphics/account\n",COUNT(routes),COUNT(corner_reference),COUNT(probes),steps,corner_differences,max_corner,max_limit,max_control,max_state,sizeof(corners),failures);return failures?1:0;
}
