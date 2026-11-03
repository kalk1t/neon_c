// Actual source AI/Car CPU fixture. No renderer, network, accounts or game launch.
import{readFile,writeFile}from'node:fs/promises';import{createHash}from'node:crypto';import{pathToFileURL}from'node:url';import path from'node:path';import vm from'node:vm';
const native=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(native,'../game'),sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const names=['package.json','src/ai.js','src/main.js','src/car.js','src/track.js','src/roads.js'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const[THREE,{aiInput,Racer,LIVERIES},{Car},{Track},{ROADS}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/ai.js'),load('src/car.js'),load('src/track.js'),load('src/roads.js')]);
const car=()=>{const c=Object.create(Car.prototype);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),t:0,boost:1,boosting:false,maxMul:1,nitro:0,thrust:0,kick:0,sputter:0,model:{position:{},rotation:{}},pods:[],fans:[],ribbons:[],sync(){}});c.place(new THREE.Vector3(0,118,0),0,0);return c;};
const snapshot=c=>({pos:c.pos.toArray(),vel:c.vel.toArray(),fwd:c.fwd.toArray(),yaw:c.yaw,pitch:c.pitch,roll:c.roll,speed:c.speed,boost:c.boost,t:c.t,stun:c.stun,roll_time:c.rollT??0,surge:c.surge??0,max_multiplier:c.maxMul,thrust:c.thrust,nitro:c.nitro,kick:c.kick,sputter:c.sputter,impact:c.impact??0,roll_direction:c.rollDir??0,boosting:c.boosting});
const rows=[],traces=[];let index=0;
for(const road of ROADS){const track=new Track(road);
 for(let scenario=0;scenario<32;scenario++){
  const c=car(),sample=Math.floor(track.n*scenario/32),f=track.frame(sample);c.pos.copy(f.p).addScaledVector(f.right,(scenario%5-2)*9);c.pos.y+=(scenario%3-1)*7;c.yaw=Math.atan2(-f.t.x,-f.t.z)+(scenario%4-1.5)*.38;c.fwd.set(-Math.sin(c.yaw),0,-Math.cos(c.yaw));c.speed=[-8,0,7,95,138][scenario%5];c.boost=[.35,.35000001,1,0][scenario%4];
  const r={car:c,sample,seed:scenario*.7,lane:scenario%11-5,laneY:scenario%5-2,dodge:scenario%3-1,skill:LIVERIES[scenario%10].skill};const before={...r,car:snapshot(c)};const peers=[r];
  for(let p=0;p<3;p++){const lateral=p===0?5.5:p===1?-2:1,along=p===0?32:p===1?12:20;peers.push({alive:p!==2||scenario%2===0,car:{pos:c.pos.clone().addScaledVector(c.fwd,along).addScaledVector(f.right,lateral)}});}
  const target=track.checkpoints[scenario%track.checkpoints.length].pos,coin=scenario%3===0?f.p.clone().addScaledVector(f.right,15):null,dt=[1/144,1/60,1/30,.05][scenario%4],time=scenario*1.25;
  const result=aiInput(r,track,peers,target,time,dt,coin);
  rows.push({id:index++,road:road.id,before,peers:peers.slice(1).map(p=>({pos:p.car.pos.toArray(),alive:p.alive})),target:target.toArray(),coin:coin?.toArray()??null,time,dt,expected:result,expected_dodge:r.dodge});
 }
 for(let scenario=0;scenario<2;scenario++){
  const c=car(),sample=Math.floor(track.n*(scenario?.6:.05)),f=track.frame(sample);c.place(f.p,Math.atan2(-f.t.x,-f.t.z),scenario?35:0);
  const r={car:c,sample,seed:7.25+scenario,lane:scenario?-4:3,laneY:scenario?1.5:-1,dodge:0,skill:LIVERIES[scenario].skill};const initial=snapshot(c),steps=[];
  for(let frame=0;frame<180;frame++){
   r.sample=track.nearestGlobal(c.pos);const target=track.checkpoints[(scenario+Math.floor(frame/90))%track.checkpoints.length].pos;const coin=frame>=50&&frame<65?track.pts[(r.sample+12)%track.n]:null;
   const dt=[1/60,1/144,.05,1/30][frame%4],time=frame/40,input=aiInput(r,track,[r],target,time,dt,coin);c.update(dt,input,{collide:()=>null});steps.push({sample:r.sample,target:target.toArray(),coin:coin?.toArray()??null,time,dt,input,dodge:r.dodge,car:snapshot(c)});
  }
  traces.push({road:road.id,scenario,driver:{seed:r.seed,lane:r.lane,laneY:r.laneY,skill:r.skill},initial,steps});
 }
}
const progress=[];for(const n of[128,2000,4096])for(const lap of[1,2,3])for(const nextCP of[0,1,12])for(const sample of[0,n/2,n/2+1,n-1]){const r={sample,lap,nextCP};progress.push({...r,n,expected:Racer.prototype.progress.call(r,{n})});}
const main=await readFile(path.join(browser,'src/main.js'),'utf8'),begin=main.indexOf('function updateRanking()'),end=main.indexOf('function showResults()',begin);if(begin<0||end<0)throw Error('ranking source changed');
const rankingSource=main.slice(begin,end),rubber=[];for(const road of ROADS)for(const delta of[-100000,-250,0,100,100000]){
 const track=new Track(road),player={sample:400,lap:1,nextCP:2,finished:false,progress:Racer.prototype.progress},rival={sample:400-delta,lap:1,nextCP:2,finished:false,car:{maxMul:1},progress:Racer.prototype.progress};
 const g={racers:[player,rival],rankOf(r){return this.ranking.indexOf(r)+1;}},ctx={g,player,rivals:[rival],track,road,THREE};vm.createContext(ctx);vm.runInContext(rankingSource+';updateRanking();',ctx);rubber.push({road:road.id,n:track.n,player_progress:player.progress(track),rival_progress:rival.progress(track),factor:rival.car.maxMul});
}
for(const s of sources)if(sha(await readFile(path.join(browser,s.path)))!==s.sha256)throw Error('reference changed');
await writeFile(out,JSON.stringify({sources,scope:'Actual aiInput, Racer.progress, updateRanking rubber band and Car.update CPU output; render objects stubbed. No race pack, contacts/combat/rewards or runtime AI activation.',liveries:LIVERIES,input_cases:rows,traces,progress,rubber,ranking_source:rankingSource},null,2)+'\n');console.log({inputCases:rows.length,traces:traces.length,steps:traces.reduce((n,t)=>n+t.steps.length,0),progress:progress.length,rubber:rubber.length});
