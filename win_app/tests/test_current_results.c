#include "rr_run_input.h"
#include "rr_results_current.h"
#include <stdint.h>
#include "rr_physics.h"
#include "rr_weapon_current.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "run_input_reference.h"
#include "current_results_reference.h"
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
static RRBattleCurrent tuned;static const FrameFixture *binding_frame;static RRRunInputState inputs;static RRRunCurrent run;static RRWorld world;
static unsigned go,impacts,nitro,world_hits,default_frames,blocked_manual;
static unsigned binding_calls,player_respawns,player_smoke,player_finishes,manual_resets;
static RRTrack track;static RRLocalPack pack;static RRLocalRacer player;static RRLocalBattle battle;
static bool binding(void *ctx,int index,const RRLocalRacer *r,RRFireBinding *out){(void)ctx;CHECK(index>=0&&index<11&&r==battle.roster[index]);const Binding *f=binding_frame->bindings+index;CHECK(f->valid);if(!f->valid)return false;CHECK(rr_car_pose_binding(&tuned.pack.poses[index],f->model,f->accent,out));for(int k=0;k<16;k++)near(out->model_world[k],f->matrix[k]);binding_calls++;return true;}
static void resources(const RRRivalLifeResources *s,const Resource *f){near(s->bolt.heat,f->heat);near(s->bolt.bolt_cd,f->bolt_cd);CHECK(s->bolt.side==f->side&&s->bolt.overheated==f->overheated);near(s->missiles,f->missiles);near(s->seeker_cd,f->seeker_cd);near(s->spin_cd,f->spin_cd);near(s->emp_cd,f->emp_cd);CHECK(s->locked==(f->lock<0?NULL:battle.roster[f->lock]));near(s->lock_time,f->lock_time);}
static void metadata(const Meta *m){for(int i=0;i<11;i++)CHECK(pack.ranking[i]==m->ranking[i]);CHECK(pack.position==m->position);near(pack.rank_timer,m->rank_timer);near(pack.shake,m->shake);near(run.host.time,m->time);near(run.host.countdown,m->countdown);near(run.host.last_nitro,m->last_nitro);CHECK(run.host.last_count==m->last_count&&(int)run.host.phase==m->phase);if(player.race.finished)CHECK(pack.finish_position==m->finish_position);}
static void start(const ResetFixture *f,Random *rng){rr_car_place(&player.car,f->player.pos,f->player.yaw,0);rr_race_reset(&player.race,RR_RACE_TWO_LAPS,f->player.pos,f->player.yaw);load(&player,&f->player);player.previous=f->player.pos;player.wrong_way=0;player.is_player=true;(void)random_value(rng);(void)random_value(rng);CHECK(rr_battle_current_start(&tuned,&battle,&pack,&track,&player));CHECK(rr_run_current_begin(&run,&tuned));CHECK(rr_run_input_center(&inputs,&player.car));CHECK(rng->state==f->rng&&rng->draws==f->draws);for(int i=0;i<11;i++){actor(battle.roster[i],&f->after[i]);if(i)resources(&tuned.pack.resources[i-1],&f->after[i].resources);}metadata(&f->meta);}
static void mutate(unsigned m){
 if(m&8192){rr_car_place(&player.car,rr_v3(8,115.5,64),0,95);CHECK(rr_car_pose_sync(&tuned.pack.poses[0],&player.car));}
 if(m&16384){const RRGate *gate=track.gates;rr_car_place(&player.car,rr_sub(gate->pos,rr_scale(gate->forward,.5)),gate->yaw,45);player.race.next_checkpoint=0;player.passed=0;CHECK(rr_car_pose_sync(&tuned.pack.poses[0],&player.car));}

 if(m&2){rr_car_place(&player.car,pack.coins.list[0].pos,0,20);player.ai.sample=rr_track_nearest(&track,player.car.pos);CHECK(rr_car_pose_sync(&tuned.pack.poses[0],&player.car));}
 if(m&4)player.car.vel=rr_v3(20,0,0);
 if(m&512)player.hp=40;
 if(m&1024){player.hp=0;player.wreck=.1;player.visible=false;}
 if(m&2048){player.car.pos.y=25;player.lost=1.09;}
 if(m&256){rr_car_place(&player.car,rr_v3(0,126,.1),0,35);player.hp=100;player.wreck=0;player.visible=true;player.race.next_checkpoint=track.gate_count;player.race.lap=2;player.race.finished=false;player.passed=(UINT32_C(1)<<track.gate_count)-1;CHECK(rr_car_pose_sync(&tuned.pack.poses[0],&player.car));}

 if(m&1)for(int i=0;i<10;i++)pack.rivals[i].fire_cd=0;
 if(m&4){RRLocalRacer *r=pack.rivals;rr_car_place(&r->car,rr_add(player.car.pos,rr_v3(2,0,0)),0,0);r->car.vel=rr_v3(-5,0,0);CHECK(rr_car_pose_sync(&tuned.pack.poses[1],&r->car));}
 if(m&8)pack.rivals[0].hp=40;
 if(m&16){pack.rivals[0].hp=0;pack.rivals[0].wreck=.03;pack.rivals[0].visible=false;}
 if(m&32){pack.rivals[1].stuck=2.49;pack.rivals[1].car.speed=0;pack.rivals[1].car.vel=rr_v3(0,0,0);}
 if(m&64){pack.rivals[2].car.pos.y=25;pack.rivals[2].lost=1.09;}
 if(m&128){pack.rivals[0].hp=0;pack.rivals[0].wreck=2.4;pack.rivals[0].visible=false;}
}
typedef struct {unsigned calls,hits;} Segments;
static bool segment(void *p,RRVec3 a,RRVec3 b,RRVec3 *hit){Segments *s=p;s->calls++;bool value=rr_world_segment(&world,a,b,.6,hit);s->hits+=value;return value;}
static void foreign_hook(void *p){(void)p;}
static unsigned generic_boards,snapshot_boards;
static void result(const RRResultsCurrent *r,const ExpectedResults *e){
 near(r->total,e->total);CHECK(r->best_lap==(e->best<0?SIZE_MAX:(size_t)e->best));CHECK(r->time_count==e->time_count&&r->coin_count==e->coin_count&&r->player_time_rank==e->time_rank&&r->player_coin_rank==e->coin_rank&&r->finish_position==e->finish_position);
 for(size_t i=0;i<r->time_count&&i<e->time_count;i++){CHECK(r->times[i].racer==e->times[i].racer&&r->times[i].estimated==e->times[i].estimated);near(r->times[i].seconds,e->times[i].seconds);}
 for(size_t i=0;i<r->coin_count&&i<e->coin_count;i++)CHECK(r->coins[i].racer==e->coins[i].racer&&r->coins[i].coins==e->coins[i].coins);
}
static void generic_results(void){
 for(size_t c=0;c<COUNT(result_cases);c++){
  const ResultCase *f=result_cases+c;RRLocalRacer actors[11]={0},saved[11];const RRLocalRacer *roster[11];double laps[2]={f->laps[0],f->laps[1]};
  for(size_t i=0;i<f->count;i++){const ResultActor *s=f->actors+i;RRLocalRacer *r=actors+i;r->ai.sample=s->sample;r->race.lap=s->lap;r->race.next_checkpoint=s->next;r->race.finished=s->finished;r->race.finish_time=s->finish_time;r->coins=s->coins;r->is_player=i==f->player;roster[i]=r;}
  actors[f->player].lap_times=laps;actors[f->player].lap_count=f->lap_count;actors[f->player].lap_capacity=2;memcpy(saved,actors,sizeof(saved));
  RRResultsCurrent value,old;memset(&value,0x5a,sizeof(value));old=value;bool ok=rr_results_current_build(roster,f->count,f->player,f->samples,f->time,f->finish,(RRSessionPhase)f->phase,&value);CHECK(ok==f->expected.enabled);
  if(ok){result(&value,&f->expected);CHECK(value.lap_count==f->lap_count);for(size_t i=0;i<value.lap_count;i++)near(value.laps[i],f->laps[i]);generic_boards++;}else CHECK(!memcmp(&value,&old,sizeof(value)));
  CHECK(!memcmp(saved,actors,sizeof(saved)));near(laps[0],f->laps[0]);near(laps[1],f->laps[1]);
 }
}
int main(void){generic_results();
 Random rng={0};RRShot shots[64];RRFxLife fx[512];int trace=-1;unsigned restarts=0,respawns=0,smoke=0,contacts=0,collections=0,paused=0,countdown=0,total_draws=0;size_t max_shots=0,max_fx=0;
 for(current=0;current<COUNT(frame_fixtures);current++){
  const FrameFixture *f=&frame_fixtures[current];
  if(trace!=(int)f->trace){rr_run_current_detach(&run);rr_pack_dispose(&pack);rr_rival_dispose(&player);memset(&run,0,sizeof(run));memset(&inputs,0,sizeof(inputs));memset(&world,0,sizeof(world));memset(&pack,0,sizeof(pack));memset(&player,0,sizeof(player));memset(&battle,0,sizeof(battle));memset(&tuned,0,sizeof(tuned));tuned.binding=binding;trace=(int)f->trace;const TraceFixture *t=&trace_fixtures[trace];CHECK(rr_track_init_road(&track,t->road));if(t->road==9){CHECK(rr_world_add(&world,rr_box(rr_v3(8,118,55),rr_v3(12,10,1.2),.15)));CHECK(rr_world_add(&world,rr_cylinder(-8,45,4,100,135)));CHECK(rr_world_add(&world,rr_torus(rr_v3(0,126,10),0,12,1.4)));}run.host.time=17.25;pack.rank_timer=.13;pack.shake=.1;for(int i=0;i<10;i++){rr_car_reset(&pack.rivals[i].car,rr_v3(0,118,0),0,0);pack.rivals[i].ai=(RRAIDriver){0,t->drivers[i][0],t->drivers[i][1],t->drivers[i][2],t->drivers[i][3],t->drivers[i][4]};}rng=(Random){t->seed,0};battle.fire.shots=shots;battle.fire.capacity=64;battle.fire.fx=(RRFxLedger){fx,0,512,false,true};battle.fire.random=random_value;battle.fire.random_ctx=&rng;battle.fire.cue=cue;battle.fire.cue_ctx=&rng;rr_arsenal_init(&battle.arsenal);rr_lock_init(&battle.lock);start(&t->start,&rng);}
  if(f->restart){start(&f->reset,&rng);restarts++;}
  if(f->entered==RR_SESSION_PAUSED&&run.host.phase!=RR_SESSION_PAUSED)CHECK(rr_run_current_pause(&run));
  if(f->entered!=RR_SESSION_PAUSED&&run.host.phase==RR_SESSION_PAUSED)CHECK(rr_run_current_resume(&run));
  CHECK((int)run.host.phase==f->entered);mutate(f->mask);actor(&player,&f->before);
  RRRunEvents tick;Segments segments={0};bool use_default=current%17==0;binding_frame=f;
  CHECK(rr_run_input_tick(&inputs,&run,f->raw_input,f->settings,f->raw,&world,use_default?NULL:segment,&segments,&tick));RRPackEvents e=tick.pack;
  RRInput actual=run.host.current_input.control,expected=f->control;near(actual.throttle,expected.throttle);near(actual.brake,expected.brake);near(actual.steer,expected.steer);near(actual.climb,expected.climb);near(actual.strafe,expected.strafe);CHECK(actual.boost==expected.boost&&actual.aim_steer==expected.aim_steer);CHECK(run.host.current_input.weapons.fire==f->fire);near(run.host.current_input.weapons.steer,f->steer);vector(run.host.current_input.aim,f->aim);
  near(inputs.aim.yaw,f->expected_aim.yaw);near(inputs.aim.pitch,f->expected_aim.pitch);near(inputs.aim.look_yaw,f->expected_aim.look_yaw);near(inputs.aim.look_pitch,f->expected_aim.look_pitch);CHECK(inputs.aim.looking==f->expected_aim.looking&&inputs.mouse_camera==f->mouse_camera);vector(inputs.aim.direction,f->expected_aim.direction);

  CHECK(tick.player==f->player_events);near(run.host.race_time,f->race_time);near(tick.motion.impact,f->motion.impact);CHECK(tick.motion.collided==f->motion.collided);if(f->motion.collided)vector(tick.motion.normal,f->motion.normal);
  if(!use_default){CHECK(segments.calls==f->segment_calls&&segments.hits==f->segment_hits);world_hits+=segments.hits;}else default_frames++;
  go+=(tick.player&RR_RUN_GO)!=0;impacts+=(tick.player&RR_RUN_IMPACT)!=0;nitro+=(tick.player&RR_RUN_NITRO)!=0;blocked_manual+=f->manual&&!f->manual_allowed;
  player_respawns+=(tick.player&RR_RUN_RESPAWN)!=0;player_smoke+=(tick.player&RR_RUN_SMOKE)!=0;player_finishes+=(tick.player&RR_RACE_FINISH)!=0;manual_resets+=(tick.player&RR_RUN_MANUAL)!=0;
  CHECK(!e.error&&!battle.fire.error&&!battle.fire.fx.error&&rng.state==f->rng&&rng.draws==f->draws);
  RRResultsCurrent view,old_view;memset(&view,0x5a,sizeof(view));old_view=view;const ExpectedResults *expected_results=run_results_expected+current;bool shown=rr_run_results_build(&run,&view);CHECK(shown==expected_results->enabled);if(shown){result(&view,expected_results);CHECK(view.lap_count==player.lap_count);for(size_t k=0;k<view.lap_count;k++)near(view.laps[k],player.lap_times[k]);snapshot_boards++;}else CHECK(!memcmp(&view,&old_view,sizeof(view)));
  for(int i=0;i<11;i++){actor(battle.roster[i],&f->after[i]);if(i)resources(&tuned.pack.resources[i-1],&f->after[i].resources);}metadata(&f->meta);near(player.wrong_way,f->player_more.wrong);vector(player.previous,f->player_more.previous);vector(player.race.safe_position,f->player_more.safe);near(player.race.safe_yaw,f->player_more.yaw);CHECK(player.lap_count==f->player_more.count);for(size_t k=0;k<player.lap_count&&k<8;k++)near(player.lap_times[k],f->player_more.history[k]);
  near(battle.arsenal.heat,f->arsenal.heat);near(battle.arsenal.bolt_cd,f->arsenal.bolt_cd);near(battle.arsenal.missiles,f->arsenal.missiles);near(battle.arsenal.spin_cd,f->arsenal.spin_cd);near(battle.arsenal.emp_cd,f->arsenal.emp_cd);CHECK(battle.arsenal.overheated==f->arsenal.overheated&&battle.arsenal.side==f->arsenal.side);CHECK((battle.lock.target<0?-1:battle.lock.target+1)==f->lock);near(battle.lock.charge,f->charge);
  CHECK(battle.fire.count==f->shot_count&&battle.fire.fx.count==f->fx_count);for(size_t i=0;i<battle.fire.count&&i<f->shot_count;i++)shot(shots+i,f->shots+i,battle.roster);for(size_t i=0;i<battle.fire.fx.count&&i<f->fx_count;i++){near(fx[i].t,f->fx[i].t);near(fx[i].life,f->fx[i].life);}
  CHECK(pack.coins.count==f->coin_count&&e.coins.count==f->collection_count);for(int i=0;i<pack.coins.count;i++){CHECK(pack.coins.list[i].active==f->coins[i].active);near(pack.coins.list[i].timer,f->coins[i].timer);near(pack.coins.list[i].pop,f->coins[i].pop);}for(int i=0;i<e.coins.count&&i<f->collection_count;i++)CHECK(e.coins.list[i].coin==f->collections[i][0]&&e.coins.list[i].racer==f->collections[i][1]);
  for(int i=0;i<10;i++){respawns+=(e.rival[i]&RR_RIVAL_RESPAWN)!=0;smoke+=(e.rival[i]&RR_RIVAL_SMOKE)!=0;}contacts+=(unsigned)e.contacts.count;collections+=(unsigned)e.coins.count;paused+=f->branch==RR_PACK_PAUSED;countdown+=f->branch==RR_PACK_COUNTDOWN;if(battle.fire.count>max_shots)max_shots=battle.fire.count;if(battle.fire.fx.count>max_fx)max_fx=battle.fire.fx.count;
  if(current+1==COUNT(frame_fixtures)||frame_fixtures[current+1].trace!=f->trace)total_draws+=rng.draws;
 }
 RRRunInputState saved_inputs=inputs;RRCar saved=player.car;RRCarPose pose=tuned.pack.poses[0];double before_time=run.host.time;unsigned before_draws=rng.draws;RRRunEvents empty;
 double invalid[]={0,-1,NAN,INFINITY};for(int i=0;i<4;i++){CHECK(rr_run_input_tick(&inputs,&run,(RRRunInputFrame){0},(RRMouseSettings){1,false},invalid[i],&world,NULL,NULL,&empty));CHECK(!empty.player&&!memcmp(&saved,&player.car,sizeof(saved))&&!memcmp(&pose,&tuned.pack.poses[0],sizeof(pose)));near(run.host.time,before_time);CHECK(rng.draws==before_draws&&!memcmp(&saved_inputs,&inputs,sizeof(inputs)));}
 RRRunCurrent other={0};CHECK(!rr_run_current_begin(&other,&tuned));battle.after_weapons=foreign_hook;CHECK(!rr_run_current_tick(&run,(RRRunInput){0},.02,&world,NULL,NULL,&empty));rr_run_current_detach(&run);CHECK(battle.after_weapons==foreign_hook);battle.after_weapons=NULL;battle.after_weapons_context=NULL;CHECK(rr_run_current_begin(&other,&tuned));rr_run_current_detach(&other);
 rr_pack_dispose(&pack);rr_rival_dispose(&player);
 printf("RUN_RESULTS_SUMMARY roads=%zu frames=%zu restarts=%u respawns=%u smoke=%u contacts=%u collections=%u paused=%u countdown=%u sequence_draws=%u max_shots=%zu max_fx=%zu max_error=%.17g failures=%u\n",COUNT(trace_fixtures),COUNT(frame_fixtures),restarts,respawns,smoke,contacts,collections,paused,countdown,total_draws,max_shots,max_fx,worst,failures);printf("RUN_RESULTS_DETAIL bindings=%u player_respawns=%u player_smoke=%u player_finishes=%u manual_resets=%u go=%u impacts=%u nitro=%u observed_world_hits=%u default_frames=%u blocked_manual=%u host_bytes=%zu input_bytes=%zu production_inactive=true\n",binding_calls,player_respawns,player_smoke,player_finishes,manual_resets,go,impacts,nitro,world_hits,default_frames,blocked_manual,sizeof(run),sizeof(inputs));printf("RESULTS_PROJECTION_DETAIL cases=%zu generic_shown=%u full_pack_shown=%u board_bytes=%zu no_rewards_or_render=true\n",COUNT(result_cases),generic_boards,snapshot_boards,sizeof(RRResultsCurrent));return failures?EXIT_FAILURE:EXIT_SUCCESS;
}

