from pathlib import Path
import shutil
p=Path(r'C:\Users\User\Projects\neon_c\win_app');b=p/'reference/pre-audio-control-20261104';b.mkdir(exist_ok=False)
for f in ['CMakeLists.txt','Run-RoadstersRace.cmd','docs/PARITY.md','docs/CLIENT-LAUNCHER-2026-11-03.md']:
 d=b/f;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p/f,d)
files={
'src/core/rr_audio_current.h':r'''#ifndef RR_AUDIO_CURRENT_H
#define RR_AUDIO_CURRENT_H
#include "rr_session.h"
#include "rr_rival.h"
typedef enum RRAudioParameter {RR_AUDIO_OSC0,RR_AUDIO_OSC1,RR_AUDIO_OSC2,RR_AUDIO_ENGINE_LOWPASS,RR_AUDIO_ENGINE_GAIN,RR_AUDIO_WIND_LOWPASS,RR_AUDIO_WIND_GAIN,RR_AUDIO_MASTER_GAIN} RRAudioParameter;
typedef struct RRAudioTarget {RRAudioParameter parameter;double target,time,time_constant;} RRAudioTarget;
enum {RR_AUDIO_TARGETS=7};
typedef struct RRAudioPlan {RRAudioTarget targets[RR_AUDIO_TARGETS];size_t count;} RRAudioPlan;
/* Commands are WebAudio-style setTargetAtTime targets, NOT samples or a mixer.
 * Caller owns node/device readiness and monotonic audio-context seconds. Empty
 * plans model missing context before method input reads, as in source. No device,
 * callback, heap, RNG, graph allocation or audible output. Invalid ready inputs
 * or derived overflow leave output unchanged. No implicit gain/speed clamping. */
bool rr_audio_engine_plan(bool nodes_ready,double speed_fraction,bool boosting,bool active,double audio_time,RRAudioPlan *out);
bool rr_audio_master_plan(bool nodes_ready,double volume,bool muted,double audio_time,RRAudioPlan *out);
/* playing is CAPTURED at the source main-loop playing declaration, not derived
 * again from current_phase after the countdown branch. Current phase is used
 * only for countdown. Thus GO can be inactive for its first frame. Forward speed
 * only, max(0,speed)/current CAR.MAX; sideways HUD speed does not rev the engine.
 * Source alive is wreck<=0, not HP or visibility. No production main activation. */
bool rr_audio_engine_frame(bool nodes_ready,bool playing,RRSessionPhase current_phase,const RRLocalRacer *player,double audio_time,RRAudioPlan *out);
#endif
''',
'src/core/rr_audio_current.c':r'''#include "rr_audio_current.h"
static bool target(RRAudioPlan *p,RRAudioParameter parameter,double value,double now,double tau)
{
 if(p->count>=RR_AUDIO_TARGETS||!isfinite(value))return false;
 p->targets[p->count++]=(RRAudioTarget){parameter,value,now,tau};return true;
}
bool rr_audio_engine_plan(bool ready,double sf,bool boosting,bool active,double now,RRAudioPlan *out)
{
 if(!out)return false;RRAudioPlan p={0};if(!ready){*out=p;return true;}
 if(!isfinite(sf)||!isfinite(now)||now<0)return false;
 double f=52+sf*78+(boosting?14:0);
 if(!target(&p,RR_AUDIO_OSC0,f,now,.12)||!target(&p,RR_AUDIO_OSC1,f*2.004,now,.12)||!target(&p,RR_AUDIO_OSC2,f*.5,now,.12)||
    !target(&p,RR_AUDIO_ENGINE_LOWPASS,320+sf*650+(boosting?350:0),now,.15)||
    !target(&p,RR_AUDIO_ENGINE_GAIN,active?.045+sf*.03:0,now,.25)||
    !target(&p,RR_AUDIO_WIND_LOWPASS,220+sf*700,now,.2)||
    !target(&p,RR_AUDIO_WIND_GAIN,active?sf*sf*.03+(boosting?.015:0):0,now,.3))return false;
 *out=p;return true;
}
bool rr_audio_master_plan(bool ready,double volume,bool muted,double now,RRAudioPlan *out)
{
 if(!out)return false;RRAudioPlan p={0};if(!ready){*out=p;return true;}
 if(!isfinite(now)||now<0||(!muted&&!isfinite(volume)))return false;
 if(!target(&p,RR_AUDIO_MASTER_GAIN,muted?0:.28*volume,now,.03))return false;*out=p;return true;
}
bool rr_audio_engine_frame(bool ready,bool playing,RRSessionPhase phase,const RRLocalRacer *player,double now,RRAudioPlan *out)
{
 if(!ready)return rr_audio_engine_plan(false,0,false,false,now,out);
 if(!player||!isfinite(player->car.speed)||!isfinite(player->wreck)||phase<RR_SESSION_MENU||phase>RR_SESSION_ERROR)return false;
 return rr_audio_engine_plan(true,fmax(0,player->car.speed)/RR_CURRENT_CAR_MAX,player->car.boosting,(playing||phase==RR_SESSION_COUNTDOWN)&&player->wreck<=0,now,out);
}
''',
'tools/capture-audio-control.mjs':r'''import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';
const root=process.argv[2],game=path.resolve(root,'../game'),out=path.join(root,'reference/ui-parity-20261103/audio-control-capture');fs.mkdirSync(out,{recursive:true});
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');const paths=['src/audio.js','src/main.js','src/car.js','src/ai.js','package.json'];
const sources=paths.map(file=>({path:file,sha256:sha(fs.readFileSync(path.join(game,file)))}));
const audio=fs.readFileSync(path.join(game,'src/audio.js'),'utf8'),main=fs.readFileSync(path.join(game,'src/main.js'),'utf8'),car=fs.readFileSync(path.join(game,'src/car.js'),'utf8'),ai=fs.readFileSync(path.join(game,'src/ai.js'),'utf8');
let Sound;vm.runInNewContext(audio.replace('export class Sound','class Sound')+'\ncapture(Sound);',{capture:s=>Sound=s},{timeout:1000});
const defaults=new Sound();if(defaults.ctx!==null||defaults.volume!==.7||defaults.muted!==false||defaults.voices!==0)throw Error('Constructor changed');
const playing=main.split('\n').find(l=>l.trimStart().startsWith('const playing ='));
const engine=main.split('\n').find(l=>l.trimStart().startsWith('sound.engine('));
const constants=car.split('\n').filter(l=>l.startsWith('const TOP =')||l.startsWith('export const CAR =')).join('\n').replace('export const','const');
const alive=ai.match(/get alive\(\)\s*\{[^}]+\}/)?.[0];if(!playing||!engine||!alive)throw Error('Frame boundary changed');
const oscExpr=audio.match(/this\.oscs = (\[\[.*?\]\])\.map/)?.[1];let oscillatorSpec=null;if(!oscExpr)throw Error('Oscillator source changed');vm.runInNewContext('capture('+oscExpr+');',{capture:v=>oscillatorSpec=v},{timeout:1000});if(oscillatorSpec.length!==3)throw Error('Unsupported oscillator count');
function make(ready,now){const s=new Sound(),events=[];if(ready){const param=id=>({setTargetAtTime:(target,time,tau)=>events.push({id,target,time,tau})});s.ctx={currentTime:now};s.oscs=oscillatorSpec.map(([,mul],i)=>({mul,o:{frequency:param(i)}}));s.engLP={frequency:param(3)};s.engGain={gain:param(4)};s.windLP={frequency:param(5)};s.wind={gain:param(6)};s.master={gain:param(7)};}return {s,events};}
const engineCases=[];for(const ready of [false,true])for(const sf of [-.25,0,.01,.5,1,138/95,2,8])for(const boosting of [false,true])for(const active of [false,true])for(const now of [0,.016,123.456]){const {s,events}=make(ready,now);s.engine(sf,boosting,active);engineCases.push({ready,sf,boosting,active,now,events});}
const masterCases=[];for(const ready of [false,true])for(const volume of [-1,0,.3,.7,1,1.5])for(const muted of [false,true])for(const now of [0,42]){const {s,events}=make(ready,now);s.volume=volume;s.muted=muted;s.applyVolume();masterCases.push({ready,volume,muted,now,events});}
const frameCases=[];const frameProgram=constants+'\nclass Actor { '+alive+' }\nconst player=new Actor();player.wreck=wreck;\n'+playing+'\ng.state=currentState;\n'+engine;
const phases=[['menu','menu',0],['shop','shop',0],['paused','paused',2],['countdown','countdown',3],['countdown','race',1],['race','finished',4],['race','race',1],['finished','finished',4]];
for(const ready of [false,true])for(const [entered,current,phase]of phases)for(const speed of [-40,0,25,135,138*135/95])for(const side of [0,25])for(const boosting of [false,true])for(const wreck of [-1,0,.001,2.4]){
 const {s,events}=make(ready,8.25);vm.runInNewContext(frameProgram,{wreck,g:{state:entered},currentState:current,car:{speed,side,boosting},sound:s,Math},{timeout:1000});
 frameCases.push({ready,entered,current,phase,playing:entered==='race'||entered==='finished',speed,side,boosting,wreck,now:8.25,events});
}
for(const f of sources)if(sha(fs.readFileSync(path.join(game,f.path)))!==f.sha256)throw Error('Source changed');
fs.writeFileSync(path.join(out,'reference.json'),JSON.stringify({version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,defaults:{volume:defaults.volume,muted:defaults.muted,voices:defaults.voices},frameProgram,frameProgramSHA256:sha(frameProgram),engineCases,masterCases,frameCases,scope:'Actual Sound class constructor/engine/applyVolume plus actual main engine call, playing expression, CAR constants and alive getter in isolated VM with recording parameter nodes. init/tone/rumble/AudioContext/DOM/audio samples/devices/network were never invoked. Scripted frame state changes are policy fixtures, not simulated driving or audible acceptance.'})+'\n');
console.log({engine:engineCases.length,master:masterCases.length,frames:frameCases.length,noDevices:true});
'''}
for f,s in files.items():
 assert not(p/f).exists(),f
 (p/f).write_text(s,encoding='utf-8')
f=p/'CMakeLists.txt';s=f.read_text().replace('add_library(rr_core STATIC ','add_library(rr_core STATIC src/core/rr_audio_current.c ');marker='    add_executable(rr_results_summary_tests ';assert s.count(marker)==1
block='''    add_executable(rr_audio_control_tests tests/test_audio_control.c)
    target_link_libraries(rr_audio_control_tests PRIVATE rr_core)
    if(MSVC)
        target_compile_options(rr_audio_control_tests PRIVATE /W4 /WX)
    else()
        target_compile_options(rr_audio_control_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME browser_continuous_audio_control COMMAND rr_audio_control_tests)
''';f.write_text(s.replace(marker,block+marker))
print('Installed audio target-command model and source capture tool. No audio device, DSP, playback, graphics or launcher activation.')
