import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';
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
