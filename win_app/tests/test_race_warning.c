#include "rr_race_warning.h"
#include <stdio.h>
typedef struct WarningCase {int mode,finished,next,count;double time;RRVec3 previous,current;bool expected;double duration;} WarningCase;
#include "missed_gate_fixture.h"
int main(void){int failures=0;size_t warnings=0;
    for(size_t i=0;i<sizeof(warning_cases)/sizeof(warning_cases[0]);i++){
        const WarningCase *c=&warning_cases[i];RRRace race={0};race.mode=(RRRaceMode)c->mode;race.finished=c->finished!=0;race.next_checkpoint=c->next;
        RRCar car={0};car.pos=c->current;bool got=rr_race_missed_gate(&race,c->count,&car,c->previous,c->time);
        if(got)warnings++;
        if(got!=c->expected||(got&&fabs(c->duration-RR_MISSED_GATE_SECONDS)>1e-12)){fprintf(stderr,"missed gate row %zu\n",i);failures++;}
    }
    printf("Actual raceRules missed-gate warning: %zu boundary cases, %zu warnings, %d failures\n",sizeof(warning_cases)/sizeof(warning_cases[0]),warnings,failures);return failures?1:0;
}
