#include "rr_local_run.h"
#include "rr_physics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generated_run_reference.h"
static unsigned failures;static size_t current;static double worst;
#define CHECK(x) do{if(!(x)){if(failures<16)fprintf(stderr,"case %zu line %d: %s\n",current,__LINE__,#x);++failures;}}while(0)
#define COUNT(x) (sizeof(x)/sizeof((x)[0]))
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-5);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
typedef struct{uint32_t state;unsigned draws;} Random;
static double random_value(void *p){Random *r=p;r->state=r->state*UINT32_C(1664525)+UINT32_C(1013904223);r->draws++;return r->state/4294967296.0;}
static void cue(void *p,unsigned kind){if(kind==RR_ARSENAL_BOLT)(void)random_value(p);}
static void actor(const RRLocalRacer *r,const State *s){
 vector(r->car.pos,s->pos);vector(r->car.vel,s->vel);vector(r->car.fwd,s->fwd);near(r->car.yaw,s->yaw);near(r->car.pitch,s->pitch);near(r->car.roll,s->roll);near(r->car.speed,s->speed);near(r->car.boost,s->boost);near(r->car.max_multiplier,s->max);near(r->car.stun,s->stun);near(r->car.t,s->t);near(r->car.roll_time,s->roll_time);near(r->car.surge,s->surge);CHECK(r->car.roll_direction==s->roll_dir&&r->car.boosting==s->boosting&&r->ai.sample==s->sample);near(r->ai.dodge,s->dodge);CHECK(r->race.lap==s->lap&&r->race.next_checkpoint==s->next&&r->lap_count==(size_t)s->lap_count);near(r->race.lap_time,s->lap_time);near(r->race.finish_time,s->finish_time);near(r->hp,s->hp);near(r->wreck,s->wreck);near(r->lost,s->lost);near(r->stuck,s->stuck);CHECK(r->coins==s->coins&&r->race.finished==s->finished&&r->visible==s->visible&&r->passed==s->passed);near(r->fire_cd,s->fire_cd);near(r->react,s->react);near(r->smoke_time,s->smoke);near(r->wrong_way,s->wrong_way);vector(r->previous,s->previous);vector(r->race.safe_position,s->safe_pos);near(r->race.safe_yaw,s->safe_yaw);for(size_t i=0;i<r->lap_count&&i<2;i++)near(r->lap_times[i],s->history[i]);
}
static void shot(const RRShot *s,const ShotFixture *f,RRLocalRacer **r){CHECK(s->owner==(f->owner<0?NULL:r[f->owner])&&s->target==(f->target<0?NULL:r[f->target]));vector(s->pos,f->pos);vector(s->vel,f->vel);near(s->speed,f->speed);near(s->accel,f->accel);near(s->max_speed,f->max_speed);near(s->life,f->life);near(s->turn,f->turn);near(s->damage,f->damage);near(s->radius,f->radius);near(s->trail_time,f->trail_time);CHECK((int)s->kind==f->kind&&s->always==f->always&&s->trail==f->trail&&s->has_damage==f->has_damage);}
static RRTrack track;static RRWorld world;static RRLocalPack pack;static RRLocalRacer player;static RRLocalBattle battle;static RRLocalRun run;
static void metadata(const Meta *m){for(int i=0;i<11;i++)CHECK(pack.ranking[i]==m->ranking[i]);CHECK(pack.position==m->position&&pack.finish_position==m->finish_position);near(pack.rank_timer,m->rank_timer);near(pack.shake,m->shake);near(run.time,m->time);near(run.last_nitro,m->last_nitro);near(run.countdown,m->countdown);CHECK(run.last_count==m->last_count&&(int)run.phase==m->phase);}
static void start(const ResetFixture *f,Random *rng){CHECK(rr_local_run_start(&run,&battle,&track,&player));CHECK(rng->state==f->rng&&rng->draws==f->draws);for(int i=0;i<11;i++)actor(battle.roster[i],&f->after[i]);metadata(&f->meta);}
static void mutate(unsigned m){
 if(m&1)rr_car_place(&player.car,rr_v3(8,115.5,64),0,95);
 if(m&2){player.hp=40;player.wreck=0;player.visible=true;}
 if(m&4){player.hp=0;player.wreck=.1;player.visible=false;}
 if(m&8){player.car.pos.y=25;player.lost=1.09;}
 if(m&16){const RRGate *g=&track.gates[0];rr_car_place(&player.car,rr_sub(g->pos,rr_scale(g->forward,.5)),g->yaw,45);player.race.next_checkpoint=0;player.passed=0;}
 if(m&32){rr_car_place(&player.car,rr_v3(0,126,.2),0,35);player.race.next_checkpoint=track.gate_count;player.passed=(UINT32_C(1)<<track.gate_count)-1;}
}
typedef struct {unsigned calls,hits;} Segments;
static bool segment(void *p,RRVec3 a,RRVec3 b,RRVec3 *hit){Segments *s=p;s->calls++;bool result=rr_world_segment(&world,a,b,.6,hit);s->hits+=result;return result;}
int main(void){
 Random rng={0};RRShot shots[128];RRFxLife fx[512];int trace=-1;unsigned restarts=0,go=0,laps=0,finishes=0,respawns=0,manual=0,smoke=0,impacts=0,nitro=0,paused=0,total_draws=0,world_hits=0,collections=0;size_t max_shots=0,max_fx=0;
 for(current=0;current<COUNT(frames);current++){
  const FrameFixture *f=&frames[current];
  if(trace!=(int)f->trace){rr_pack_dispose(&pack);rr_rival_dispose(&player);memset(&pack,0,sizeof(pack));memset(&player,0,sizeof(player));memset(&battle,0,sizeof(battle));memset(&run,0,sizeof(run));memset(&world,0,sizeof(world));trace=(int)f->trace;const TraceFixture *t=&traces[trace];CHECK(rr_track_init_road(&track,t->road));if(t->road==9){CHECK(rr_world_add(&world,rr_box(rr_v3(8,118,55),rr_v3(12,10,1.2),.15)));CHECK(rr_world_add(&world,rr_cylinder(-8,45,4,100,135)));CHECK(rr_world_add(&world,rr_torus(rr_v3(0,126,10),0,12,1.4)));}pack.rank_timer=.13;pack.shake=.1;rr_car_reset(&player.car,rr_v3(0,118,0),0,0);player.is_player=true;for(int i=0;i<10;i++){rr_car_reset(&pack.rivals[i].car,rr_v3(0,118,0),0,0);pack.rivals[i].ai=(RRAIDriver){0,t->drivers[i][0],t->drivers[i][1],t->drivers[i][2],t->drivers[i][3],t->drivers[i][4]};}rng=(Random){t->seed,0};battle.pack=&pack;battle.fire.shots=shots;battle.fire.capacity=128;battle.fire.fx=(RRFxLedger){fx,0,512,false,true};battle.fire.random=random_value;battle.fire.random_ctx=&rng;battle.fire.cue=cue;battle.fire.cue_ctx=&rng;rr_arsenal_init(&battle.arsenal);rr_lock_init(&battle.lock);run.time=17.25;start(&t->start,&rng);}
  if(f->restart){start(&f->reset,&rng);restarts++;}
  if(f->entered==RR_SESSION_PAUSED&&run.phase!=RR_SESSION_PAUSED)CHECK(rr_local_run_pause(&run));if(f->entered!=RR_SESSION_PAUSED&&run.phase==RR_SESSION_PAUSED)CHECK(rr_local_run_resume(&run));
  CHECK((int)run.phase==f->entered);mutate(f->mask);RRRunInput input={f->control,{f->fire,f->missile,f->spin,f->emp,false,f->steer,0},f->aim,f->manual,true};Segments segments={0};RRRunEvents e;CHECK(rr_local_run_tick(&run,input,f->raw,&world,segment,&segments,&e));CHECK(e.player==f->player_events);CHECK(!run.error&&!battle.fire.error&&!battle.fire.fx.error&&rng.state==f->rng&&rng.draws==f->draws&&segments.calls==f->segment_calls&&segments.hits==f->segment_hits);
  for(int i=0;i<11;i++)actor(battle.roster[i],&f->after[i]);metadata(&f->meta);near(run.race_time,f->race_time);
  near(e.motion.impact,f->motion.impact);CHECK(e.motion.collided==f->motion.collided);if(f->motion.collided)vector(e.motion.normal,f->motion.normal);
  near(battle.arsenal.heat,f->arsenal.heat);near(battle.arsenal.bolt_cd,f->arsenal.bolt_cd);near(battle.arsenal.missiles,f->arsenal.missiles);near(battle.arsenal.spin_cd,f->arsenal.spin_cd);near(battle.arsenal.emp_cd,f->arsenal.emp_cd);CHECK(battle.arsenal.overheated==f->arsenal.overheated&&battle.arsenal.side==f->arsenal.side);CHECK((battle.lock.target<0?-1:battle.lock.target+1)==f->lock);near(battle.lock.charge,f->charge);
  CHECK(battle.fire.count==f->shot_count&&battle.fire.fx.count==f->fx_count);for(size_t i=0;i<battle.fire.count&&i<f->shot_count;i++)shot(shots+i,f->shots+i,battle.roster);for(size_t i=0;i<battle.fire.fx.count&&i<f->fx_count;i++){near(fx[i].t,f->fx[i].t);near(fx[i].life,f->fx[i].life);}
  CHECK(pack.coins.count==f->coin_count&&e.pack.coins.count==f->collection_count);for(int i=0;i<pack.coins.count;i++){CHECK(pack.coins.list[i].active==f->coins[i].active);near(pack.coins.list[i].timer,f->coins[i].timer);near(pack.coins.list[i].pop,f->coins[i].pop);}for(int i=0;i<e.pack.coins.count&&i<f->collection_count;i++)CHECK(e.pack.coins.list[i].coin==f->collections[i][0]&&e.pack.coins.list[i].racer==f->collections[i][1]);
  go+=(e.player&RR_RUN_GO)!=0;laps+=(e.player&RR_RACE_LAP)!=0;finishes+=(e.player&RR_RACE_FINISH)!=0;respawns+=(e.player&RR_RUN_RESPAWN)!=0;manual+=(e.player&RR_RUN_MANUAL)!=0;smoke+=(e.player&RR_RUN_SMOKE)!=0;impacts+=(e.player&RR_RUN_IMPACT)!=0;nitro+=(e.player&RR_RUN_NITRO)!=0;paused+=f->entered==RR_SESSION_PAUSED;world_hits+=segments.hits;collections+=(unsigned)e.pack.coins.count;if(battle.fire.count>max_shots)max_shots=battle.fire.count;if(battle.fire.fx.count>max_fx)max_fx=battle.fire.fx.count;
  if(current+1==COUNT(frames)||frames[current+1].trace!=f->trace)total_draws+=rng.draws;
 }
 rr_pack_dispose(&pack);rr_rival_dispose(&player);
 printf("RUN_SUMMARY roads=%zu frames=%zu restarts=%u go=%u laps=%u finishes=%u respawns=%u manual=%u smoke=%u impacts=%u nitro=%u paused=%u world_hits=%u collections=%u sequence_draws=%u max_shots=%zu max_fx=%zu max_error=%.17g failures=%u\n",COUNT(traces),COUNT(frames),restarts,go,laps,finishes,respawns,manual,smoke,impacts,nitro,paused,world_hits,collections,total_draws,max_shots,max_fx,worst,failures);return failures?EXIT_FAILURE:EXIT_SUCCESS;
}
