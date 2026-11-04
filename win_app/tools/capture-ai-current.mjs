import{readFile,writeFile,mkdir}from'node:fs/promises';import{createHash}from'node:crypto';import{pathToFileURL}from'node:url';import path from'node:path';import vm from'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),game=path.resolve(root,'../game'),sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(game,p)).href);await mkdir(out,{recursive:true});
const names=['package.json','src/ai.js','src/race.js','src/car.js','src/track.js','src/roads.js','docs/changelog.md','docs/manual/04-flying.md'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(game,p)))})));
const[THREE,{aiInput,cornerSpeeds,CORNER,Racer},{Car,CAR},{Track},{ROADS,rivalTop}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/ai.js'),load('src/car.js'),load('src/track.js'),load('src/roads.js')]);
const text=await readFile(path.join(game,'src/ai.js'),'utf8'),race=await readFile(path.join(game,'src/race.js'),'utf8');
function cut(s,needle){const a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;while(n){if(s[i]==='{')n++;if(s[i]==='}')n--;i++;}return s.slice(a,i).replace('export function','function');}
const limitCtx={cornerSpeeds,CORNER,Math};vm.createContext(limitCtx);vm.runInContext(cut(text,'function cornerLimit(r, track)')+';this.limit=cornerLimit;',limitCtx);
function car(){const c=Object.create(Car.prototype);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),t:0,boost:1,boosting:false,maxMul:1,nitro:0,thrust:0,kick:0,sputter:0,model:{position:{},rotation:{}},pods:[],fans:[],ribbons:[],sync(){}});c.place(new THREE.Vector3(0,118,0),0,0);return c;}
const input=s=>[s.throttle,s.brake,s.steer,s.climb,s.boost],state=c=>[c.pos.toArray(),c.vel.toArray(),c.fwd.toArray(),c.yaw,c.pitch,c.roll,c.speed,c.boost,c.t,c.stun,c.rollT,c.surge,c.maxMul,c.thrust,c.nitro,c.kick,c.sputter,c.boosting];
const routes=[];
for(const road of ROADS){
 const track=new Track(road),corners=[...cornerSpeeds(track)],probes=[],traces=[],factors=[];
 for(let scenario=0;scenario<64;scenario++){
  const c=car(),sample=Math.floor(track.n*scenario/64),f=track.frame(sample);c.pos.copy(f.p).addScaledVector(f.right,(scenario%5-2)*9);c.pos.y+=(scenario%3-1)*7;c.yaw=Math.atan2(-f.t.x,-f.t.z)+(scenario%4-1.5)*.38;c.pitch=(scenario%5-2)*.13;c.fwd.set(-Math.sin(c.yaw)*Math.cos(c.pitch),Math.sin(c.pitch),-Math.cos(c.yaw)*Math.cos(c.pitch));c.speed=[-8,0,7,120,135,196,220,141][scenario%8];c.vel.copy(c.fwd).multiplyScalar(c.speed);c.vel.y+=(scenario%3-1)*20;c.boost=[.35,.35000001,1,0][scenario%4];
  const r={car:c,sample,seed:scenario*.7,lane:scenario%11-5,laneY:scenario%5-2,dodge:scenario%3-1,skill:[.8,.9,1,1.15][scenario%4]},before=[r.sample,r.seed,r.lane,r.laneY,r.dodge,r.skill],peers=[r];
  for(let j=0;j<3;j++){const lateral=j===0?5.5:j===1?-2:1,along=j===0?32:j===1?12:20;peers.push({alive:j!==2||scenario%2===0,car:{pos:c.pos.clone().addScaledVector(c.fwd,along).addScaledVector(f.right,lateral)}});}
  const target=track.checkpoints[scenario%track.checkpoints.length].pos,coin=scenario%3===0?f.p.clone().addScaledVector(f.right,15):null,dt=[1/144,1/60,1/30,.05][scenario%4],time=scenario*1.25,limit=limitCtx.limit(r,track),result=aiInput(r,track,peers,target,time,dt,coin);
  probes.push({driver:before,car:state(c),peers:peers.slice(1).map(p=>[p.car.pos.toArray(),p.alive]),target:target.toArray(),coin:coin?.toArray()??null,time,dt,limit,expected:input(result),dodge:r.dodge});
 }
 for(let scenario=0;scenario<2;scenario++){
  const c=car(),sample=Math.floor(track.n*(scenario?.6:.05)),f=track.frame(sample);c.place(f.p,Math.atan2(-f.t.x,-f.t.z),scenario?135:0);c.maxMul=rivalTop(road.id)/CAR.MAX*(scenario?1.07:1);
  const r={car:c,sample,seed:7.25+scenario,lane:scenario?-4:3,laneY:scenario?1.5:-1,dodge:0,skill:scenario?1.1:.9},initial=state(c),driver=[r.sample,r.seed,r.lane,r.laneY,r.dodge,r.skill],frames=[];
  for(let i=0;i<240;i++){
   r.sample=track.nearestGlobal(c.pos);const target=track.checkpoints[(scenario+Math.floor(i/90))%track.checkpoints.length].pos,coin=i>=50&&i<65?track.pts[(r.sample+12)%track.n]:null,dt=[1/60,1/144,.05,1/30][i%4],time=i/40,limit=limitCtx.limit(r,track),control=aiInput(r,track,[r],target,time,dt,coin);c.update(dt,control,{collide:()=>null});frames.push({sample:r.sample,target:target.toArray(),coin:coin?.toArray()??null,time,dt,limit,input:input(control),dodge:r.dodge,car:state(c)});
  }traces.push({driver,initial,frames});
 }
 for(const delta of[-100000,-250,0,100,100000]){
  const player={sample:400,lap:1,nextCP:2,finished:false,progress:Racer.prototype.progress},rival={sample:400-delta,lap:1,nextCP:2,finished:false,car:{maxMul:1},progress:Racer.prototype.progress},g={racers:[player,rival],rankOf(r){return this.ranking.indexOf(r)+1;}},ctx={g,player,rivals:[rival],track,road,THREE,CAR,rivalTop,RUBBER_BAND:Number(race.match(/export const RUBBER_BAND = ([0-9.]+);/)[1])};vm.createContext(ctx);vm.runInContext(cut(race,'export function updateRanking()')+';updateRanking();',ctx);factors.push([player.progress(track),rival.progress(track),rival.car.maxMul]);
 }
 const extraLimits=[0,track.n-1].map(sample=>[sample,2000,limitCtx.limit({sample,car:{speed:2000}},track)]);routes.push({id:road.id,n:track.n,length:track.length,corners,top:rivalTop(road.id),probes,traces,factors,extraLimits});
}
for(const s of sources){const buf=await readFile(path.join(game,s.path));if(sha(buf)!==s.sha256)throw Error('source changed '+s.path);const dest=path.join(out,'source-snapshot',s.path);await mkdir(path.dirname(dest),{recursive:true});await writeFile(dest,buf);}
const summary={routes:routes.length,samples:routes.reduce((n,r)=>n+r.n,0),probes:routes.reduce((n,r)=>n+r.probes.length,0),frames:routes.reduce((n,r)=>n+r.traces.reduce((m,t)=>m+t.frames.length,0),0),factors:routes.reduce((n,r)=>n+r.factors.length,0),maxSamples:Math.max(...routes.map(r=>r.n))};await writeFile(path.join(out,'ai-current-reference.json'),JSON.stringify({version:JSON.parse(await readFile(path.join(game,'package.json'))).version,sources,CORNER,CAR,summary,routes})+'\n');console.log(summary);
