#include "rr_player_phase.h"
#include <stdio.h>
#include <string.h>
#include "profile_health_reference.h"
static unsigned failures,checks;static double worst;static size_t at;
#define CHECK(x) do{checks++;if(!(x)){if(failures<12)fprintf(stderr,"profile health case%zu line%d: %s\n",at,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-10);}
static void apply(RRLocalRacer *r,const HealthState *s,int profile){memset(r,0,sizeof(*r));const double *v=s->v;r->hp=v[0];r->wreck=v[1];r->smoke_time=v[2];r->lost=v[3];r->stuck=v[4];r->wrong_way=v[5];r->fire_cd=v[6];r->react=v[7];r->car.boost=v[8];r->car.t=v[9];r->car.nitro=v[10];r->car.kick=v[11];r->car.sputter=v[12];r->car.thrust=v[13];r->car.speed=v[14];r->car.side=v[15];r->car.stun=v[16];r->car.roll_time=v[17];r->car.surge=v[18];r->car.yaw=v[19];r->car.pitch=v[20];r->car.roll=v[21];r->car.pos=rr_v3(v[22],v[23],v[24]);r->car.vel=rr_v3(v[25],v[26],v[27]);r->car.fwd=rr_v3(v[28],v[29],v[30]);r->previous=rr_v3(v[31],v[32],v[33]);r->visible=s->visible;r->car.profile=(RRCarProfile)profile;r->race.mode=RR_RACE_TWO_LAPS;r->car.max_multiplier=1;}
static void compare(const RRLocalRacer *r,const HealthState *s){const RRCar *c=&r->car;double v[]={r->hp,r->wreck,r->smoke_time,r->lost,r->stuck,r->wrong_way,r->fire_cd,r->react,c->boost,c->t,c->nitro,c->kick,c->sputter,c->thrust,c->speed,c->side,c->stun,c->roll_time,c->surge,c->yaw,c->pitch,c->roll,c->pos.x,c->pos.y,c->pos.z,c->vel.x,c->vel.y,c->vel.z,c->fwd.x,c->fwd.y,c->fwd.z,r->previous.x,r->previous.y,r->previous.z};for(int k=0;k<34;k++)near(v[k],s->v[k]);CHECK(r->visible==s->visible);}
static double random_unit(void *ctx){(void)ctx;return .5;}
static unsigned random_pairs;
static bool random_pair(void *ctx,double values[2]){(void)ctx;random_pairs++;values[0]=.2;values[1]=.8;return true;}
static RRTrack one,track;static RRLocalPack pack;static RRLocalBattle battle;static RRBattleCurrent current;static RRLocalRacer player;
int main(void){
 one.sample_count=1;one.samples[0]=rr_v3(8,115.5,60);one.tangents[0]=rr_v3(0,0,-1);
 for(at=0;at<sizeof(reset_cases)/sizeof(*reset_cases);at++){
 const ResetCase *c=&reset_cases[at];RRLocalRacer r;apply(&r,&c->before,c->profile);RRRivalLifeResources resource={0};resource.bolt.side=-1;CHECK(rr_rival_lifecycle_reset(&r,&resource,&one,c->pos,c->yaw,RR_RACE_TWO_LAPS,c->units[0],c->units[1]));compare(&r,&c->after);near(resource.bolt.bolt_cd,c->bolt);near(resource.missiles,c->missiles);CHECK(resource.bolt.side==-1&&r.ai.sample==0&&(int)r.car.profile==c->profile);near(r.hp,rr_racer_max_hp(&r));
 }
 CHECK(rr_track_init_road(&track,1));player.is_player=true;rr_car_reset(&player.car,rr_v3(8,115.5,60),0,0);rr_race_reset(&player.race,RR_RACE_TWO_LAPS,player.car.pos,0);RRShot shots[64];RRFxLife fx[128];battle.fire.shots=shots;battle.fire.capacity=64;battle.fire.fx=(RRFxLedger){fx,0,128,false,false};battle.fire.random=random_unit;CHECK(rr_battle_current_start(&current,&battle,&pack,&track,&player));
 unsigned player_respawns=0,rival_respawns=0;
 for(at=0;at<sizeof(respawn_cases)/sizeof(*respawn_cases);at++){
 const RespawnCase *c=&respawn_cases[at];
 if(c->player){apply(&player,&c->before,c->profile);player.is_player=true;player.race.safe_position=c->pos;player.race.safe_yaw=c->yaw;CHECK(rr_player_phase_respawn(&current));compare(&player,&c->after);CHECK((int)player.car.profile==c->profile&&current.pack.poses[0].ready&&c->draws==0);player_respawns++;}
 else{RRLocalRacer r;apply(&r,&c->before,c->profile);r.race.safe_position=c->pos;r.race.safe_yaw=c->yaw;CHECK(rr_rival_respawn_current(&r,&one,c->keep,c->units[0],c->units[1]));compare(&r,&c->after);CHECK((int)r.car.profile==c->profile&&r.ai.sample==0&&c->draws==2);rival_respawns++;}
 }
 RRLocalRacer smoker={0};unsigned emits=0;
 for(at=0;at<sizeof(smoke_cases)/sizeof(*smoke_cases);at++){
 const SmokeCase *c=&smoke_cases[at];if(c->start){memset(&smoker,0,sizeof(smoker));smoker.car.profile=(RRCarProfile)c->profile;smoker.smoke_time=c->before;}near(smoker.smoke_time,c->before);smoker.hp=c->hp;double strength=-999;CHECK(rr_racer_smoke_current(&smoker,c->dt,&strength)==c->emit);near(smoker.smoke_time,c->after);if(c->emit){near(strength,c->strength);emits++;}else near(strength,-999);
 }
 // Current runtime recovery: expire wreck and stuck-rule callbacks heal profile,
 // only consume a random pair on recovery, and preserve unrelated resources.
 for(int profile=0;profile<3;profile++){
 RRLocalRacer r={0};r.car.max_multiplier=1;r.car.profile=(RRCarProfile)profile;r.race.mode=RR_RACE_TWO_LAPS;r.race.safe_position=one.samples[0];r.car.pos=one.samples[0];r.wreck=.005;r.hp=0;r.smoke_time=.09;r.fire_cd=5;r.car.boost=.37;unsigned before=random_pairs;
 unsigned e=rr_rival_step_current_random(&r,&one,NULL,(RRInput){0},.01,0,34,random_pair,NULL);CHECK((e&RR_RIVAL_RESPAWN)&&!(e&RR_RIVAL_SMOKE)&&random_pairs==before+1);near(r.hp,rr_racer_max_hp(&r));near(r.smoke_time,.09);near(r.fire_cd,5);near(r.car.boost,.37);
 before=random_pairs;(void)rr_rival_step_current_random(&r,&one,NULL,(RRInput){0},.01,0,34,random_pair,NULL);CHECK(random_pairs==before);
 r.hp=1;r.stuck=2.499;r.car.speed=0;r.car.pos=one.samples[0];r.previous=r.car.pos;r.race.safe_position=one.samples[0];e=rr_rival_rules_current(&r,&one,.01,4,34,.2,.8);CHECK(e&RR_RIVAL_RESPAWN);near(r.hp,rr_racer_max_hp(&r));
 // Real current player phase: wreck expiry and source fractional smoke event.
 player.car.profile=(RRCarProfile)profile;player.race.mode=RR_RACE_TWO_LAPS;player.race.safe_position=track.samples[0];player.race.safe_yaw=0;player.wreck=.005;player.hp=0;RRCarStepReport motion;unsigned flags;CHECK(rr_player_phase_step(&current,(RRInput){0},.01,0,NULL,&motion,&flags)&&!!(flags&RR_RUN_RESPAWN));near(player.hp,rr_racer_max_hp(&player));CHECK(!(flags&RR_RUN_SMOKE));
 player.hp=.3*rr_racer_max_hp(&player);player.smoke_time=0;size_t before_fx=battle.fire.fx.count;CHECK(rr_player_phase_step(&current,(RRInput){0},.01,0,NULL,&motion,&flags)&&!!(flags&RR_RUN_SMOKE));near(player.smoke_time,.1);CHECK(battle.fire.fx.count==before_fx+1);
 }
 RRLocalRacer legacy={0};legacy.car.profile=RR_PROFILE_TITAN;legacy.race.mode=RR_RACE_TWO_LAPS;legacy.race.safe_position=one.samples[0];CHECK(rr_rival_respawn(&legacy,&one,false,.5,.5));near(legacy.hp,100);legacy.hp=70;legacy.smoke_time=0;legacy.car.pos=one.samples[0];unsigned e=rr_rival_step_random(&legacy,&one,NULL,(RRInput){0},.01,0,34,random_pair,NULL);CHECK(!(e&RR_RIVAL_SMOKE));
 RRLocalRacer saved=legacy;CHECK(!rr_rival_respawn_current(&legacy,&one,false,-.1,.5)&&!memcmp(&legacy,&saved,sizeof(legacy)));
 rr_pack_dispose(&pack);
 printf("PROFILE_HEALTH_SUMMARY resets=%zu player_respawns=%u rival_respawns=%u smoke_steps=%zu emissions=%u runtime_random_pairs=%u checks=%u max_error=%.17g failures=%u; current recovery/smoke routed, production inactive,no graphics/account/progression\n",sizeof(reset_cases)/sizeof(*reset_cases),player_respawns,rival_respawns,sizeof(smoke_cases)/sizeof(*smoke_cases),emits,random_pairs,checks,worst,failures);return failures?1:0;
}
