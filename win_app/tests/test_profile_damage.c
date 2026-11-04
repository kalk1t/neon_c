#include "rr_battle_current.h"
#include <stdio.h>
#include <string.h>
#include "profile_damage_reference.h"
static unsigned failures,checks;static size_t at;static double worst;
#define CHECK(x) do{checks++;if(!(x)){if(failures<12)fprintf(stderr,"profile damage case%zu line%d: %s\n",at,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-10);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static void compare(const RRLocalRacer *r,const DamageState *s){near(r->hp,s->hp);near(r->car.speed,s->speed);near(r->car.stun,s->stun);vector(r->car.vel,s->vel);near(r->wreck,s->wreck);CHECK(r->visible==s->visible);}
static RRShotKind kind(int id){return id==4?(RRShotKind)999:(RRShotKind)id;}
static void apply(RRLocalRacer *r,const DamageCase *c){memset(r,0,sizeof(*r));r->car.profile=(RRCarProfile)c->profile;r->car.speed=c->before.speed;r->car.stun=c->before.stun;r->car.vel=c->before.vel;r->hp=c->before.hp;r->wreck=c->before.wreck;r->visible=c->before.visible;r->is_player=c->player;r->race.finished=c->guard==2;r->car.roll_time=c->guard==4?.2:0;r->car.pos=rr_v3(0,400,0);}
static double random_unit(void *ctx){(void)ctx;return .5;}
static bool binding(void *ctx,int index,const RRLocalRacer *r,RRFireBinding *b){(void)ctx;(void)index;(void)r;memset(b,0,sizeof(*b));b->model_world[0]=b->model_world[5]=b->model_world[10]=b->model_world[15]=1;return true;}
static RRTrack track;static RRLocalPack pack;static RRBattleCurrent current;static RRLocalBattle battle;static RRLocalRacer player;
int main(void){
 unsigned applied=0,dodged=0,wrecked=0,blocked=0;
 for(at=0;at<sizeof(damage_cases)/sizeof(*damage_cases);at++){
 const DamageCase *c=&damage_cases[at];RRLocalRacer r;apply(&r,c);unsigned result=rr_combat_damage_current(&r,c->amount,kind(c->kind),c->guard!=1);CHECK(result==c->result);compare(&r,&c->after);CHECK((int)r.car.profile==c->profile&&r.is_player==c->player&&r.race.finished==(c->guard==2));near(r.car.roll_time,c->guard==4?.2:0);
 applied+=(result&RR_DAMAGE_APPLIED)?1:0;dodged+=(result&RR_DAMAGE_DODGED)?1:0;wrecked+=(result&RR_DAMAGE_WRECK)?1:0;blocked+=result==0?1:0;
 }
 // Projectile adapter: compare original-damage states at a known impact seam.
 // Reverse shot/world/trail ordering remains independently source-qualified.
 unsigned impacts=0;
 for(at=0;at<sizeof(damage_cases)/sizeof(*damage_cases);at++){
 const DamageCase *c=&damage_cases[at];if(c->before.speed!=135||c->before.stun!=0||c->amount!=21||c->before.hp<70||c->kind==4)continue;
 RRLocalRacer r,owner={0};apply(&r,c);owner.car.pos=rr_v3(0,400,20);RRLocalRacer *roster[]={&owner,&r};RRShot shot={0};shot.owner=&owner;shot.pos=r.car.pos;shot.radius=3.6;shot.life=1;shot.kind=kind(c->kind);shot.damage=c->amount;shot.has_damage=true;
 RRFxLife fx[64];double shake=.2;RRFireLocal l={0};l.player=&owner;l.racers=roster;l.racer_count=2;l.shots=&shot;l.count=l.capacity=1;l.fx=(RRFxLedger){fx,0,64,false,false};l.random=random_unit;l.locked=&r;l.shake=&shake;
 rr_fire_projectiles_v217_tick(&l,.01,24,NULL,NULL,c->guard!=1);CHECK(!l.error&&!l.fx.error);compare(&r,&c->after);CHECK(l.count==(c->guard==2||c->guard==3?1u:0u));if(c->result&RR_DAMAGE_WRECK)CHECK(l.locked==NULL);impacts++;
 }
 // Exact HP-boundary kills clear lock and keep the existing13-FX impact tail.
 for(int profile=0;profile<3;profile++){
 RRLocalRacer r={0},owner={0};r.car.profile=(RRCarProfile)profile;r.hp=21;r.visible=true;r.car.pos=rr_v3(0,400,0);r.car.speed=135;r.car.vel=rr_v3(0,0,-135);owner.car.pos=rr_v3(0,400,20);RRLocalRacer *roster[]={&owner,&r};RRShot shot={0};shot.owner=&owner;shot.pos=r.car.pos;shot.radius=3.6;shot.life=1;shot.kind=RR_SHOT_BOLT;shot.damage=21;shot.has_damage=true;RRFxLife fx[64];RRFireLocal l={0};l.racers=roster;l.racer_count=2;l.shots=&shot;l.count=l.capacity=1;l.fx=(RRFxLedger){fx,0,64,false,false};l.random=random_unit;l.locked=&r;
 rr_fire_projectiles_v217_tick(&l,.01,24,NULL,NULL,true);CHECK(!l.error&&!l.fx.error&&l.count==0&&l.fx.count==13&&r.hp==0&&r.wreck==2.4&&!r.visible&&l.locked==NULL);vector(r.car.vel,rr_v3(0,0,0));near(r.car.speed,0);
 }
 // Legacy damage remains profile-independent for frozen older callers.
 RRLocalRacer legacy={0};legacy.car.profile=RR_PROFILE_VIPER;legacy.hp=75;legacy.car.speed=135;CHECK(rr_combat_damage(&legacy,21,RR_SHOT_BOLT,true)==RR_DAMAGE_APPLIED);near(legacy.car.speed,135*.985);near(legacy.car.stun,.12);
 // Current full-pack after-player path must route to profile-aware projectiles.
 CHECK(rr_track_init_road(&track,1));player.is_player=true;rr_car_reset(&player.car,rr_v3(0,400,0),0,135);rr_car_use_stats(&player.car,"viper");player.hp=75;player.visible=true;rr_race_reset(&player.race,RR_RACE_TWO_LAPS,player.car.pos,0);
 RRShot shots[64];RRFxLife fx[128];battle.fire.shots=shots;battle.fire.capacity=64;battle.fire.fx=(RRFxLedger){fx,0,128,false,false};battle.fire.random=random_unit;CHECK(rr_battle_current_start(&current,&battle,&pack,&track,&player));current.binding=binding;
 shots[0]=(RRShot){0};shots[0].owner=&pack.rivals[0];shots[0].pos=player.car.pos;shots[0].radius=3.6;shots[0].life=1;shots[0].kind=RR_SHOT_BOLT;shots[0].damage=21;shots[0].has_damage=true;battle.fire.count=1;RRPackEvents events;
 CHECK(rr_battle_current_after_player(&current,RR_PACK_PLAYING,true,.01,1,0,NULL,NULL,NULL,(RRArsenalInput){0},&events));CHECK(!current.error&&!pack.coin_pending);near(player.hp,54);near(player.car.speed,135*(1-(1-.985)*1.3));near(player.car.stun,.12);rr_pack_dispose(&pack);
 printf("PROFILE_DAMAGE_SUMMARY cases=%zu applied=%u dodged=%u wrecks=%u blocked=%u projectile_seams=%u checks=%u max_error=%.17g failures=%u; current full-pack projectile route tested, reset/EMP/production inactive,no graphics/rewards\n",sizeof(damage_cases)/sizeof(*damage_cases),applied,dodged,wrecked,blocked,impacts,checks,worst,failures);return failures?1:0;
}
