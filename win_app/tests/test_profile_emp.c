#include "rr_weapon_current.h"
#include "rr_battle_current.h"
#include <stdio.h>
#include <string.h>
#include "profile_emp_reference.h"
static unsigned failures,checks;static double worst;static size_t at;
#define CHECK(x) do{checks++;if(!(x)){if(failures<12)fprintf(stderr,"profile EMP case%zu line%d: %s\n",at,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-9);}
static void compare(const RRLocalRacer *r,const EmpActor *a){double v[]={r->hp,r->car.speed,r->car.stun,r->fire_cd,r->wreck,r->car.roll_time,r->car.surge,r->car.roll_direction,r->car.pos.x,r->car.pos.y,r->car.pos.z,r->car.vel.x,r->car.vel.y,r->car.vel.z};for(int k=0;k<14;k++)near(v[k],a->v[k]);CHECK((int)r->car.profile==a->profile&&r->is_player==a->player&&r->race.finished==a->finished&&r->visible==a->visible);}
static void apply(RRLocalRacer *r,const EmpActor *a){memset(r,0,sizeof(*r));r->car.profile=(RRCarProfile)a->profile;r->is_player=a->player;r->race.finished=a->finished;r->visible=a->visible;const double *v=a->v;r->hp=v[0];r->car.speed=v[1];r->car.stun=v[2];r->fire_cd=v[3];r->wreck=v[4];r->car.roll_time=v[5];r->car.surge=v[6];r->car.roll_direction=(int)v[7];r->car.pos=rr_v3(v[8],v[9],v[10]);r->car.vel=rr_v3(v[11],v[12],v[13]);r->car.fwd=rr_v3(0,0,-1);r->car.max_multiplier=1;}
static void arsenal_load(RRArsenal *s,const double v[7]){s->heat=v[0];s->bolt_cd=v[1];s->missiles=v[2];s->spin_cd=v[3];s->emp_cd=v[4];s->overheated=v[5]!=0;s->side=(int)v[6];}
static void arsenal_compare(const RRArsenal *s,const double v[7]){double actual[]={s->heat,s->bolt_cd,s->missiles,s->spin_cd,s->emp_cd,s->overheated?1:0,s->side};for(int i=0;i<7;i++)near(actual[i],v[i]);}
static void life_load(RRRivalLifeResources *s,const double v[7]){memset(s,0,sizeof(*s));s->bolt.heat=v[0];s->bolt.bolt_cd=v[1];s->missiles=v[2];s->spin_cd=v[3];s->emp_cd=v[4];s->bolt.overheated=v[5]!=0;s->bolt.side=(int)v[6];s->seeker_cd=1;}
static void life_compare(const RRRivalLifeResources *s,const double v[7]){double actual[]={s->bolt.heat,s->bolt.bolt_cd,s->missiles,s->spin_cd,s->emp_cd,s->bolt.overheated?1:0,s->bolt.side};for(int i=0;i<7;i++)near(actual[i],v[i]);}
typedef struct {const EmpCase *c;RRFireLocal *local;RRLocalRacer *actors;unsigned draws,cues,damage_at,purge_at;} Harness;
static double random_unit(void *ctx){Harness *h=ctx;h->draws++;return .25;}
static void remaining(const RRFireLocal *l,const EmpRemaining *shots,size_t count){CHECK(l->count==count);for(size_t i=0;i<l->count&&i<count;i++)CHECK(l->shots[i].serial==shots[i].id&&l->shots[i].dodge_tried==shots[i].tried);}
static void cue_check(Harness *h){h->cues++;remaining(h->local,h->c->cue_shots,h->c->cue_shots_count);for(int i=0;i<3;i++)compare(h->actors+i,h->c->cue_actors+i);CHECK(h->local->fx.count==1+h->c->purged_count);}
static void player_cue(void *ctx,unsigned kind){if(kind==RR_ARSENAL_EMP)cue_check(ctx);else CHECK(kind==RR_ARSENAL_DENY);}
static void rival_cue(void *ctx,unsigned kind,RRLocalRacer *owner,RRLocalRacer *target){Harness *h=ctx;(void)owner;if(kind==RR_RIVAL_CUE_EMP)cue_check(h);else if(kind==RR_RIVAL_NOTICE_EMP){CHECK(target==h->actors+1);compare(target,h->c->after+1);}else CHECK(false);}
static void damage_notice(void *ctx,RRLocalRacer *r,unsigned result){Harness *h=ctx;(void)result;CHECK(h->damage_at<h->c->damage_count);if(h->damage_at<h->c->damage_count)CHECK(r-h->actors==h->c->damage_order[h->damage_at]);h->damage_at++;}
static void purge_notice(void *ctx,unsigned event,const RRShot *s,RRLocalRacer *r,RRVec3 point){Harness *h=ctx;(void)r;(void)point;CHECK(event==RR_SHOT_PURGED&&h->purge_at<h->c->purged_count);if(h->purge_at<h->c->purged_count)CHECK(s->serial==h->c->purged[h->purge_at]);h->purge_at++;}
static void make_shots(RRShot shots[7],const EmpCase *c,RRLocalRacer actors[3]){for(int i=0;i<7;i++){shots[i]=(RRShot){0};shots[i].serial=c->shots_before[i].id;shots[i].owner=actors+c->shots_before[i].owner;shots[i].pos=rr_v3(c->shots_before[i].x,0,0);shots[i].kind=c->shots_before[i].missile?RR_SHOT_MISSILE:RR_SHOT_GENERIC;shots[i].target=c->shots_before[i].target?actors:NULL;shots[i].life=10;}}
static double fixed_random(void *ctx){(void)ctx;return .25;}
static bool binding(void *ctx,int index,const RRLocalRacer *r,RRFireBinding *b){(void)ctx;(void)index;(void)r;memset(b,0,sizeof(*b));b->model_world[0]=b->model_world[5]=b->model_world[10]=b->model_world[15]=1;return true;}
static RRTrack track;static RRLocalPack pack;static RRLocalBattle battle;static RRBattleCurrent current;static RRLocalRacer player;
int main(void){
 unsigned casts=0,purges=0,full_calls=0,inactive_player=0;
 for(at=0;at<sizeof(emp_cases)/sizeof(*emp_cases);at++){
 const EmpCase *c=emp_cases+at;RRLocalRacer actors[3];for(int i=0;i<3;i++)apply(actors+i,c->before+i);RRLocalRacer *roster[]={actors,actors+1,actors+2};Harness h={0};h.c=c;h.actors=actors;
 if(c->mode==0&&c->wave>0){size_t count=rr_combat_emp_current(actors[0].car.pos,c->wave,roster,3,c->flag!=4,damage_notice,&h);CHECK(count==c->damage_count&&h.damage_at==c->damage_count);for(int i=0;i<3;i++)compare(actors+i,c->after+i);for(int i=0;i<3;i++)apply(actors+i,c->before+i);}
 if(c->wave>0){RRShot shots[7];make_shots(shots,c,actors);size_t count=7;CHECK(rr_shots_purge(shots,&count,actors[0].car.pos,c->wave,actors,purge_notice,&h)==c->purged_count);CHECK(h.purge_at==c->purged_count);}
 // Raw playerWeapons outside race is a source diagnostic, not a valid fire-host
 // precondition. Its inactive damage/push is covered by the direct helper above.
 if(c->mode==0&&c->flag==4){inactive_player++;continue;}
 RRShot shots[7];make_shots(shots,c,actors);RRFxLife fx[128];double shake=.2;RRFireLocal l={0};l.player=c->mode?actors+1:actors;l.racers=roster;l.racer_count=3;l.shots=shots;l.count=l.capacity=7;l.fx=(RRFxLedger){fx,0,128,false,false};l.random=random_unit;l.random_ctx=&h;l.cue=player_cue;l.cue_ctx=&h;l.shake=&shake;l.aim=rr_v3(0,0,-1);h.local=&l;RRFireBinding bind;binding(NULL,0,actors,&bind);
 if(c->mode==0){RRArsenal s;arsenal_load(&s,c->resources_before);RRArsenalEvents e;CHECK(rr_fire_player_v217_step(&l,&s,.01,(RRArsenalInput){.emp_hit=true},&bind,&e));arsenal_compare(&s,c->resources_after);}
 else{RRRivalLifeResources s;life_load(&s,c->resources_before);CHECK(rr_rival_arsenal_v217_step(&l,actors,&s,.01,7,c->flag!=4,1,&bind,rival_cue,&h));life_compare(&s,c->resources_after);near(s.seeker_cd,c->flag==4?1:.99);}
 for(int i=0;i<3;i++)compare(actors+i,c->after+i);remaining(&l,c->shots_after,c->shots_count);CHECK(!l.error&&!l.fx.error&&h.cues==c->cues);CHECK(l.fx.count==(c->wave>0?1:0)+c->purged_count);CHECK(h.draws==c->draws+2*(unsigned)l.fx.count);casts+=c->wave>0?1:0;purges+=(unsigned)c->purged_count;full_calls++;
 }
 // Current full-pack player routing: Titan reaches targets outside Roadster95m;
 // a rolling Viper is shoved/held but takes no damage, Titan victim uses .4/.6.
 CHECK(rr_track_init_road(&track,1));RRShot shots[64];RRFxLife fx[256];battle.fire.shots=shots;battle.fire.capacity=64;battle.fire.fx=(RRFxLedger){fx,0,256,false,false};battle.fire.random=fixed_random;player.is_player=true;rr_car_reset(&player.car,rr_v3(0,400,0),0,0);player.car.profile=RR_PROFILE_TITAN;player.hp=140;rr_race_reset(&player.race,RR_RACE_TWO_LAPS,player.car.pos,0);CHECK(rr_battle_current_start(&current,&battle,&pack,&track,&player));current.binding=binding;
 for(int i=0;i<RR_AI_RIVALS;i++){pack.rivals[i].car.profile=RR_PROFILE_ROADSTER;pack.rivals[i].hp=100;pack.rivals[i].car.pos=rr_v3(2000+i*20,400,0);pack.rivals[i].react=100;pack.rivals[i].fire_cd=100;}pack.rivals[0].car.pos=rr_v3(100,400,0);pack.rivals[0].car.profile=RR_PROFILE_VIPER;pack.rivals[0].hp=75;pack.rivals[0].car.roll_time=.2;pack.rivals[1].car.pos=rr_v3(110,400,0);pack.rivals[1].car.profile=RR_PROFILE_TITAN;pack.rivals[1].hp=140;RRPackEvents events;CHECK(rr_battle_current_after_player(&current,RR_PACK_PLAYING,true,.01,1,0,NULL,NULL,NULL,(RRArsenalInput){.emp_hit=true},&events));near(pack.rivals[0].hp,75);CHECK(pack.rivals[0].car.vel.x>0);near(pack.rivals[1].hp,140-25*(1-.5*110/118.75));near(pack.rivals[1].car.stun,.96);CHECK(!pack.coin_pending&&!current.error);rr_pack_dispose(&pack);
 // Current full-pack rival routing: normal Roadster issuer hits a Titan player;
 // the player's already-completed movement does not consume the new push/stun.
 memset(&player,0,sizeof(player));player.is_player=true;rr_car_reset(&player.car,rr_v3(80,400,0),0,0);player.car.profile=RR_PROFILE_TITAN;player.hp=140;rr_race_reset(&player.race,RR_RACE_TWO_LAPS,player.car.pos,0);CHECK(rr_battle_current_start(&current,&battle,&pack,&track,&player));current.binding=binding;
 for(int i=0;i<RR_AI_RIVALS;i++){pack.rivals[i].car.profile=RR_PROFILE_ROADSTER;pack.rivals[i].hp=100;pack.rivals[i].car.pos=rr_v3(2000+i*20,400,0);pack.rivals[i].car.vel=rr_v3(0,0,0);pack.rivals[i].car.speed=0;pack.rivals[i].react=100;pack.rivals[i].fire_cd=100;}pack.rivals[0].car.pos=rr_v3(0,400,0);pack.rivals[0].fire_cd=0;pack.rivals[1].car.pos=rr_v3(90,400,0);CHECK(rr_battle_current_after_player(&current,RR_PACK_PLAYING,true,.01,1,7,NULL,NULL,NULL,(RRArsenalInput){0},&events));near(player.hp,140-25*(1-.5*80/95));near(player.car.stun,.96);near(player.car.vel.x,(30*(1-80./95)+8)*.4);near(current.pack.resources[0].emp_cd,11);CHECK(!pack.coin_pending&&!current.error);rr_pack_dispose(&pack);
 printf("PROFILE_EMP_SUMMARY cases=%zu full_weapon_calls=%u raw_inactive_player=%u casts=%u purges=%u checks=%u max_error=%.17g failures=%u; source numeric/order oracle, inert FX/nonlethal, current full-pack player+rival routed, production/graphics/rewards inactive\n",sizeof(emp_cases)/sizeof(*emp_cases),full_calls,inactive_player,casts,purges,checks,worst,failures);return failures?1:0;
}
