#include "rr_pack_current.h"
#include <stdio.h>
#include <string.h>
#include "contact_mass_reference.h"
static unsigned failures,checks;static size_t at;static double worst;
#define CHECK(x) do{checks++;if(!(x)){if(failures<12)fprintf(stderr,"contact mass case%zu line%d: %s\n",at,__LINE__,#x);failures++;}}while(0)
static void near(double a,double b){double e=fabs(a-b);if(e>worst)worst=e;CHECK(isfinite(a)&&isfinite(b)&&e<1e-10);}
static void vector(RRVec3 a,RRVec3 b){near(a.x,b.x);near(a.y,b.y);near(a.z,b.z);}
static void apply(RRLocalRacer *r,const ContactActor *a,int index){memset(r,0,sizeof(*r));r->car.profile=(RRCarProfile)a->profile;r->car.pos=a->pos;r->car.vel=a->vel;r->car.speed=42+index;r->hp=61+index;r->wreck=a->alive?0:1;r->race.finished=a->finished;r->is_player=a->player;}
static RRTrack track;static RRLocalPack pack;static RRPackCurrent current_pack;static RRLocalRacer player;
static bool random_pair(void *ctx,int index,double values[2]){(void)ctx;(void)index;values[0]=values[1]=.5;return true;}
static unsigned hooks;
static void setup(RRLocalRacer *p,RRLocalRacer *b){rr_car_use_stats(&p->car,"viper");rr_car_use_stats(&b->car,"titan");p->car.pos=rr_v3(0,400,0);b->car.pos=rr_v3(-2,400,0);p->car.vel=rr_v3(-20,0,0);b->car.vel=rr_v3(0,0,0);p->wreck=b->wreck=0;}
static void inject_last(void *ctx,RRLocalRacer *r,int index,unsigned flags){(void)ctx;(void)r;(void)flags;hooks++;if(index!=RR_AI_RIVALS)return;for(int i=0;i<RR_AI_RIVALS;i++)pack.rivals[i].car.pos=rr_v3(1000+i*20,400,0);setup(&player,&pack.rivals[0]);}
int main(void){
 size_t pairs=0,sounds=0;RRLocalRacer racers[11];RRCarContacts out;
 for(at=0;at<sizeof(contact_cases)/sizeof(*contact_cases);at++){
 const ContactCase *c=&contact_cases[at];for(size_t i=0;i<c->count;i++)apply(&racers[i],&c->before[i],(int)i);
 CHECK(rr_local_car_contacts_current(racers,c->count,c->shake,&out));near(out.shake,c->outshake);CHECK(out.count==c->events);pairs+=out.count;
 for(size_t i=0;i<c->count;i++){const ContactActor *e=&c->after[i];vector(racers[i].car.pos,e->pos);vector(racers[i].car.vel,e->vel);CHECK((int)racers[i].car.profile==e->profile&&racers[i].is_player==e->player&&(racers[i].wreck<=0)==e->alive&&racers[i].race.finished==e->finished);near(racers[i].car.speed,42+(double)i);near(racers[i].hp,61+(double)i);}
 for(size_t i=0;i<out.count&&i<c->events;i++){const RRCarContactEvent *a=&out.events[i],*e=&c->pairs[i];CHECK(a->a==e->a&&a->b==e->b&&a->player_hit==e->player_hit);near(a->relative_speed,e->relative_speed);near(a->hit_volume,e->hit_volume);sounds+=a->player_hit?1:0;}
 }
 // Legacy keeps equal-mass response; invalid calls preserve caller state.
 RRLocalRacer legacy[2]={0},equal[2]={0};setup(legacy,legacy+1);memcpy(equal,legacy,sizeof(equal));equal[0].car.profile=equal[1].car.profile=RR_PROFILE_ROADSTER;
 RRCarContacts a,b;CHECK(rr_local_car_contacts(legacy,2,.2,&a)&&rr_local_car_contacts_current(equal,2,.2,&b));for(int i=0;i<2;i++){vector(legacy[i].car.pos,equal[i].car.pos);vector(legacy[i].car.vel,equal[i].car.vel);}
 memset(&a,0x5a,sizeof(a));b=a;RRLocalRacer saved=legacy[0];CHECK(!rr_local_car_contacts_current(legacy,12,0,&a)&&!memcmp(&a,&b,sizeof(a))&&!memcmp(&saved,legacy,sizeof(saved)));CHECK(!rr_local_car_contacts_current(NULL,1,0,&a)&&!memcmp(&a,&b,sizeof(a)));CHECK(!rr_local_car_contacts_current(legacy,2,0,NULL));CHECK(rr_local_car_contacts_current(NULL,0,.7,&a)&&a.count==0);near(a.shake,.7);
 // Real current pack phase routing: controlled scalar mutation in permitted
 // final-rival callback establishes a mixed pair immediately before contacts.
 CHECK(rr_track_init_road(&track,1));player.is_player=true;rr_car_reset(&player.car,rr_v3(8,115.5,60),0,0);rr_race_reset(&player.race,RR_RACE_TWO_LAPS,player.car.pos,0);CHECK(rr_pack_current_start(&current_pack,&pack,&track,&player,random_pair,NULL));
 RRPackEvents e;pack.shake=.2;CHECK(rr_pack_current_rivals_dispatch(&current_pack,RR_PACK_PLAYING,.01,1,0,NULL,random_pair,NULL,inject_last,NULL,&e));CHECK(!e.error&&hooks==RR_AI_RIVALS&&e.contacts.count==1&&pack.coin_pending);
 // Browser source formula for Viper(.8) / Titan(2) paired at distance2.
 near(player.car.pos.x,1.3*(4./2.8));near(pack.rivals[0].car.pos.x,-2-1.3*(1.6/2.8));near(player.car.vel.x,-20+12*(4./2.8));near(pack.rivals[0].car.vel.x,-12*(1.6/2.8));near(pack.shake,.7);
 CHECK(e.contacts.events[0].a==0&&e.contacts.events[0].b==1&&e.contacts.events[0].player_hit);near(e.contacts.events[0].hit_volume,12);CHECK(rr_pack_coins_after_weapons(&pack,&e.coins)&&!pack.coin_pending);rr_pack_dispose(&pack);
 printf("CONTACT_MASS_SUMMARY cases=%zu pairs=%zu sounds=%zu checks=%u max_error=%.17g hooks=%u failures=%u; source contact oracle plus C pack routing, production inactive,no graphics/account/RNG changes\n",sizeof(contact_cases)/sizeof(*contact_cases),pairs,sounds,checks,worst,hooks,failures);return failures?1:0;
}
