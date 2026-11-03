// Actual Coins CPU constructor prefix and methods with render-only sinks.
import{readFile,writeFile}from'node:fs/promises';import{createHash}from'node:crypto';import{pathToFileURL}from'node:url';import path from'node:path';import vm from'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game'),sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const names=['package.json','src/coins.js','src/ai.js','src/track.js','src/roads.js','src/main.js'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const[THREE,{Coins},{COIN},{Track},{ROADS}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/coins.js'),load('src/ai.js'),load('src/track.js'),load('src/roads.js')]);
const source=await readFile(path.join(browser,'src/coins.js'),'utf8'),a=source.indexOf('    this.track = track;'),b=source.indexOf("    // the shop's gold wheel coin",a);if(a<0||b<0)throw Error('source hook changed');
const context={THREE,COIN};vm.createContext(context);vm.runInContext('this.make=function(scene,track){'+source.slice(a,b)+'};',context);
function field(track){const c=Object.create(Coins.prototype);context.make.call(c,new THREE.Group(),track);c.mesh={setMatrixAt(){},instanceMatrix:{},visible:true};c.halos=c.list.map(()=>({visible:true,scale:{setScalar(){}}}));c.calls=[];c.onCollect=(r,coin)=>c.calls.push({coin:c.list.indexOf(coin),racer:c.racers.indexOf(r)});return c;}
const layout=c=>({hidden:!!c.hidden,list:c.list.map(x=>({pos:x.pos.toArray(),sample:x.sample,active:x.active,timer:x.timer,pop:x.pop}))});
const state=c=>({hidden:!!c.hidden,list:c.list.map(x=>({active:x.active,timer:x.timer,pop:x.pop}))});
const peer=(p,coins=0,alive=true,finished=false)=>({car:{pos:new THREE.Vector3(...p),fwd:new THREE.Vector3(0,0,-1)},coins,alive,finished});
const ps=r=>({pos:r.car.pos.toArray(),fwd:r.car.fwd.toArray(),coins:r.coins,alive:r.alive,finished:r.finished});
const layouts=[],ahead=[],updates=[],traces=[],resets=[];
for(const road of ROADS){const track=new Track(road),c=field(track);layouts.push({road:road.id,...layout(c)});
 for(let k=0;k<36;k++){const index=k%c.list.length,coin=c.list[index];const r=peer(coin.pos.clone().add(new THREE.Vector3((k%3-1)*8,0,30)).toArray());if(k%5===0)c.list[index].active=false;const max=k%7===0?25:90;const best=c.ahead(r,max);ahead.push({road:road.id,inactive:k%5===0?index:-1,racer:ps(r),max,best:best?c.list.findIndex(x=>x.pos===best):-1});c.list[index].active=true;}
 for(let k=0;k<12;k++){
  c.reset();c.setVisible(k%2===0);const p=c.list[k%c.list.length].pos.clone();const racers=[peer(p.toArray(),3,k%4!==0,k%5===0),peer(p.toArray(),9,true,false),peer(p.clone().add(new THREE.Vector3(6.5,0,0)).toArray(),1)];c.racers=racers;
  if(k%3===0){c.list[k%c.list.length].active=false;c.list[k%c.list.length].timer=[0,.05,6][k%3];c.list[k%c.list.length].pop=.8;}
  const dt=[0,.05,6][k%3],active=k%4!==1,before=state(c),beforeRacers=racers.map(ps);c.calls=[];c.update(dt,1.7,racers,active);updates.push({road:road.id,before,racers:beforeRacers,dt,active,after:state(c),after_racers:racers.map(ps),events:c.calls});
 }
 c.reset();c.setVisible(false);for(let i=0;i<c.list.length;i++){c.list[i].active=i%2===0;c.list[i].timer=.3+i*.01;c.list[i].pop=.7;}const before=state(c);c.reset();resets.push({road:road.id,before,after:state(c)});
 c.reset();c.setVisible(true);const racers=[peer(c.list[0].pos.toArray()),peer(c.list[1].pos.toArray(),0,true,false)];c.racers=racers;const initial=state(c),initialRacers=racers.map(ps),frames=[];
 for(let k=0;k<160;k++){const dt=[1/30,.05,.05,.05][k%4];if(k===3||k===115)for(const r of racers)r.car.pos.set(100000,100000,100000);if(k===130)racers[0].car.pos.copy(c.list[0].pos);c.calls=[];const beforeRacers=racers.map(ps);c.update(dt,k*.03,racers,k<150);frames.push({dt,active:k<150,racers:beforeRacers,after:state(c),after_racers:racers.map(ps),events:c.calls});}
 traces.push({road:road.id,before:initial,racers:initialRacers,frames});
}
// Source tie order, exact collection radius, inclusive ahead thresholds and respawn-frame rule.
const c=field(new Track(ROADS[0]));c.list=[{pos:new THREE.Vector3(-10,0,-30),sample:0,active:true,timer:0,pop:0},{pos:new THREE.Vector3(10,0,-30),sample:1,active:true,timer:0,pop:0}];c.halos=c.list.map(()=>({scale:{setScalar(){}}}));const r=peer([0,0,0]);const synthetic_ahead=[];
for(const max of[90,Math.sqrt(1000),31,0]){const best=c.ahead(r,max);synthetic_ahead.push({max,best:best?c.list.findIndex(x=>x.pos===best):-1});}
for(const s of sources)if(sha(await readFile(path.join(browser,s.path)))!==s.sha256)throw Error('source changed');
await writeFile(out,JSON.stringify({sources,scope:'Actual source coin construction/placement and Coins.ahead/reset/setVisible/update methods; render-only matrix/halo sinks, no texture load, GPU, account or native activation.',layouts,ahead,updates,traces,resets,synthetic_ahead},null,2)+'\n');console.log({layouts:layouts.length,coins:layouts.reduce((n,x)=>n+x.list.length,0),max:Math.max(...layouts.map(x=>x.list.length)),ahead:ahead.length,updates:updates.length,traces:traces.length,frames:traces.reduce((n,x)=>n+x.frames.length,0),resets:resets.length});
