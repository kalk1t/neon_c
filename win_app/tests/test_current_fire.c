#include "rr_fire.h"
#include "rr_weapon_current.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "current_fire_reference.h"
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
static void shot(const RRShot *s,ShotFixture f,RRLocalRacer **r){CHECK(s->owner==(f.owner<0?NULL:r[f.owner])&&s->target==(f.target<0?NULL:r[f.target]));vector(s->pos,f.pos);vector(s->vel,f.vel);near(s->speed,f.speed);near(s->accel,f.accel);near(s->max_speed,f.max_speed);near(s->life,f.life);near(s->turn,f.turn);near(s->damage,f.damage);near(s->radius,f.radius);near(s->trail_time,f.trail_time);CHECK((int)s->kind==f.kind&&s->always==f.always&&s->trail==f.trail&&s->has_damage==f.has_damage);near(s->visual.length,f.length);near(s->visual.width,f.width);near(s->visual.flown,f.flown);CHECK(s->visual.painted==f.painted);vector(s->visual.color,f.color);}
typedef struct{bool hit;unsigned count;} World;
static bool world(void *p,RRVec3 a,RRVec3 b,RRVec3 *hit){World *w=p;(void)a;(void)b;*hit=rr_v3(3,80,-20);++w->count;return w->hit;}

