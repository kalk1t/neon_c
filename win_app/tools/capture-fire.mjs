import {readFile,writeFile} from 'node:fs/promises';import {createHash} from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';import vm from 'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game'),sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const names=['package.json','src/main.js','src/fx.js','src/audio.js','src/ai.js','src/car.js','node_modules/three/build/three.core.js'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const [THREE,FX,{Car},{WEAPON,DAMAGE}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/fx.js'),load('src/car.js'),load('src/ai.js')]);
let rngState=1,draws=0;const random=()=>{rngState=(Math.imul(rngState,1664525)+1013904223)>>>0;draws++;return rngState/4294967296;},seed=x=>{rngState=x>>>0;draws=0;};const math=Object.create(Math);math.random=random;
const fxSrc=await readFile(path.join(browser,'src/fx.js'),'utf8'),audioSrc=await readFile(path.join(browser,'src/audio.js'),'utf8'),main=await readFile(path.join(browser,'src/main.js'),'utf8');
const fxCtx={THREE,axisBillboard:FX.axisBillboard,additive:FX.additive,tex:()=>null,Math:math,_rel:new THREE.Vector3(),_dir:new THREE.Vector3(),_seg:new THREE.Vector3()};vm.createContext(fxCtx);vm.runInContext(fxSrc.slice(fxSrc.indexOf('export class Weapons')).replace('export class Weapons','class Weapons')+';this.Weapons=Weapons;',fxCtx);
const audioCtx={Math:math};vm.createContext(audioCtx);vm.runInContext(audioSrc.replace('export class Sound','class Sound')+';this.Sound=Sound;',audioCtx);
// Warm shared Sprite geometry and cached texture stand-in before measured stream.
new THREE.Sprite(new THREE.SpriteMaterial());
const makeWeapons=()=>new fxCtx.Weapons(new THREE.Scene(),{segment:()=>null});
const kinds=['shot','bolt','missile','emp'];
function actors(){return [0,1,2].map(i=>{const c={pos:new THREE.Vector3(),vel:new THREE.Vector3(),root:new THREE.Object3D(),fwd:new THREE.Vector3(0,0,-1),speed:95,stun:0,rollT:0,surge:0,rollDir:1,yaw:0,barrelRoll:Car.prototype.barrelRoll,dodge:Car.prototype.dodge,muzzle:Car.prototype.muzzle,setVisible(v){this.visible=v;}};Object.defineProperty(c,'dodging',{get(){return this.rollT>0;}});const r={car:c,isPlayer:i===0,hp:100,wreck:0,finished:false,fireCD:0};Object.defineProperty(r,'alive',{get(){return this.wreck<=0;}});return r;});}
function reset(rs){rs.forEach((r,i)=>{Object.assign(r,{hp:i===1?20:100,wreck:0,finished:false,fireCD:0});Object.assign(r.car,{speed:i===0?95:0,stun:0,rollT:0,surge:0,rollDir:1,yaw:0,visible:true});r.car.pos.set(i===2?20:0,100,i===0?0:i===1?-60:-80);r.car.vel.set(0,0,i===0?-95:0);r.car.root.position.copy(r.car.pos);r.car.root.updateMatrixWorld(true);});}
const actorOut=r=>[r.car.pos.toArray(),r.car.vel.toArray(),r.car.speed,r.car.stun,r.car.rollT,r.car.surge,r.car.yaw,r.hp,r.wreck,r.fireCD,r.finished,r.isPlayer,r.car.visible];
const shotOut=(s,rs)=>[rs.indexOf(s.owner),rs.indexOf(s.target),s.pos.toArray(),s.vel.toArray(),s.speed,s.accel,s.maxSpeed,s.life,s.turn,s.damage??0,s.radius,s.trailT,kinds.indexOf(s.kind),s.always,s.trail,s.damage!==undefined];
const state=g=>[g.heat,g.boltCD,g.missiles,g.spinCD,g.empCD,g.overheated,g.side];
const fires=[];const rs=actors();reset(rs);const wi=makeWeapons();
const nativeRandom=Math.random;try {Math.random=random;
for(let i=0;i<96;i++){const o={accel:i%3?0:420,maxSpeed:i%5?0:300,len:i%4?0:24,wid:i%6?0:5,radius:i%7?0:3.6,kind:kinds[i%4],trail:!!(i&1),always:!!(i&2)};if(i&4)o.speed=[0,60,430][i%3];if(i&8)o.life=i%2?0:1.3;if(i&16)o.turn=i%2?0:2.5;if(i&32)o.damage=i%2?0:48;const owner=i%2;seed(i+1);const before=draws,s=wi.fire(rs[owner],new THREE.Vector3(1,100,-3),new THREE.Vector3(i%9===0?0:.03,i%9===0?0:.08,i%9===0?0:-1),i%3===0?-10:95,i%2?rs[2]:null,o);fires.push({owner,inherit:i%3===0?-10:95,target:i%2?2:-1,dir:[i%9===0?0:.03,i%9===0?0:.08,i%9===0?0:-1],options:[o.speed??0,o.accel,o.maxSpeed,o.life??0,o.turn??0,o.damage??0,o.len,o.wid,o.radius,'speed'in o,'life'in o,'turn'in o,'damage'in o,o.always,o.trail,i%4],expected:shotOut(s,rs),length:s.len,width:s.wid,draws:draws-before});wi.clear();}
const fxCases=[];for(const initial of[0,260,261,299,300])for(let kind=0;kind<7;kind++){const w=makeWeapons();w.fx=Array.from({length:initial},()=>({s:new THREE.Sprite(new THREE.SpriteMaterial()),t:.1,life:1,size:1,rise:0,spin:0}));seed(123+initial+kind);const p=new THREE.Vector3(0,100,0);if(kind===0)w.flash(p);if(kind===1)w.impact(p);if(kind===2)w.impact(p,true);if(kind===3)w.shockwave(p,95);if(kind===4)w.explode(p);if(kind===5)w.smoke(p);if(kind===6&&w.fx.length<300){w._sprite(null,new THREE.Color(),THREE.NormalBlending,p,2.2,.7,.6);w._sprite(null,new THREE.Color(),THREE.AdditiveBlending,p,3,.18);}w.update(.05,new THREE.Vector3());fxCases.push({initial,kind,seed:123+initial+kind,draws,rng:rngState,fx:w.fx.map(f=>[f.t,f.life])});}
const traces=[];
for(const traceSeed of[12345,987654321]){
 const racers=actors();reset(racers);const player=racers[0],car=player.car,weapons=makeWeapons(),sound=new audioCtx.Sound(),g={side:1,heat:0,boltCD:0,missiles:3,spinCD:0,empCD:0,overheated:false,lock:null,lockT:0,aimDir:new THREE.Vector3(0,0,-1),racers,remotes:[],mode:'race',state:'race',shake:0,kick:0};
 let inp={},hit={};const ctx={THREE,WEAPON,DAMAGE,SPIN:{COOLDOWN:6},g,player,car,rivals:racers.slice(1),weapons,sound,Math:math,live:null,hud:new Proxy({},{get:()=>()=>{}}),volAt(){return 0;},panAt(){return 0;},dodged(){},input:{hit(...ks){return ks.some(k=>hit[k]);}},_d:new THREE.Vector3(),_tmp:new THREE.Vector3(),_side:new THREE.Vector3()};vm.createContext(ctx);
 const da=main.indexOf('const KNOCK ='),raw=main.slice(da,main.indexOf('function completeLap',da)),cut=raw.indexOf('  if (attacker === player && !r.isPlayer)');vm.runInContext(raw.slice(0,cut)+'}\n',ctx);
 const ta=main.indexOf('function aimAngle('),tb=main.indexOf('const _side =',ta);vm.runInContext(main.slice(ta,tb),ctx);
 const pa=main.indexOf('function playerWeapons('),pb=main.indexOf('// Debug/demo pilot',pa);vm.runInContext(main.slice(pa,pb),ctx);
 weapons.onHit=(shot,r)=>ctx.damage(r,shot.damage??(r.isPlayer?DAMAGE.RIVAL_SHOT_ON_PLAYER:DAMAGE.RIVAL_SHOT),shot.owner,shot.kind);
 const resetStatement=main.match(/Object\.assign\(g, \{ heat: 0, overheated: false, boltCD: 0,[^\n]+\}\);/)[0];
 weapons.racers=racers;
 const frames=[];seed(traceSeed);let worldCalls=0;
 for(let frame=0;frame<240;frame++){
  const restart=frame===90||frame===170;if(restart){reset(racers);vm.runInContext(resetStatement,ctx);weapons.clear();}
  const dt=[.01,1/60,1/30,.05][frame%4];for(const r of racers){r.car.rollT=Math.max(0,r.car.rollT-dt);r.car.surge=Math.max(0,r.car.surge-dt);}
  worldCalls=0;const worldHit=frame%53===0;weapons.colliders.segment=()=>{worldCalls++;return worldHit?new THREE.Vector3(3,80,-20):null;};
  const inject=frame%19===0;if(inject)weapons.fire(racers[2],racers[2].car.pos,new THREE.Vector3(0,0,1),0,player,{});
  const smoke=frame%41===0;if(smoke)weapons.smoke(racers[2].car.pos);
  inp={fire:frame%100<70,steer:frame%3===0?0:frame%2?-.5:.5};hit={Space:frame%31===0,KeyF:frame%83===0,KeyX:frame%97===0};
  const active=player.alive;if(active){ctx.updateLock(dt);ctx.playerWeapons(dt,inp);}
  weapons.update(dt,new THREE.Vector3(70,130,80));
  frames.push({dt,restart,inject,smoke,worldHit,active,input:[inp.fire,hit.Space,hit.KeyF,hit.KeyX,inp.steer],rng:rngState,draws,worldCalls,arsenal:state(g),lock:racers.indexOf(g.lock),charge:g.lockT,actors:racers.map(actorOut),shots:weapons.shots.map(s=>shotOut(s,racers)),fx:weapons.fx.map(f=>[f.t,f.life])});
 }traces.push({seed:traceSeed,frames});
}
for(const s of sources)if(sha(await readFile(path.join(browser,s.path)))!==s.sha256)throw Error('source changed');
await writeFile(out,JSON.stringify({sources,scope:'Actual source fire/defaults, FX class and playerWeapons/targeting/projectile/damage-nonreward sequence. Pinned Three, prewarmed Sprite geometry and cached-texture stand-in; actual Sound ctx=null. No renderer/audio device/window, movement/AI/pack/world geometry/rewards/network.',fires,fxCases,traces},null,2)+'\n');console.log({fires:fires.length,fxCases:fxCases.length,frames:480,maxShots:Math.max(...traces.flatMap(t=>t.frames.map(f=>f.shots.length))),maxFx:Math.max(...traces.flatMap(t=>t.frames.map(f=>f.fx.length))),draws:traces.map(t=>t.frames.at(-1).draws)});
}finally{Math.random=nativeRandom;}
