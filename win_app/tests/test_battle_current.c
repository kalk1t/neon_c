#include "rr_battle_current.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "battle_current_reference.h"
static unsigned failures;static size_t current;static double worst;
#define CHECK(x) do{if(!(x)){if(failures<15)fprintf(stderr,"case %zu line %d: %s\n",current,__LINE__,#x);++failures;}}while(0)
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-5);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
typedef struct{uint32_t state;unsigned draws;} Random;
static double random_value(void *p){Random *r=p;r->state=r->state*UINT32_C(1664525)+UINT32_C(1013904223);r->draws++;return r->state/4294967296.0;}

static void cue(void *p,unsigned kind){if(kind==RR_ARSENAL_BOLT)(void)random_value(p);}
static void load(RRLocalRacer *r,const BattleState *s){
 r->car.pos=s->pos;r->car.vel=s->vel;r->car.fwd=s->fwd;r->car.yaw=s->yaw;r->car.pitch=s->pitch;r->car.roll=s->roll;r->car.speed=s->speed;r->car.boost=s->boost;r->car.max_multiplier=s->max;r->car.stun=s->stun;r->car.t=s->t;r->car.roll_time=s->roll_time;r->car.surge=s->surge;r->car.roll_direction=s->roll_dir;r->car.boosting=s->boosting;r->ai.sample=s->sample;r->ai.dodge=s->dodge;
 r->race.mode=RR_RACE_TWO_LAPS;r->race.lap=s->lap;r->race.next_checkpoint=s->next;r->lap_count=(size_t)s->lap_count;r->race.lap_time=s->lap_time;r->race.finish_time=s->finish_time;r->hp=s->hp;r->wreck=s->wreck;r->lost=s->lost;r->stuck=s->stuck;r->coins=s->coins;r->race.finished=s->finished;r->visible=s->visible;r->passed=s->passed;r->fire_cd=s->fire_cd;r->react=s->react;r->smoke_time=s->smoke;
}
static void actor(const RRLocalRacer *r,const BattleState *s){
 vector(r->car.pos,s->pos);vector(r->car.vel,s->vel);vector(r->car.fwd,s->fwd);near(r->car.yaw,s->yaw);near(r->car.pitch,s->pitch);near(r->car.roll,s->roll);near(r->car.speed,s->speed);near(r->car.boost,s->boost);near(r->car.max_multiplier,s->max);near(r->car.stun,s->stun);near(r->car.t,s->t);near(r->car.roll_time,s->roll_time);near(r->car.surge,s->surge);CHECK(r->car.roll_direction==s->roll_dir&&r->car.boosting==s->boosting&&r->ai.sample==s->sample);near(r->ai.dodge,s->dodge);CHECK(r->race.lap==s->lap&&r->race.next_checkpoint==s->next&&r->lap_count==(size_t)s->lap_count);near(r->race.lap_time,s->lap_time);near(r->race.finish_time,s->finish_time);near(r->hp,s->hp);near(r->wreck,s->wreck);near(r->lost,s->lost);near(r->stuck,s->stuck);CHECK(r->coins==s->coins&&r->race.finished==s->finished&&r->visible==s->visible&&r->passed==s->passed);near(r->fire_cd,s->fire_cd);near(r->react,s->react);near(r->smoke_time,s->smoke);
}
static void shot(const RRShot *s,const ShotFixture *f,RRLocalRacer **r){CHECK(s->owner==(f->owner<0?NULL:r[f->owner])&&s->target==(f->target<0?NULL:r[f->target]));vector(s->pos,f->pos);vector(s->vel,f->vel);near(s->speed,f->speed);near(s->accel,f->accel);near(s->max_speed,f->max_speed);near(s->life,f->life);near(s->turn,f->turn);near(s->damage,f->damage);near(s->radius,f->radius);near(s->trail_time,f->trail_time);CHECK((int)s->kind==f->kind&&s->always==f->always&&s->trail==f->trail&&s->has_damage==f->has_damage&&s->dodge_tried==f->dodge_tried);near(s->visual.length,f->length);near(s->visual.width,f->width);near(s->visual.flown,f->flown);vector(s->visual.color,f->color);CHECK(s->visual.painted==f->painted);}
static RRBattleCurrent tuned;static const FrameFixture *binding_frame;static unsigned binding_calls;
static RRTrack track;static RRLocalPack pack;static RRLocalRacer player;static RRLocalBattle battle;
static bool binding(void *ctx,int index,const RRLocalRacer *r,RRFireBinding *out){(void)ctx;if(index==0)vector(r->car.pos,binding_frame->before.pos);CHECK(index>=0&&index<11&&r==battle.roster[index]);const Binding *f=binding_frame->bindings+index;CHECK(f->valid);if(!f->valid)return false;*out=(RRFireBinding){0};out->model=f->model;out->accent=f->accent;memcpy(out->model_world,f->matrix,sizeof(out->model_world));binding_calls++;return true;}
static void resources(const RRRivalLifeResources *s,const Resource *f){near(s->bolt.heat,f->heat);near(s->bolt.bolt_cd,f->bolt_cd);CHECK(s->bolt.side==f->side&&s->bolt.overheated==f->overheated);near(s->missiles,f->missiles);near(s->seeker_cd,f->seeker_cd);near(s->spin_cd,f->spin_cd);near(s->emp_cd,f->emp_cd);CHECK(s->locked==(f->lock<0?NULL:battle.roster[f->lock]));near(s->lock_time,f->lock_time);}
static void metadata(const Meta *m){for(int i=0;i<11;i++)CHECK(pack.ranking[i]==m->ranking[i]);CHECK(pack.position==m->position);near(pack.rank_timer,m->rank_timer);near(pack.shake,m->shake);}
static void start(const ResetFixture *f,Random *rng){load(&player,&f->player);player.is_player=true;(void)random_value(rng);(void)random_value(rng);CHECK(rr_battle_current_start(&tuned,&battle,&pack,&track,&player));CHECK(rng->state==f->rng&&rng->draws==f->draws);for(int i=0;i<11;i++){actor(battle.roster[i],&f->after[i]);if(i)resources(&tuned.pack.resources[i-1],&f->after[i].resources);}metadata(&f->meta);}
static void mutate(unsigned m){
 if(m&1)for(int i=0;i<10;i++)pack.rivals[i].fire_cd=0;
 if(m&4){RRLocalRacer *r=pack.rivals;rr_car_place(&r->car,rr_add(player.car.pos,rr_v3(2,0,0)),0,0);r->car.vel=rr_v3(-5,0,0);}
 if(m&8)pack.rivals[0].hp=40;
 if(m&16){pack.rivals[0].hp=0;pack.rivals[0].wreck=.03;pack.rivals[0].visible=false;}
 if(m&32){pack.rivals[1].stuck=2.49;pack.rivals[1].car.speed=0;pack.rivals[1].car.vel=rr_v3(0,0,0);}
 if(m&64){pack.rivals[2].car.pos.y=25;pack.rivals[2].lost=1.09;}
 if(m&128){pack.rivals[0].hp=0;pack.rivals[0].wreck=2.4;pack.rivals[0].visible=false;}
}
int main(void){
 Random rng={0};RRShot shots[64];RRFxLife fx[512];int trace=-1;unsigned restarts=0,respawns=0,smoke=0,contacts=0,collections=0,paused=0,countdown=0,total_draws=0;size_t max_shots=0,max_fx=0;
 for(current=0;current<COUNT(frame_fixtures);current++){
  const FrameFixture *f=&frame_fixtures[current];
  if(trace!=(int)f->trace){rr_pack_dispose(&pack);memset(&pack,0,sizeof(pack));memset(&player,0,sizeof(player));memset(&battle,0,sizeof(battle));memset(&tuned,0,sizeof(tuned));tuned.binding=binding;trace=(int)f->trace;const TraceFixture *t=&trace_fixtures[trace];CHECK(rr_track_init_road(&track,t->road));pack.rank_timer=.13;pack.shake=.1;for(int i=0;i<10;i++){rr_car_reset(&pack.rivals[i].car,rr_v3(0,118,0),0,0);pack.rivals[i].ai=(RRAIDriver){0,t->drivers[i][0],t->drivers[i][1],t->drivers[i][2],t->drivers[i][3],t->drivers[i][4]};}rng=(Random){t->seed,0};battle.fire.shots=shots;battle.fire.capacity=64;battle.fire.fx=(RRFxLedger){fx,0,512,false,true};battle.fire.random=random_value;battle.fire.random_ctx=&rng;battle.fire.cue=cue;battle.fire.cue_ctx=&rng;rr_arsenal_init(&battle.arsenal);rr_lock_init(&battle.lock);start(&t->start,&rng);}
  if(f->restart){start(&f->reset,&rng);restarts++;}
  load(&player,&f->before);player.is_player=true;mutate(f->mask);battle.fire.muzzle=f->muzzle;battle.fire.side_axis=f->side;battle.fire.aim=rr_v3(0,0,-1);
  RRArsenalInput input={f->fire,f->missile,f->spin,f->emp,false,f->steer,0};RRPackEvents e;binding_frame=f;CHECK(rr_battle_current_after_player(&tuned,f->branch,f->active,f->dt,f->meta.time,f->race_time,NULL,NULL,NULL,input,&e));CHECK(!e.error&&!battle.fire.error&&!battle.fire.fx.error&&rng.state==f->rng&&rng.draws==f->draws);
  for(int i=0;i<11;i++){actor(battle.roster[i],&f->after[i]);if(i)resources(&tuned.pack.resources[i-1],&f->after[i].resources);}metadata(&f->meta);
  near(battle.arsenal.heat,f->arsenal.heat);near(battle.arsenal.bolt_cd,f->arsenal.bolt_cd);near(battle.arsenal.missiles,f->arsenal.missiles);near(battle.arsenal.spin_cd,f->arsenal.spin_cd);near(battle.arsenal.emp_cd,f->arsenal.emp_cd);CHECK(battle.arsenal.overheated==f->arsenal.overheated&&battle.arsenal.side==f->arsenal.side);CHECK((battle.lock.target<0?-1:battle.lock.target+1)==f->lock);near(battle.lock.charge,f->charge);
  CHECK(battle.fire.count==f->shot_count&&battle.fire.fx.count==f->fx_count);for(size_t i=0;i<battle.fire.count&&i<f->shot_count;i++)shot(shots+i,f->shots+i,battle.roster);for(size_t i=0;i<battle.fire.fx.count&&i<f->fx_count;i++){near(fx[i].t,f->fx[i].t);near(fx[i].life,f->fx[i].life);}
  CHECK(pack.coins.count==f->coin_count&&e.coins.count==f->collection_count);for(int i=0;i<pack.coins.count;i++){CHECK(pack.coins.list[i].active==f->coins[i].active);near(pack.coins.list[i].timer,f->coins[i].timer);near(pack.coins.list[i].pop,f->coins[i].pop);}for(int i=0;i<e.coins.count&&i<f->collection_count;i++)CHECK(e.coins.list[i].coin==f->collections[i][0]&&e.coins.list[i].racer==f->collections[i][1]);
  for(int i=0;i<10;i++){respawns+=(e.rival[i]&RR_RIVAL_RESPAWN)!=0;smoke+=(e.rival[i]&RR_RIVAL_SMOKE)!=0;}contacts+=(unsigned)e.contacts.count;collections+=(unsigned)e.coins.count;paused+=f->branch==RR_PACK_PAUSED;countdown+=f->branch==RR_PACK_COUNTDOWN;if(battle.fire.count>max_shots)max_shots=battle.fire.count;if(battle.fire.fx.count>max_fx)max_fx=battle.fire.fx.count;
  if(current+1==COUNT(frame_fixtures)||frame_fixtures[current+1].trace!=f->trace)total_draws+=rng.draws;
 }
 rr_pack_dispose(&pack);
 printf("BATTLE_CURRENT_SUMMARY roads=%zu frames=%zu restarts=%u respawns=%u smoke=%u contacts=%u collections=%u paused=%u countdown=%u sequence_draws=%u max_shots=%zu max_fx=%zu max_error=%.17g failures=%u\n",COUNT(trace_fixtures),COUNT(frame_fixtures),restarts,respawns,smoke,contacts,collections,paused,countdown,total_draws,max_shots,max_fx,worst,failures);printf("CURRENT_BATTLE_DETAIL bindings=%u context_bytes=%zu production_inactive=true\n",binding_calls,sizeof(tuned));return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

