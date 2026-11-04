#include "rr_audio_submit.h"
#include "rr_arsenal.h"
#include <stdio.h>
#include <string.h>
static unsigned draws,clocks;static int checks,failures;
#define CHECK(x) do{checks++;if(!(x)){failures++;fprintf(stderr,"fire audio line%d\n",__LINE__);}}while(0)
static double random_unit(void *ctx){(void)ctx;draws++;return .25;}
static double clock_read(void *ctx){(void)ctx;return 10+clocks++*.001;}
int main(void){
 const unsigned events[]={RR_ARSENAL_BOLT,RR_ARSENAL_OVERHEAT,RR_ARSENAL_MISSILE,RR_ARSENAL_DENY,RR_ARSENAL_SPIN,RR_ARSENAL_EMP};
 const RRAudioCue cues[]={RR_CUE_BLASTER,RR_CUE_OVERHEAT,RR_CUE_MISSILE,RR_CUE_DENY,RR_CUE_BOOST_ON,RR_CUE_EMP};
 for(unsigned ready=0;ready<2;ready++)for(unsigned i=0;i<6;i++){
 RRAudioVoiceBank a,b;rr_audio_voice_bank_init(&a);rr_audio_voice_bank_init(&b);RRAudioCueSubmission x,y;
 draws=clocks=0;CHECK(rr_audio_submit_arsenal(&a,ready!=0,events[i],random_unit,NULL,clock_read,NULL,&x));unsigned d=draws,c=clocks;
 draws=clocks=0;CHECK(rr_audio_submit_cue(&b,ready!=0,cues[i],1,0,false,random_unit,NULL,clock_read,NULL,&y));
 CHECK(d==draws&&c==clocks&&x.complete&&y.complete&&x.voices.count==y.voices.count&&x.total_draws==y.total_draws&&x.clock_reads==y.clock_reads);
 CHECK(!memcmp(&x.recipe,&y.recipe,sizeof(x.recipe)));
 for(size_t j=0;j<x.voices.count;j++){CHECK(!memcmp(&x.voices.commands[j],&y.voices.commands[j],sizeof(x.voices.commands[j])));CHECK(rr_audio_voice_end(&a,x.voices.commands[j].id));}
 CHECK(a.active==0);if(!ready)CHECK(d==(events[i]==RR_ARSENAL_BOLT?1u:0u)&&c==0);
 }
 RRAudioVoiceBank bank;rr_audio_voice_bank_init(&bank);RRAudioCueSubmission x,before;memset(&x,0x5a,sizeof(x));before=x;draws=clocks=0;
 const unsigned invalid[]={0,7,~0u};for(unsigned i=0;i<3;i++)CHECK(!rr_audio_submit_arsenal(&bank,true,invalid[i],random_unit,NULL,clock_read,NULL,&x)&&!memcmp(&x,&before,sizeof(x))&&bank.active==0&&draws==0&&clocks==0);
 printf("FIRE_AUDIO mappings=6 readiness_states=2 checks=%d failures=%d; callback binding tested separately, no device/main activation\n",checks,failures);return failures?1:0;
}
