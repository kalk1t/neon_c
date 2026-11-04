import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';
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
