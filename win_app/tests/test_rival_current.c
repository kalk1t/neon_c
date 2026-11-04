#include "rr_rival_current.h"
#include <stdio.h>
#include <string.h>
#include "rival_current_reference.h"
static unsigned failures;static size_t current;static double max_error;
#define CHECK(x) do{if(!(x)){if(failures<12)fprintf(stderr,"case%zu line%d: %s\n",current,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>max_error)max_error=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-10);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
typedef struct{uint32_t state;unsigned draws;} Random;
static double random_value(void *p){Random *r=p;r->state=r->state*UINT32_C(1664525)+UINT32_C(1013904223);r->draws++;return r->state/4294967296.0;}
typedef struct{Random *random;RRRivalCurrent *resources;unsigned calls;unsigned rng,draws;double heat,cd;int side;} Cue;
static void cue(void *p,const RRLocalRacer *r){Cue *c=p;(void)r;c->calls++;c->rng=c->random->state;c->draws=c->random->draws;c->heat=c->resources->heat;c->cd=c->resources->bolt_cd;c->side=c->resources->side;}
int main(void){
 RRLocalRacer actors[3]={0};RRLocalRacer *roster[3]={actors,actors+1,actors+2};RRShot shots[1];RRFireLocal l={0};RRRivalCurrent s={0};Random random={0};unsigned total_draws=0,total_shots=0,resets=0,guards=0;
 l.racers=roster;l.racer_count=3;l.shots=shots;l.capacity=1;l.random=random_value;l.random_ctx=&random;l.fx.three_warm_uuids=true;
 for(current=0;current<sizeof(rival_current_frames)/sizeof(*rival_current_frames);current++){
  const Fixture *f=rival_current_frames+current;if(f->first){random=(Random){f->seed,0};s=(RRRivalCurrent){0};}
  if(f->reset){rr_rival_current_reset(&s,random_value(&random)*.5);(void)random_value(&random);resets++;}
  if(f->forced_heat){s.heat=1;s.overheated=true;}
  near(s.heat,f->before.heat);near(s.bolt_cd,f->before.bolt_cd);CHECK(s.side==f->before.side&&s.overheated==f->before.overheated);
  RRLocalRacer *r=actors;r->car.pos=rr_v3(3,104,-8);r->car.fwd=rr_v3(0,0,-1);r->car.speed=f->speed;r->wreck=f->wreck;r->race.finished=f->finished;r->fire_cd=f->before.fire_cd;
  for(int i=0;i<2;i++){actors[i+1].car.pos=f->targets[i].pos;actors[i+1].car.vel=f->targets[i].vel;actors[i+1].wreck=f->targets[i].wreck;actors[i+1].race.finished=f->targets[i].finished;}
  RRFireBinding b={0};b.model=f->model;b.accent=f->accent;memcpy(b.model_world,f->matrix,sizeof(b.model_world));Cue event={&random,&s,0,0,0,0,0,0};bool fired=false;l.count=0;
  CHECK(rr_rival_current_step(&l,r,&s,f->dt,f->time,f->active,f->road,&b,cue,&event,&fired));
  CHECK(!l.error&&fired==f->fired&&l.count==(fired?1u:0u)&&event.calls==(fired?1u:0u));CHECK(random.state==f->rng&&random.draws==f->draws);
  near(r->fire_cd,f->after.fire_cd);near(s.heat,f->after.heat);near(s.bolt_cd,f->after.bolt_cd);CHECK(s.side==f->after.side&&s.overheated==f->after.overheated);
  if(fired){RRShot *q=shots;const Emitted *e=&f->shot;CHECK(q->owner==r&&q->target==NULL&&e->no_target&&e->bolt&&q->kind==RR_SHOT_BOLT&&q->has_damage&&q->visual.painted);vector(q->pos,e->pos);vector(q->vel,e->vel);near(q->speed,e->speed);near(q->life,e->life);near(q->turn,e->turn);near(q->damage,e->damage);near(q->radius,e->radius);near(q->visual.length,e->length);near(q->visual.width,e->width);near(q->visual.flown,0);vector(q->visual.color,e->color);CHECK(!q->trail&&!q->always&&l.fx.count==0);CHECK(event.rng==f->rng&&event.draws==f->draws&&event.side==s.side);near(event.heat,s.heat);near(event.cd,s.bolt_cd);total_shots++;}
  if(f->time<6||!f->active||f->wreck>0||f->finished)guards++;
  if(current+1==sizeof(rival_current_frames)/sizeof(*rival_current_frames)||rival_current_frames[current+1].first)total_draws+=random.draws;
 }
 // Invalid pose/storage errors are explicit, and guard skips freeze timers.
 RRFireBinding invalid={0};bool fired=true;unsigned before=random.draws;actors[0].wreck=0;actors[0].race.finished=false;CHECK(!rr_rival_current_step(&l,actors,&s,.01,7,true,1,&invalid,NULL,NULL,&fired)&&l.error&&!fired&&random.draws==before);l.error=false;
 RRRivalCurrent frozen=s;double fire_cd=actors[0].fire_cd;CHECK(rr_rival_current_step(&l,actors,&s,.5,5.9,true,1,NULL,NULL,NULL,&fired)&&!fired);CHECK(memcmp(&frozen,&s,sizeof(s))==0);near(actors[0].fire_cd,fire_cd);
 printf("RIVAL_CURRENT_SUMMARY frames=%zu models=4 shots=%u resets=%u guards=%u draws=%u state_bytes=%zu max_error=%.17g failures=%u; production inactive,no graphics/audio device/account\n",sizeof(rival_current_frames)/sizeof(*rival_current_frames),total_shots,resets,guards,total_draws,sizeof(s),max_error,failures);return failures?1:0;
}
