#include "rr_rival_arsenal.h"
#include <stdio.h>
#include <string.h>
#include "rival_arsenal_reference.h"
static unsigned failures;static size_t current;static double max_error;
#define CHECK(x) do{if(!(x)){if(failures<18)fprintf(stderr,"frame%zu line%d: %s\n",current,__LINE__,#x);failures++;}}while(0)
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
static void near(double a,double b){double e=fabs(a-b);if(e>max_error)max_error=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-8);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
typedef struct {uint32_t state;unsigned draws;} Random;
static double random_value(void *p){Random *r=p;r->state=r->state*UINT32_C(1664525)+UINT32_C(1013904223);r->draws++;return r->state/4294967296.0;}
static RRLocalRacer actors[4];static RRLocalRacer *roster[4]={actors,actors+1,actors+2,actors+3};static RRRivalLifeResources resources;static Random rng;static double shake;static const Frame *frame;static size_t event_at;static unsigned events[5];
static void load(RRLocalRacer *r,const Actor *f){r->car.pos=f->pos;r->car.vel=f->vel;r->car.fwd=f->fwd;r->car.speed=f->speed;r->car.stun=f->stun;r->car.roll_time=f->roll_time;r->car.roll_direction=f->roll_direction;r->car.surge=f->surge;r->car.yaw=f->yaw;r->car.pitch=f->pitch;r->car.roll=f->roll;r->hp=f->hp;r->wreck=f->wreck;r->fire_cd=f->fire_cd;r->race.finished=f->finished;r->is_player=f->player;r->visible=f->visible;}
static void actor(const RRLocalRacer *r,const Actor *f){vector(r->car.pos,f->pos);vector(r->car.vel,f->vel);vector(r->car.fwd,f->fwd);near(r->car.speed,f->speed);near(r->car.stun,f->stun);near(r->car.roll_time,f->roll_time);CHECK(r->car.roll_direction==f->roll_direction);near(r->car.surge,f->surge);near(r->car.yaw,f->yaw);near(r->car.pitch,f->pitch);near(r->car.roll,f->roll);near(r->hp,f->hp);near(r->wreck,f->wreck);near(r->fire_cd,f->fire_cd);CHECK(r->race.finished==f->finished&&r->is_player==f->player&&r->visible==f->visible);}
static void resource(const Resource *f){near(resources.bolt.heat,f->heat);near(resources.bolt.bolt_cd,f->bolt_cd);CHECK(resources.bolt.side==f->side&&resources.bolt.overheated==f->overheated);near(resources.missiles,f->missiles);near(resources.seeker_cd,f->seeker_cd);near(resources.spin_cd,f->spin_cd);near(resources.emp_cd,f->emp_cd);CHECK(resources.locked==(f->lock<0?NULL:roster[f->lock]));near(resources.lock_time,f->lock_time);}
static void cue(void *p,unsigned event,RRLocalRacer *owner,RRLocalRacer *target){(void)p;CHECK(owner==roster[1]&&event_at<frame->event_count);if(event_at>=frame->event_count)return;const Event *e=frame->events+event_at++;CHECK(event==e->event&&target==(e->target<0?NULL:roster[e->target])&&rng.state==e->rng&&rng.draws==e->draws);resource(&e->resource);for(int i=0;i<4;i++)actor(roster[i],e->actors+i);near(shake,e->shake);if(event>=1&&event<=5)events[event-1]++;}
static void shot(const RRShot *p,const Shot *f){CHECK(p->owner==(f->owner<0?NULL:roster[f->owner])&&p->target==(f->target<0?NULL:roster[f->target]));vector(p->pos,f->pos);vector(p->vel,f->vel);near(p->speed,f->speed);near(p->accel,f->accel);near(p->max_speed,f->max_speed);near(p->life,f->life);near(p->turn,f->turn);near(p->damage,f->damage);near(p->radius,f->radius);near(p->trail_time,f->trail_time);CHECK((int)p->kind==f->kind&&p->always==f->always&&p->trail==f->trail&&p->has_damage==f->has_damage&&p->dodge_tried==f->dodge_tried);near(p->visual.length,f->length);near(p->visual.width,f->width);near(p->visual.flown,f->flown);vector(p->visual.color,f->color);CHECK(p->visual.painted==f->painted);}
int main(void){static RRTrack track;CHECK(rr_track_init_road(&track,1));RRShot shots[64];RRFxLife fx[512];RRFireLocal l={0};unsigned total_draws=0,resets=0,dodges=0;size_t max_shots=0,max_fx=0;
 for(current=0;current<COUNT(frames);current++){
  frame=frames+current;if(frame->first){memset(actors,0,sizeof(actors));memset(&resources,0,sizeof(resources));rng=(Random){frame->seed,0};l=(RRFireLocal){0};l.player=roster[0];l.racers=roster;l.racer_count=4;l.shots=shots;l.capacity=64;l.fx=(RRFxLedger){fx,0,512,false,true};l.random=random_value;l.random_ctx=&rng;l.shake=&shake;}
  if(frame->restart){double bolt=random_value(&rng),reaction=random_value(&rng);CHECK(rr_rival_lifecycle_reset(roster[1],&resources,&track,rr_v3(0,100,0),0,RR_RACE_TWO_LAPS,bolt,reaction));rr_fire_clear(&l);resets++;}
  if(frame->forced_heat){resources.bolt.heat=1;resources.bolt.overheated=true;}
  for(int i=0;i<4;i++)load(roster[i],frame->before+i);shake=frame->shake_before;resource(&frame->before_resource);
  RRFireBinding binding={0};binding.model=frame->model;binding.accent=frame->accent;memcpy(binding.model_world,frame->matrix,sizeof(binding.model_world));
  if(frame->inject){for(unsigned i=0;i<4;i++)(void)random_value(&rng);CHECK(l.count<l.capacity);l.shots[l.count++]=rr_fire_init(roster[0],rr_v3(0,100,80),rr_v3(0,0,-1),0,roster[1],(RRFireOptions){.speed=60,.accel=420,.max_speed=330,.turn=7,.life=3.6,.damage=48,.length=12,.width=6,.radius=5.5,.has_speed=true,.has_turn=true,.has_life=true,.has_damage=true,.always=true,.trail=true,.kind=RR_SHOT_MISSILE}).shot;}
  event_at=0;double before_spin=resources.spin_cd;CHECK(rr_rival_arsenal_step(&l,roster[1],&resources,frame->dt,frame->time,frame->active,frame->road,&binding,cue,NULL));if(resources.spin_cd==6&&before_spin<6)dodges++;
  rr_fire_projectiles_current_tick(&l,frame->dt,24,NULL,NULL,frame->active);CHECK(!l.error&&!l.fx.error&&event_at==frame->event_count&&rng.state==frame->rng&&rng.draws==frame->draws);resource(&frame->resource);for(int i=0;i<4;i++)actor(roster[i],frame->actors+i);near(shake,frame->shake);
  CHECK(l.count==frame->shot_count&&l.fx.count==frame->fx_count);for(size_t i=0;i<l.count&&i<frame->shot_count;i++)shot(shots+i,frame->shots+i);for(size_t i=0;i<l.fx.count&&i<frame->fx_count;i++){near(fx[i].t,frame->fx[i].t);near(fx[i].life,frame->fx[i].life);}if(l.count>max_shots)max_shots=l.count;if(l.fx.count>max_fx)max_fx=l.fx.count;
  if(current+1==COUNT(frames)||frames[current+1].first)total_draws+=rng.draws;
 }
 // Once-per-shot marker survives the common stable-compaction path and adds no
 // slot/shot size growth; actual-source frames also compare every surviving flag.
 CHECK(sizeof(RRShot)==(sizeof(void*)==8?200u:192u));
 printf("RIVAL_ARSENAL_SUMMARY frames=%zu models=4 resets=%u dodges=%u draws=%u events=%u,%u,%u,%u,%u max_shots=%zu max_fx=%zu shot_bytes=%zu max_error=%.17g failures=%u; inactive production,no graphics/rewards/network\n",COUNT(frames),resets,dodges,total_draws,events[0],events[1],events[2],events[3],events[4],max_shots,max_fx,sizeof(RRShot),max_error,failures);return failures?1:0;
}
