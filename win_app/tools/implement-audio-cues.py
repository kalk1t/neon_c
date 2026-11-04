from pathlib import Path
import shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-audio-cues-20261104';b.mkdir(exist_ok=False)
for f in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/f;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,d)
files={
'src/core/rr_audio_cues.h':r'''#ifndef RR_AUDIO_CUES_H
#define RR_AUDIO_CUES_H
#include <stdbool.h>
#include <stddef.h>
typedef enum RRAudioCue {RR_CUE_COUNTDOWN,RR_CUE_CHECKPOINT,RR_CUE_LAP,RR_CUE_COIN,RR_CUE_BOOST_ON,RR_CUE_BOOST_OFF,RR_CUE_BLASTER,RR_CUE_OVERHEAT,RR_CUE_MISSILE,RR_CUE_EMP,RR_CUE_DENY,RR_CUE_LOCK_TICK,RR_CUE_LOCKED,RR_CUE_ZAP,RR_CUE_HIT_MARK,RR_CUE_HIT,RR_CUE_DAMAGE,RR_CUE_EXPLOSION,RR_CUE_COUNT} RRAudioCue;
typedef enum RRAudioRequestKind {RR_CUE_TONE,RR_CUE_RUMBLE} RRAudioRequestKind;
typedef enum RRAudioWave {RR_CUE_SINE,RR_CUE_TRIANGLE} RRAudioWave;
typedef enum RRAudioFilter {RR_CUE_LOWPASS,RR_CUE_BANDPASS} RRAudioFilter;
typedef struct RRAudioCueRequest {RRAudioRequestKind kind;RRAudioWave wave;RRAudioFilter filter;double frequency,duration,volume,when,slide,pan,from,to;} RRAudioCueRequest;
enum {RR_CUE_REQUESTS=4};
typedef struct RRAudioCuePlan {RRAudioCueRequest requests[RR_CUE_REQUESTS];size_t count;unsigned recipe_random_draws;} RRAudioCuePlan;
/* Pure ordered SOURCE CALL recipes, BEFORE tone/rumble context/gain/voice guards.
 * amount is explicit vol for coin/zap/damage/explosion, impact k for hit; unused
 * by other cues. flag is GO for countdown / big for hitMark. pan is raw source
 * input, not yet clamped by out(). blaster_unit is one ALREADY consumed [0,1)
 * source RNG value, read only by blaster. This function never advances RNG.
 * recipe_random_draws counts that pre-request blaster draw only; admitted rumble
 * offsets consume separate RNG in the future dispatcher. Preserve request order.
 * These are NOT approved voices, samples or audible output. Context/vol<=.002/
 * MAX_VOICES18, source envelopes, offsets, panner and onended lifecycle remain
 * external. No backend/network/account/main activation. Failure preserves out. */
bool rr_audio_cue_recipe(RRAudioCue cue,double amount,double pan,bool flag,double blaster_unit,RRAudioCuePlan *out);
#endif
''',
'src/core/rr_audio_cues.c':r'''#include "rr_audio_cues.h"
#include <math.h>
static void tone(RRAudioCuePlan *p,double f,double dur,RRAudioWave wave,double vol,double when,double slide,double pan){RRAudioCueRequest *r=p->requests+p->count++;r->kind=RR_CUE_TONE;r->wave=wave;r->frequency=f;r->duration=dur;r->volume=vol;r->when=when;r->slide=slide;r->pan=pan;}
static void rumble(RRAudioCuePlan *p,double dur,double from,double to,double vol,double pan,RRAudioFilter filter){RRAudioCueRequest *r=p->requests+p->count++;r->kind=RR_CUE_RUMBLE;r->filter=filter;r->duration=dur;r->from=from;r->to=to;r->volume=vol;r->pan=pan;}
bool rr_audio_cue_recipe(RRAudioCue cue,double amount,double pan,bool flag,double unit,RRAudioCuePlan *out)
{
 if(!out||(unsigned)cue>=RR_CUE_COUNT)return false;RRAudioCuePlan p={0};
 switch(cue){
 case RR_CUE_COUNTDOWN:tone(&p,flag?880:440,flag?.6:.22,RR_CUE_SINE,flag?.12:.09,0,0,0);break;
 case RR_CUE_CHECKPOINT:tone(&p,784,.32,RR_CUE_SINE,.05,0,0,0);tone(&p,1175,.32,RR_CUE_SINE,.05,.07,0,0);break;
 case RR_CUE_LAP:{const double f[]={659,784,988,1318};for(size_t i=0;i<4;i++)tone(&p,f[i],.45,RR_CUE_SINE,.055,(double)i*.08,0,0);break;}
 case RR_CUE_COIN:tone(&p,1318,.09,RR_CUE_SINE,.05*amount,0,0,pan);tone(&p,1976,.22,RR_CUE_SINE,.045*amount,.06,0,pan);break;
 case RR_CUE_BOOST_ON:rumble(&p,.55,1600,260,.04,0,RR_CUE_LOWPASS);tone(&p,95,.4,RR_CUE_SINE,.05,0,40,0);rumble(&p,.3,5000,1800,.015,0,RR_CUE_BANDPASS);break;
 case RR_CUE_BOOST_OFF:rumble(&p,.35,2400,500,.018,0,RR_CUE_BANDPASS);break;
 case RR_CUE_BLASTER:{if(!isfinite(unit)||unit<0||unit>=1)return false;double f=620+unit*60;p.recipe_random_draws=1;tone(&p,f,.075,RR_CUE_TRIANGLE,.028,0,f*.45,0);tone(&p,f*2,.04,RR_CUE_SINE,.008,0,0,0);break;}
 case RR_CUE_OVERHEAT:tone(&p,520,.4,RR_CUE_TRIANGLE,.05,0,180,0);rumble(&p,.5,3000,600,.025,0,RR_CUE_BANDPASS);break;
 case RR_CUE_MISSILE:rumble(&p,.7,400,2200,.06,0,RR_CUE_BANDPASS);tone(&p,180,.35,RR_CUE_SINE,.06,0,90,0);break;
 case RR_CUE_EMP:tone(&p,700,.7,RR_CUE_SINE,.09,0,55,0);tone(&p,1400,.3,RR_CUE_TRIANGLE,.025,0,300,0);rumble(&p,.9,2400,80,.07,0,RR_CUE_LOWPASS);break;
 case RR_CUE_DENY:tone(&p,220,.1,RR_CUE_TRIANGLE,.035,0,0,0);break;
 case RR_CUE_LOCK_TICK:tone(&p,1320,.04,RR_CUE_SINE,.02,0,0,0);break;
 case RR_CUE_LOCKED:tone(&p,1568,.07,RR_CUE_SINE,.035,0,0,0);tone(&p,2093,.12,RR_CUE_SINE,.03,.07,0,0);break;
 case RR_CUE_ZAP:tone(&p,760,.18,RR_CUE_SINE,.06*amount,0,240,pan);break;
 case RR_CUE_HIT_MARK:tone(&p,flag?1760:2350,flag?.08:.035,RR_CUE_SINE,flag?.05:.018,0,0,0);break;
 case RR_CUE_HIT:{if(!isfinite(amount))return false;double v=fmin(.16,.05+amount*.005);tone(&p,140,.25,RR_CUE_SINE,v,0,45,0);rumble(&p,.18,800,120,v*.35,0,RR_CUE_LOWPASS);break;}
 case RR_CUE_DAMAGE:tone(&p,300,.14,RR_CUE_TRIANGLE,.035*amount,0,140,pan);break;
 case RR_CUE_EXPLOSION:tone(&p,85,.8,RR_CUE_SINE,.18*amount,0,28,pan);rumble(&p,1,1100,60,.12*amount,pan,RR_CUE_LOWPASS);break;
 default:return false;
 }
 for(size_t i=0;i<p.count;i++){const RRAudioCueRequest *r=p.requests+i;if(!isfinite(r->frequency)||!isfinite(r->duration)||!isfinite(r->volume)||!isfinite(r->when)||!isfinite(r->slide)||!isfinite(r->pan)||!isfinite(r->from)||!isfinite(r->to))return false;}
 *out=p;return true;
}
''',
'tools/capture-audio-cues.mjs':r'''import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';
const root=process.argv[2],game=path.resolve(root,'../game'),out=path.join(root,'reference/ui-parity-20261103/audio-cues-capture');fs.mkdirSync(out,{recursive:true});
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),file=path.join(game,'src/audio.js'),source=fs.readFileSync(file,'utf8');
const names=['countdown','checkpoint','lap','coin','boostOn','boostOff','blaster','overheat','missile','emp','deny','lockTick','locked','zap','hitMark','hit','damage','explosion'];
const cases=[];let rngCalls=0,unit=0;const math=Object.create(Math);math.random=()=>{rngCalls++;return unit;};let Sound;vm.runInNewContext(source.replace('export class Sound','class Sound')+'\ncapture(Sound);',{Math:math,capture:v=>Sound=v},{timeout:1000});
for(let cue=0;cue<names.length;cue++){
 const name=names[cue],dynamic=['coin','zap','damage','explosion'].includes(name),amounts=dynamic?[-1,0,.02,.04,1,2]:name==='hit'?[-20,-10,0,1,21,22,23,100]:[1],pans=dynamic?[-2,-.5,0,.5,2]:[0],flags=['countdown','hitMark'].includes(name)?[false,true]:[false],units=name==='blaster'?[0,.5,.999999]:[0];
 for(const amount of amounts)for(const pan of pans)for(const flag of flags)for(const random of units){
  const sound=new Sound(),requests=[];rngCalls=0;unit=random;
  sound.tone=(frequency,duration=.15,wave='sine',volume=.2,when=0,slide=0,panning=0)=>requests.push({kind:0,wave:wave==='sine'?0:1,filter:0,frequency,duration,volume,when,slide,pan:panning,from:0,to:0});
  sound.rumble=(duration,from,to,volume,panning=0,type='lowpass')=>requests.push({kind:1,wave:0,filter:type==='lowpass'?0:1,frequency:0,duration,volume,when:0,slide:0,pan:panning,from,to});
  if(dynamic)sound[name](amount,pan);else if(name==='hit')sound.hit(amount);else if(['countdown','hitMark'].includes(name))sound[name](flag);else sound[name]();
  if(requests.length>4||rngCalls!==(name==='blaster'?1:0))throw Error('Source recipe changed');cases.push({cue,name,amount,pan,flag,unit:random,randomDraws:rngCalls,requests});
 }
}
if(sha(fs.readFileSync(file))!==sha(source))throw Error('Source changed');
fs.writeFileSync(path.join(out,'reference.json'),JSON.stringify({version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources:[{path:'src/audio.js',sha256:sha(source)},{path:'package.json',sha256:sha(fs.readFileSync(path.join(game,'package.json')))}],cases,scope:'Actual18 Sound cue methods, with tone/rumble overridden solely to record raw request arguments/defaults before context/gain/voice guards. Only pre-request blaster Math.random is observed; rumble-offset RNG and all allocation/DSP/device/onended behavior are deliberately outside this oracle. No init, samples, audio/graphics context, game or network execution.'})+'\n');
console.log({cues:names.length,cases:cases.length,requests:cases.reduce((n,c)=>n+c.requests.length,0),noPlayback:true});
'''}
for f,s in files.items():
 assert not(p/f).exists(),f
 (p/f).write_text(s,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/core/rr_audio_cues.c ');marker='    add_executable(rr_audio_control_tests ';assert s.count(marker)==1
block='''    add_executable(rr_audio_cue_tests tests/test_audio_cues.c)
    target_link_libraries(rr_audio_cue_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_audio_cue_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_audio_cue_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_audio_cue_recipes COMMAND rr_audio_cue_tests)
''';f.write_text(s.replace(marker,block+marker))
print('Installed all18 source audio cue recipes, before backend/voice dispatch. No audible output or launcher change.')
