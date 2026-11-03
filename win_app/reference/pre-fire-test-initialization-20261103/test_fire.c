#include "rr_fire.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generated_fire_reference.h"
static unsigned failures;static size_t current;static double max_error;
#define CHECK(x) do{if(!(x)){if(failures<14)fprintf(stderr,"case %zu line %d: %s\n",current,__LINE__,#x);++failures;}}while(0)
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
static void near(double a,double b){double e=fabs(a-b);if(e>max_error)max_error=e;CHECK(isfinite(a)&&e<1e-10);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
typedef struct{uint32_t state;unsigned draws;} Random;
static double random_value(void *p){Random *r=p;r->state=r->state*UINT32_C(1664525)+UINT32_C(1013904223);++r->draws;return r->state/4294967296.0;}
static void cue(void *p,unsigned kind){if(kind==RR_ARSENAL_BOLT)(void)random_value(p);}
static void reset(RRLocalRacer *r){for(unsigned i=0;i<3;i++){memset(r+i,0,sizeof(*r));r[i].car.pos=rr_v3(i==2?20:0,100,i==0?0:i==1?-60:-80);r[i].car.vel=rr_v3(0,0,i==0?-95:0);r[i].car.speed=i==0?95:0;r[i].car.fwd=rr_v3(0,0,-1);r[i].car.roll_direction=1;r[i].hp=i==1?20:100;r[i].is_player=i==0;r[i].visible=true;}}
static void actor(const RRLocalRacer *r,ActorFixture f){vector(r->car.pos,f.pos);vector(r->car.vel,f.vel);near(r->car.speed,f.speed);near(r->car.stun,f.stun);near(r->car.roll_time,f.roll);near(r->car.surge,f.surge);near(r->car.yaw,f.yaw);near(r->hp,f.hp);near(r->wreck,f.wreck);near(r->fire_cd,f.fire_cd);CHECK(r->race.finished==f.finished&&r->is_player==f.player&&r->visible==f.visible);}
static void shot(const RRShot *s,ShotFixture f,RRLocalRacer **r){CHECK(s->owner==(f.owner<0?NULL:r[f.owner])&&s->target==(f.target<0?NULL:r[f.target]));vector(s->pos,f.pos);vector(s->vel,f.vel);near(s->speed,f.speed);near(s->accel,f.accel);near(s->max_speed,f.max_speed);near(s->life,f.life);near(s->turn,f.turn);near(s->damage,f.damage);near(s->radius,f.radius);near(s->trail_time,f.trail_time);CHECK((int)s->kind==f.kind&&s->always==f.always&&s->trail==f.trail&&s->has_damage==f.has_damage);}
typedef struct{bool hit;unsigned count;} World;
static bool world(void *p,RRVec3 a,RRVec3 b,RRVec3 *hit){World *w=p;(void)a;(void)b;*hit=rr_v3(3,80,-20);++w->count;return w->hit;}
int main(void){
 RRLocalRacer racers[3];RRLocalRacer *roster[3]={racers,racers+1,racers+2};reset(racers);
 for(current=0;current<COUNT(init_fixtures);current++){const InitFixture *f=&init_fixtures[current];RRFireInit out=rr_fire_init(roster[f->owner],rr_v3(1,100,-3),f->dir,f->inherit,f->target<0?NULL:roster[f->target],f->options);shot(&out.shot,f->expected,roster);near(out.length,f->length);near(out.width,f->width);CHECK(f->draws==4);}
 unsigned fx_draws=0;
 for(current=0;current<COUNT(fx_fixtures);current++){const FxFixture *f=&fx_fixtures[current];RRFxLife items[512];RRFxLedger ledger={items,f->initial,512,false,true};for(size_t i=0;i<f->initial;i++)items[i]=(RRFxLife){.1,1};Random rng={f->seed,0};CHECK(rr_fx_request(&ledger,f->kind,random_value,&rng));rr_fx_tick(&ledger,.05);CHECK(!ledger.error&&ledger.count==f->count&&rng.draws==f->draws&&rng.state==f->rng);for(size_t i=0;i<ledger.count&&i<f->count;i++){near(items[i].t,f->fx[i].t);near(items[i].life,f->fx[i].life);}fx_draws+=rng.draws;}
 /* Insufficient experimental storage is an explicit error, never a hidden cap. */
 RRFxLife tiny[1];RRFxLedger shortage={tiny,0,1,false,true};Random rng={1,0};CHECK(!rr_fx_request(&shortage,RR_FX_EXPLODE,random_value,&rng)&&shortage.error&&rng.draws==0);
 RRShot shots[64];RRFxLife fx[512];RRFireLocal l={0};RRArsenal arsenal;RRLocalLock lock;
 unsigned total_draws=0,restarts=0;size_t max_shots=0,max_fx=0;
 for(current=0;current<COUNT(frame_fixtures);current++){
  const FrameFixture *f=&frame_fixtures[current];
  if(f->first){reset(racers);rng=(Random){f->seed,0};memset(&l,0,sizeof(l));l.player=racers;l.racers=roster;l.racer_count=3;l.shots=shots;l.capacity=64;l.fx=(RRFxLedger){fx,0,512,false,true};l.random=random_value;l.random_ctx=&rng;l.cue=cue;l.cue_ctx=&rng;l.side_axis=rr_v3(1,0,0);l.aim=rr_v3(0,0,-1);rr_arsenal_init(&arsenal);rr_lock_init(&lock);}
  if(f->restart){reset(racers);rr_arsenal_restart(&arsenal);rr_lock_init(&lock);l.locked=NULL;rr_fire_clear(&l);++restarts;}
  for(unsigned i=0;i<3;i++){racers[i].car.roll_time=fmax(0,racers[i].car.roll_time-f->dt);racers[i].car.surge=fmax(0,racers[i].car.surge-f->dt);}
  if(f->inject){for(unsigned i=0;i<4;i++)(void)random_value(&rng);RRFireOptions o={0};CHECK(l.count<l.capacity);l.shots[l.count++]=rr_fire_init(roster[2],roster[2]->car.pos,rr_v3(0,0,1),0,roster[0],o).shot;}
  if(f->smoke)CHECK(rr_fx_request(&l.fx,RR_FX_SMOKE,random_value,&rng));
  CHECK((racers[0].wreck<=0)==f->active);
  if(f->active){RRLocalTarget targets[2];for(unsigned i=0;i<2;i++)targets[i]=(RRLocalTarget){racers[i+1].car.pos,racers[i+1].car.vel,true,racers[i+1].wreck<=0,racers[i+1].race.finished};(void)rr_lock_update(&lock,racers[0].car.pos,l.aim,targets,2,f->dt);l.locked=lock.target<0?NULL:roster[lock.target+1];l.muzzle=rr_add(racers[0].car.pos,rr_v3(0,-.4,-3.15));RRArsenalInput input={f->fire,f->missile,f->spin,f->emp,l.locked!=NULL,f->steer,lock.charge};RRArsenalEvents events;rr_fire_player_step(&l,&arsenal,f->dt,input,&events);}
  World w={f->world_hit,0};rr_fire_projectiles_tick(&l,f->dt,24,world,&w);if(!l.locked)lock.target=-1;
  CHECK(!l.error&&!l.fx.error&&rng.state==f->rng&&rng.draws==f->draws&&w.count==f->world_calls);
  near(arsenal.heat,f->arsenal.heat);near(arsenal.bolt_cd,f->arsenal.bolt_cd);near(arsenal.missiles,f->arsenal.missiles);near(arsenal.spin_cd,f->arsenal.spin_cd);near(arsenal.emp_cd,f->arsenal.emp_cd);CHECK(arsenal.overheated==f->arsenal.overheated&&arsenal.side==f->arsenal.side);
  CHECK((lock.target<0?-1:lock.target+1)==f->lock);near(lock.charge,f->charge);
  for(unsigned i=0;i<3;i++)actor(roster[i],f->actors[i]);CHECK(l.count==f->shot_count&&l.fx.count==f->fx_count);
  for(size_t i=0;i<l.count&&i<f->shot_count;i++)shot(shots+i,f->shots[i],roster);
  for(size_t i=0;i<l.fx.count&&i<f->fx_count;i++){near(fx[i].t,f->fx[i].t);near(fx[i].life,f->fx[i].life);}
  if(l.count>max_shots)max_shots=l.count;if(l.fx.count>max_fx)max_fx=l.fx.count;
  if(current+1==COUNT(frame_fixtures)||frame_fixtures[current+1].first)total_draws+=rng.draws;
 }
 printf("FIRE_SUMMARY init_cases=%zu fx_cases=%zu seeded_frames=%zu restarts=%u fx_probe_draws=%u sequence_draws=%u max_shots=%zu max_fx=%zu max_error=%.17g failures=%u\n",COUNT(init_fixtures),COUNT(fx_fixtures),COUNT(frame_fixtures),restarts,fx_draws,total_draws,max_shots,max_fx,max_error,failures);
 return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
