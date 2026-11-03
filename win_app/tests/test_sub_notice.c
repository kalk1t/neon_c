#include "rr_sub_notice.h"
typedef struct SubCase {int op,passed,total,best;double time,dt,timer;int shown;const char *text;} SubCase;
#include "sub_notice_fixture.h"
int main(void){RRSubNotice notice={0};int failures=0;
    for(size_t i=0;i<sizeof(sub_cases)/sizeof(sub_cases[0]);i++){
        const SubCase *c=&sub_cases[i];
        if(c->op==0)rr_sub_notice_checkpoint(&notice,c->passed,c->total,c->time);
        else if(c->op==1)rr_sub_notice_lap(&notice,c->time,c->best!=0);
        else rr_sub_notice_tick(&notice,c->dt);
        if(strcmp(notice.text,c->text)!=0||notice.shown!=(c->shown!=0)||fabs(notice.timer-c->timer)>1e-12){if(failures<8)fprintf(stderr,"submessage row %zu failed\n",i);failures++;}
    }
    printf("Actual source checkpoint/lap/HUD.sub: %zu Unicode caption/timer states, %d failures\n",sizeof(sub_cases)/sizeof(sub_cases[0]),failures);return failures?1:0;
}