typedef struct {Random *random;FlashFixture events[1];size_t count;} FlashLog;
static void flash(void *p,const RRFireFlash *event){FlashLog *log=p;CHECK(log->count<1);if(log->count<1)log->events[log->count++]=(FlashFixture){event->position,event->color,log->random->draws,log->random->state};}
int main(void){
 RRLocalRacer racers[3];RRLocalRacer *roster[3]={racers,racers+1,racers+2};RRShot shots[64];RRFxLife fx[512];RRFireLocal l={0};RRArsenal arsenal;RRLocalLock lock;Random rng={1,0};FlashLog flashes={0};
 unsigned total_draws=0,restarts=0,total_flashes=0;size_t max_shots=0,max_fx=0;
 for(current=0;current<COUNT(current_fire_frames);current++){
  const FrameFixture *f=&current_fire_frames[current];
  if(f->first){reset(racers);rng=(Random){f->seed,0};memset(&l,0,sizeof(l));l.player=racers;l.racers=roster;l.racer_count=3;l.shots=shots;l.capacity=64;l.fx=(RRFxLedger){fx,0,512,false,true};l.random=random_value;l.random_ctx=&rng;l.cue=cue;l.cue_ctx=&rng;l.aim=rr_v3(0,0,-1);rr_arsenal_init(&arsenal);rr_lock_init(&lock);}
  if(f->restart){reset(racers);rr_arsenal_restart(&arsenal);rr_lock_init(&lock);l.locked=NULL;rr_fire_clear(&l);restarts++;}
  for(int i=0;i<3;i++){racers[i].car.roll_time=fmax(0,racers[i].car.roll_time-f->dt);racers[i].car.surge=fmax(0,racers[i].car.surge-f->dt);}racers[0].car.yaw=f->yaw;
  flashes=(FlashLog){0};flashes.random=&rng;RRFireBinding binding={0};binding.model=f->model;binding.accent=f->accent;memcpy(binding.model_world,f->model_world,sizeof(binding.model_world));binding.flash=flash;binding.flash_context=&flashes;
  if(f->inject){for(unsigned i=0;i<4;i++)(void)random_value(&rng);CHECK(l.count<l.capacity);l.shots[l.count++]=rr_fire_init(roster[2],roster[2]->car.pos,rr_v3(0,0,1),0,roster[0],(RRFireOptions){0}).shot;}
  if(f->smoke)CHECK(rr_fx_request(&l.fx,RR_FX_SMOKE,random_value,&rng));
  CHECK(((racers[0].wreck<=0)&&f->race_active)==f->active);
  if(f->active){RRLocalTarget targets[2];for(int i=0;i<2;i++)targets[i]=(RRLocalTarget){racers[i+1].car.pos,racers[i+1].car.vel,true,racers[i+1].wreck<=0,racers[i+1].race.finished};rr_weapon_current_lock(&lock,racers[0].car.pos,l.aim,targets,2,f->dt);l.locked=lock.target<0?NULL:roster[lock.target+1];RRArsenalInput input={f->fire,f->missile,f->spin,f->emp,l.locked!=NULL,f->steer,lock.charge};RRArsenalEvents events;CHECK(rr_fire_player_current_step(&l,&arsenal,f->dt,input,&binding,&events));}
  World w={f->world_hit,0};rr_fire_projectiles_current_tick(&l,f->dt,24,world,&w,f->race_active);if(!l.locked)lock.target=-1;
  CHECK(!l.error&&!l.fx.error&&rng.state==f->rng&&rng.draws==f->draws&&w.count==f->world_calls);
  near(arsenal.heat,f->arsenal.heat);near(arsenal.bolt_cd,f->arsenal.bolt_cd);near(arsenal.missiles,f->arsenal.missiles);near(arsenal.spin_cd,f->arsenal.spin_cd);near(arsenal.emp_cd,f->arsenal.emp_cd);CHECK(arsenal.overheated==f->arsenal.overheated&&arsenal.side==f->arsenal.side);
  CHECK((lock.target<0?-1:lock.target+1)==f->lock);near(lock.charge,f->charge);
  for(int i=0;i<3;i++)actor(roster[i],f->actors[i]);CHECK(l.count==f->shot_count&&l.fx.count==f->fx_count);
  for(size_t i=0;i<l.count&&i<f->shot_count;i++)shot(shots+i,f->shots[i],roster);
  for(size_t i=0;i<l.fx.count&&i<f->fx_count;i++){near(fx[i].t,f->fx[i].t);near(fx[i].life,f->fx[i].life);}
  CHECK(flashes.count==f->flash_count);for(size_t i=0;i<flashes.count&&i<f->flash_count;i++){vector(flashes.events[i].position,f->flashes[i].position);vector(flashes.events[i].color,f->flashes[i].color);CHECK(flashes.events[i].draws==f->flashes[i].draws&&flashes.events[i].rng==f->flashes[i].rng);}total_flashes+=(unsigned)flashes.count;
  if(l.count>max_shots)max_shots=l.count;if(l.fx.count>max_fx)max_fx=l.fx.count;
  if(current+1==COUNT(current_fire_frames)||current_fire_frames[current+1].first)total_draws+=rng.draws;
 }
 // A real segment/racer impact still removes the projectile and emits its spark
 // after finish, but cannot damage the victim. The same active impact does 21.
 for(unsigned active=0;active<2;active++){
  reset(racers);l.count=1;l.fx.count=0;l.error=false;l.fx.error=false;
  shots[0]=rr_fire_init(roster[0],rr_v3(0,100,-59),rr_v3(0,0,-1),0,NULL,rr_weapon_current_bolt_options()).shot;
  rr_fire_projectiles_current_tick(&l,.01,24,NULL,NULL,active!=0);
  CHECK(l.count==0&&l.fx.count==(active?13u:1u)&&!l.error&&!l.fx.error);
  near(racers[1].hp,active?0:20);near(racers[1].wreck,active?2.4:0);
  // Active lethal impact adds the source explosion as well as the spark.
 }
 // Validation failures are observable and consume no RNG before dispatch.
 RRFireBinding bad={0};RRArsenalEvents events;unsigned before=rng.draws;CHECK(!rr_fire_player_current_step(&l,&arsenal,.01,(RRArsenalInput){0},&bad,&events)&&l.error&&rng.draws==before);l.error=false;bad.model_world[0]=NAN;CHECK(!rr_fire_player_current_step(&l,&arsenal,.01,(RRArsenalInput){0},&bad,&events)&&l.error&&rng.draws==before);
 // Flight-based streak views use retained per-shot dimensions, not a shared bolt-only size.
 RRShot view={0};view.pos=rr_v3(0,0,-20);view.vel=rr_v3(0,0,-845);view.visual=(RRShotVisual){45,7,20,{0},true};RRShotStreak shape=rr_fire_shot_streak(&view,rr_v3(0,0,0));near(shape.length,21);near(shape.center.z,-9.5);near(shape.width,7*(1+20./120));
 printf("CURRENT_FIRE_SUMMARY frames=%zu models=4 restarts=%u draws=%u flashes=%u max_shots=%zu max_fx=%zu shot_bytes=%zu visual_bytes=%zu max_error=%.17g failures=%u; production inactive,no GPU/user-store/network\n",COUNT(current_fire_frames),restarts,total_draws,total_flashes,max_shots,max_fx,sizeof(RRShot),sizeof(RRShotVisual),max_error,failures);return failures?1:0;
}
