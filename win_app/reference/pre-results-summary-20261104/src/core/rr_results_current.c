#include "rr_results_current.h"
#include <stdint.h>
bool rr_results_current_build(const RRLocalRacer *const *racers,size_t count,size_t player,int samples,double time,int finish_position,RRSessionPhase phase,RRResultsCurrent *out){
 if(!out||phase!=RR_SESSION_FINISHED||!racers||!count||count>RR_LOCAL_MAX_CARS||player>=count||samples<=0||!isfinite(time)||time<0||finish_position<1||(size_t)finish_position>count)return false;
 const RRLocalRacer *p=racers[player];if(!p||p->lap_count>2||p->lap_count>p->lap_capacity||(p->lap_count&&!p->lap_times))return false;
 RRResultsCurrent value={0};value.best_lap=SIZE_MAX;value.lap_count=p->lap_count;value.coin_count=count;value.time_count=count>1?count:0;value.finish_position=finish_position;
 for(size_t i=0;i<p->lap_count;i++){double lap=p->lap_times[i];if(!isfinite(lap)||lap<0)return false;value.laps[i]=lap;value.total+=lap;if(value.best_lap==SIZE_MAX||lap<value.laps[value.best_lap])value.best_lap=i;}
 if(!isfinite(value.total))return false;
 for(size_t i=0;i<count;i++){
  const RRLocalRacer *r=racers[i];if(!r||r->coins<0)return false;value.coins[i]=(RRResultsCoin){(int)i,r->coins};
  if(value.time_count){double seconds;
   if(r->race.finished)seconds=r->race.finish_time;
   else{if(r->race.lap<1||r->ai.sample<0||r->ai.sample>=samples||r->race.next_checkpoint<0)return false;double done=fmax(1,rr_ai_progress(r->ai.sample,r->race.lap,r->race.next_checkpoint,samples));seconds=fmax(time,time*2*samples/done);}
   if(!isfinite(seconds)||seconds<0)return false;value.times[i]=(RRResultsTime){(int)i,seconds,!r->race.finished};
  }
 }
 for(size_t i=1;i<value.time_count;i++){RRResultsTime row=value.times[i];size_t j=i;while(j&&row.seconds<value.times[j-1].seconds){value.times[j]=value.times[j-1];j--;}value.times[j]=row;}
 for(size_t i=1;i<count;i++){RRResultsCoin row=value.coins[i];size_t j=i;while(j&&row.coins>value.coins[j-1].coins){value.coins[j]=value.coins[j-1];j--;}value.coins[j]=row;}
 for(size_t i=0;i<value.time_count;i++)if((size_t)value.times[i].racer==player)value.player_time_rank=(int)i+1;
 for(size_t i=0;i<count;i++)if((size_t)value.coins[i].racer==player)value.player_coin_rank=(int)i+1;
 *out=value;return true;
}
bool rr_run_results_build(const RRRunCurrent *run,RRResultsCurrent *out){
 if(!run||!run->current||run->current->error||run->host.error||!run->host.battle||!run->host.battle->pack||!run->host.battle->pack->track||!run->host.battle->fire.player||run->host.battle->fire.player->race.mode!=RR_RACE_TWO_LAPS)return false;
 const RRLocalRacer *racers[RR_LOCAL_MAX_CARS];for(size_t i=0;i<RR_LOCAL_MAX_CARS;i++)racers[i]=run->host.battle->roster[i];
 return rr_results_current_build(racers,RR_LOCAL_MAX_CARS,0,run->host.battle->pack->track->sample_count,run->host.race_time,run->host.battle->pack->finish_position,run->host.phase,out);
}
