import {readFile,writeFile} from 'node:fs/promises';import {createHash} from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';import vm from 'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game'),sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const names=['package.json','src/main.js','src/fx.js','src/ai.js','src/car.js','src/physics.js'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const [THREE,{axisBillboard},{Car},{WEAPON,DAMAGE}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/fx.js'),load('src/car.js'),load('src/ai.js')]);
const fxSource=await readFile(path.join(browser,'src/fx.js'),'utf8');
const numericMath=Object.create(Math);numericMath.random=()=>{throw Error('unexpected numerical RNG demand');};
const fxCtx={THREE,axisBillboard,tex:()=>null,Math:numericMath,_rel:new THREE.Vector3(),_dir:new THREE.Vector3(),_seg:new THREE.Vector3()};vm.createContext(fxCtx);
vm.runInContext(fxSource.slice(fxSource.indexOf('export class Weapons')).replace('export class Weapons','class Weapons')+';this.Weapons=Weapons;',fxCtx);const Weapons=fxCtx.Weapons;
const main=await readFile(path.join(browser,'src/main.js'),'utf8'),a=main.indexOf('const KNOCK ='),b=main.indexOf('function completeLap',a),raw=main.slice(a,b),cut=raw.indexOf('  if (attacker === player && !r.isPlayer)');
if(a<0||b<0||cut<0)throw Error('damage hooks changed');
// Execute unchanged damage body; annotate acceptance/dodge bits. Wreck prefix
// stops explicitly before takedown/reward branch; visual/audio functions are sinks.
const damageSource=raw.slice(0,cut).replace('r.hp -= amount;','flags |= 1; r.hp -= amount;').replace('if (r.car.dodging) {','if (r.car.dodging) { flags |= 2;')+'flags |= 4;\n}\nthis.realDamage=damage;';
let flags=0,roster=[],notices=[];const g={state:'race',mode:'race',shake:0,lock:null,racers:[],remotes:[]},player={};
const silent=new Proxy({}, {get:()=>()=>{}});const ctx={g,player,WEAPON,DAMAGE,flags:0,weapons:{explode(){}},sound:silent,hud:silent,dodged(){},volAt(){return 0;},panAt(){return 0;},_d:new THREE.Vector3(),W:WEAPON,car:{pos:new THREE.Vector3()}};
vm.createContext(ctx);vm.runInContext(damageSource,ctx);
const apply=(r,amount,kind)=>{ctx.flags=0;ctx.realDamage(r,amount,null,kind);flags=ctx.flags;return flags;};
ctx.damage=(r,amount,attacker,kind)=>{const f=apply(r,amount,kind);notices.push([roster.indexOf(r),f]);};
const empStart=main.indexOf('    for (const r of g.racers)',main.indexOf('// EMP: hits everyone nearby')),empEnd=main.indexOf('    hud.sub(`EMP',empStart);if(empStart<0||empEnd<0)throw Error('EMP hooks changed');
vm.runInContext('this.emp=function(){let n=0;'+main.slice(empStart,empEnd)+'return n;}',ctx);
const onA=main.indexOf('weapons.onHit ='),onB=main.indexOf('function onCoin',onA);vm.runInContext(main.slice(onA,onB),ctx);
const vec=a=>new THREE.Vector3(...a),kind=['shot','bolt','missile','emp'];
// Actor wire form: pos,vel,speed,stun,roll,surge,yaw,hp,wreck,fireCD,finished,player,visible.
function actor(v){const c={pos:vec(v[0]),vel:vec(v[1]),speed:v[2],stun:v[3],rollT:v[4],surge:v[5],yaw:v[6],setVisible(x){this.visible=x;},visible:v[12]};Object.defineProperty(c,'dodging',{get(){return this.rollT>0;}});const r={car:c,hp:v[7],wreck:v[8],fireCD:v[9],finished:v[10],isPlayer:v[11]};Object.defineProperty(r,'alive',{get(){return this.wreck<=0;}});return r;}
const actorOut=r=>[r.car.pos.toArray(),r.car.vel.toArray(),r.car.speed,r.car.stun,r.car.rollT,r.car.surge,r.car.yaw,r.hp,r.wreck,r.fireCD,r.finished,r.isPlayer,r.car.visible];
const base=(p=[0,100,0])=>[p,[5,1,-7],80,0,0,0,0,100,0,.4,false,false,true];
const damages=[];for(let k=0;k<4;k++)for(const hp of[0,6,25,48,100])for(const amount of[0,6,25,48,101])for(let mask=0;mask<16;mask++){
 const v=base();v[7]=hp;v[8]=mask&1?2.4:0;v[10]=!!(mask&2);v[4]=mask&4?.01:0;v[3]=mask%3;v[11]=!!(mask&8);const r=actor(v);g.state=mask&8?'race':'finished';const f=apply(r,amount,kind[k]);damages.push({before:v,amount,kind:k,active:g.state==='race',after:actorOut(r),flags:f});
}
const defaults=[];for(const isPlayer of[false,true])for(const amount of[null,0,6,48]){const v=base();v[11]=isPlayer;const r=actor(v);g.state='race';roster=[r];notices=[];ctx.weapons.onHit({damage:amount,kind:'shot',owner:null},r);defaults.push({before:v,amount:amount??0,has_damage:amount!==null,after:actorOut(r),flags:notices[0][1]});}
const emps=[];for(const distance of[0,94.999999,95,95.000001])for(let mask=0;mask<16;mask++){
 const before=Array.from({length:4},(_,i)=>{const v=base([i===3?distance:distance/2,100,i===3?0:i*5]);v[11]=i===0;v[10]=i===1&&!!(mask&1);v[8]=i===2&&(mask&2)?1:0;v[4]=mask&4?.1:0;v[7]=mask&8?8:100;return v;});roster=before.map(actor);g.racers=roster;g.state='race';ctx.car.pos.set(0,100,0);notices=[];const count=ctx.emp();emps.push({before,after:roster.map(actorOut),count,notices});
}
// Existing dodge/flight code: verify actual Car.dodge, expiry tick and Car.place reset.
const dodges=[];for(const side of[-1,1])for(const speed of[-18,0,47.5,95,138])for(const yaw of[-2,0,1.2])for(const dt of[0,.549999,.55,.550001,2]){
 const c=Object.create(Car.prototype);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(2,3,4),speed,yaw,rollT:0,surge:0,rollDir:1});c.dodge(side);const immediately=[c.speed,...c.vel.toArray(),c.rollT,c.surge,c.rollDir];
 // Actual timer statements from Car.update; isolate from movement/render coupling.
 const carSrc=await readFile(path.join(browser,'src/car.js'),'utf8'),timer=carSrc.match(/this\.rollT = Math\.max\(0, this\.rollT - dt\);\s*this\.surge = Math\.max\(0, this\.surge - dt\);/)[0];let remaining=dt;while(remaining>0){const tick=Math.min(.05,remaining);new Function('dt',timer).call(c,tick);remaining-=tick;}dodges.push({side,speed,yaw,dt,immediately,after:[c.rollT,c.surge],dodging:c.dodging});
}
const shots=[];
const shotBase=(serial=0)=>({owner:0,target:-1,pos:[0,100,0],vel:[0,0,-100],speed:100,accel:0,max_speed:400,life:2,turn:2.5,damage:6,radius:3.2,trail_time:0,kind:1,always:false,trail:false,has_damage:true,serial});
function makeShot(s,rs){return {owner:s.owner<0?null:rs[s.owner],target:s.target<0?null:rs[s.target],pos:vec(s.pos),vel:vec(s.vel),speed:s.speed,accel:s.accel,maxSpeed:s.max_speed,life:s.life,turn:s.turn,damage:s.has_damage?s.damage:undefined,radius:s.radius,trailT:s.trail_time,kind:kind[s.kind],always:s.always,trail:s.trail,len:24,wid:5,m:new THREE.Object3D(),serial:s.serial};}
const shotOut=(s,rs)=>({owner:rs.indexOf(s.owner),target:rs.indexOf(s.target),pos:s.pos.toArray(),vel:s.vel.toArray(),speed:s.speed,accel:s.accel,max_speed:s.maxSpeed,life:s.life,turn:s.turn,damage:s.damage??0,radius:s.radius,trail_time:s.trailT,kind:kind.indexOf(s.kind),always:s.always,trail:s.trail,has_damage:s.damage!==undefined,serial:s.serial});
function runShot(before,actors,dt,worldHit=false,floor=24,purge=false){const rs=actors.map(actor),events=[];let worldCalls=0,current=null;const w=Object.create(Weapons.prototype);Object.assign(w,{shots:before.map(s=>makeShot(s,rs)),racers:rs,fx:[],waves:[],boomT:0,light:{position:new THREE.Vector3()},boom:{},scene:{remove(){}},floor,colliders:{segment(a,b){worldCalls++;return worldHit?new THREE.Vector3(7,88,9):null;}}});
 // Hook current shot from actual axisBillboard matrix call, then trail/impact sinks.
 for(const s of w.shots){const make=s.m.matrix.makeBasis.bind(s.m.matrix);s.m.matrix.makeBasis=(...a)=>{current=s;return make(...a);};}
 w._sprite=()=>{if(current&&events.at(-1)?.event!==1)events.push({event:1,serial:current.serial,victim:-1,point:current.pos.toArray()});};
 w.impact=(point)=>events.push({event:purge?4:rs.some(r=>r.car.pos===point)?2:3,serial:purge?-1:current.serial,victim:rs.findIndex(r=>r.car.pos===point),point:point.toArray()});
 w.onHit=(s,r)=>{apply(r,s.damage??(r.isPlayer?DAMAGE.RIVAL_SHOT_ON_PLAYER:DAMAGE.RIVAL_SHOT),s.kind);};
 let purged=0;if(purge){let pi=before.length-1;w.impact=point=>{while(pi>=0&&(w.shots[pi].owner===rs[0]||w.shots[pi].pos.distanceTo(vec([0,100,0]))>95))pi--;events.push({event:4,serial:w.shots[pi--].serial,victim:-1,point:point.toArray()});};purged=w.purge(vec([0,100,0]),95,rs[0]);}else w.update(dt,new THREE.Vector3(70,130,80));
 shots.push({before,actors,dt,worldHit,floor,purge,purged,worldCalls,after:w.shots.map(s=>shotOut(s,rs)),afterActors:rs.map(actorOut),events});
}
// Swept-sphere strict boundary; roster priority vs nearest, self/dead/finished exclusions.
for(const distance of[0,3.199999999,3.2,3.200000001])for(let mask=0;mask<16;mask++){
 const rs=[base([0,100,-2]),base([distance,100,-8]),base([0,100,-3])];rs[1][8]=mask&1?1:0;rs[1][10]=!!(mask&2);rs[2][4]=mask&4?.1:0;const s=shotBase();s.life=mask&8?0:.05;s.trail=true;g.state='race';runShot([s],rs,.1,true);
}
// Acceleration + behind/live target subtlety, finished targets still guide, dead target accelerates vector.
for(let i=0;i<192;i++){const rs=[base([1000,100,1000]),base([Math.sin(i)*80,100+i%7,i%2?100:-100])];rs[1][8]=i%4===0?1:0;rs[1][10]=i%3===0;const s=shotBase(i);s.target=i%5===0?-1:1;s.accel=420;s.always=i%2===0;s.trail=i%3===0;s.vel=i%17===0?[0,0,0]:[0,0,-100];s.life=i%11===0?0:2;runShot([s],rs,[0,.016,.05,.5][i%4]);}
// Reverse shot order and immediate damage/wreck eligibility; same-tick expiry still hits.
for(let i=0;i<24;i++){const rs=[base([1000,100,1000]),base([0,100,-5]),base([0,100,-6])];rs[1][7]=6;const ss=[shotBase(0),shotBase(1),shotBase(2)];ss[1].damage=101;ss[2].life=0;ss[0].trail=true;runShot(ss,rs,.1,i%2===0);}
// World callback/floor ordering, equality, zero segment and zero-life.
for(const y of[23.999999,24,24.000001])for(const dt of[0,.05])for(const wh of[false,true]){const s=shotBase();s.pos=[0,y,0];s.vel=[0,0,0];s.life=0;runShot([s],[base([1000,100,0])],dt,wh);}
for(let i=0;i<16;i++){const rs=[base(),base([1000,100,0])],ss=[];for(let j=0;j<5;j++){const s=shotBase(j);s.owner=j%2;s.pos=[[0,100,0],[95,100,0],[95.00000001,100,0],[94.9,100,0],[0,100,0]][j];ss.push(s);}runShot(ss,rs,0,false,24,true);}
for(const s of sources)if(sha(await readFile(path.join(browser,s.path)))!==s.sha256)throw Error('source changed');
await writeFile(out,JSON.stringify({sources,scope:'Actual Weapons.update/purge, instrumented main damage and nonreward wreck prefix, actual EMP loop, actual Car.dodge/timer statements. Render/audio/FX sinks; no full visual RNG, account rewards, projectile creation/defaults or production activation.',constants:{WEAPON,DAMAGE},damages,defaults,emps,dodges,shots},null,2)+'\n');console.log({damages:damages.length,defaults:defaults.length,emps:emps.length,dodges:dodges.length,shotCases:shots.length});
