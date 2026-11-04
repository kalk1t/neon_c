import {readFile,writeFile} from 'node:fs/promises';import {createHash} from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';import vm from 'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game'),sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const names=['package.json','src/main.js','src/aim.js','src/race.js','src/fx.js','src/audio.js','src/ai.js','src/car.js','node_modules/three/build/three.core.js'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const [THREE,FX,{Car,CAR_MODELS},{WEAPON,DAMAGE}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/fx.js'),load('src/car.js'),load('src/ai.js')]);
let rngState=1,draws=0;const random=()=>{rngState=(Math.imul(rngState,1664525)+1013904223)>>>0;draws++;return rngState/4294967296;},seed=x=>{rngState=x>>>0;draws=0;};const math=Object.create(Math);math.random=random;
const aimSrc=await readFile(path.join(browser,'src/aim.js'),'utf8'),raceSrc=await readFile(path.join(browser,'src/race.js'),'utf8');
const fxSrc=await readFile(path.join(browser,'src/fx.js'),'utf8'),audioSrc=await readFile(path.join(browser,'src/audio.js'),'utf8'),main=await readFile(path.join(browser,'src/main.js'),'utf8');
const fxCtx={THREE,axisBillboard:FX.axisBillboard,additive:FX.additive,tex:()=>null,Math:math,_rel:new THREE.Vector3(),_dir:new THREE.Vector3(),_seg:new THREE.Vector3()};vm.createContext(fxCtx);vm.runInContext(fxSrc.slice(fxSrc.indexOf('export class Weapons')).replace('export class Weapons','class Weapons')+';this.Weapons=Weapons;',fxCtx);
const audioCtx={Math:math};vm.createContext(audioCtx);vm.runInContext(audioSrc.replace('export class Sound','class Sound')+';this.Sound=Sound;',audioCtx);

function functionSource(text,needle){const a=text.indexOf(needle);if(a<0)throw Error('Missing '+needle);let i=text.indexOf('{',a)+1,n=1;while(n){if(text[i]==='{')n++;if(text[i]==='}')n--;i++;}return text.slice(a,i).replace('export function','function');}
if(WEAPON.BOLT_DMG!==21||WEAPON.BOLT_RATE!==.21||WEAPON.HEAT_PER_BOLT!==.08||WEAPON.LOCK_RANGE!==2000)throw Error('not owner v0.2.14 rules');

// Warm shared Sprite geometry and cached texture stand-in before measured stream.
new THREE.Sprite(new THREE.SpriteMaterial());
const makeWeapons=()=>new fxCtx.Weapons(new THREE.Scene(),{segment:()=>null});
const kinds=['shot','bolt','missile','emp'];
function actors(){return [0,1,2].map(i=>{const c={pos:new THREE.Vector3(),vel:new THREE.Vector3(),root:new THREE.Object3D(),fwd:new THREE.Vector3(0,0,-1),speed:95,stun:0,rollT:0,surge:0,rollDir:1,yaw:0,barrelRoll:Car.prototype.barrelRoll,dodge:Car.prototype.dodge,muzzle:Car.prototype.muzzle,model:new THREE.Object3D(),t:0,pitch:0,roll:0,nitro:0,kick:0,boost:1,setVisible(v){this.visible=v;}};c.root.add(c.model);Object.defineProperty(c,'dodging',{get(){return this.rollT>0;}});const r={car:c,isPlayer:i===0,hp:100,wreck:0,finished:false,fireCD:0};Object.defineProperty(r,'alive',{get(){return this.wreck<=0;}});return r;});}
function reset(rs){rs.forEach((r,i)=>{Object.assign(r,{hp:i===1?20:100,wreck:0,finished:false,fireCD:0});Object.assign(r.car,{speed:i===0?95:0,stun:0,rollT:0,surge:0,rollDir:1,yaw:0,visible:true});r.car.pos.set(i===2?20:0,100,i===0?0:i===1?-60:-80);r.car.vel.set(0,0,i===0?-95:0);r.car.root.position.copy(r.car.pos);r.car.root.updateMatrixWorld(true);});}
const actorOut=r=>[r.car.pos.toArray(),r.car.vel.toArray(),r.car.speed,r.car.stun,r.car.rollT,r.car.surge,r.car.yaw,r.hp,r.wreck,r.fireCD,r.finished,r.isPlayer,r.car.visible];
const shotOut=(s,rs)=>[rs.indexOf(s.owner),rs.indexOf(s.target),s.pos.toArray(),s.vel.toArray(),s.speed,s.accel,s.maxSpeed,s.life,s.turn,s.damage??0,s.radius,s.trailT,kinds.indexOf(s.kind),s.always,s.trail,s.damage!==undefined,s.len,s.wid,s.flown,s.kind==='bolt'&&s.owner.isPlayer?s.m.material.color.toArray():[0,0,0],s.kind==='bolt'&&s.owner.isPlayer];
const state=g=>[g.heat,g.boltCD,g.missiles,g.spinCD,g.empCD,g.overheated,g.side];
const nativeRandom=Math.random;try{Math.random=random;
const traces=[];
for(const [model,traceSeed] of [12345,987654321,31415926,27182818].entries()){
 const racers=actors();reset(racers);racers[0].car.muzzleAt=model?CAR_MODELS[['roadster','viper','titan'][model-1]].muzzle:undefined;const player=racers[0],car=player.car,weapons=makeWeapons(),sound=new audioCtx.Sound(),g={side:1,heat:0,boltCD:0,missiles:3,spinCD:0,empCD:0,overheated:false,lock:null,lockT:0,aimDir:new THREE.Vector3(0,0,-1),racers,remotes:[],mode:'race',state:'race',shake:0,kick:0};
 const accent=[0xff26d4,0xffa020,0x9a5cff,0x3a8cff][model];player.shotColor=new THREE.Color(accent).lerp(new THREE.Color(1,1,1),.35).multiplyScalar(3);player.shotMat=new THREE.MeshBasicMaterial({color:player.shotColor});let flashes=[];const originalFlash=weapons.flash.bind(weapons);weapons.flash=(pos,color)=>{originalFlash(pos,color);flashes.push([pos.toArray(),color.toArray(),draws,rngState]);};
 let inp={},hit={};const ctx={THREE,WEAPON,DAMAGE,SPIN:{COOLDOWN:6},g,player,car,rivals:racers.slice(1),weapons,sound,Math:math,live:null,hud:new Proxy({},{get:()=>()=>{}}),volAt(){return 0;},panAt(){return 0;},dodged(){},input:{hit(...ks){return ks.some(k=>hit[k]);}},_d:new THREE.Vector3(),_tmp:new THREE.Vector3(),_side:new THREE.Vector3()};vm.createContext(ctx);
 const knock=raceSrc.match(/const KNOCK = [^;]+;/)[0];let wreck=functionSource(raceSrc,'export function wreck(r, attacker)');const ra=wreck.indexOf('  if (attacker === player && !r.isPlayer)');let ri=wreck.indexOf('{',ra)+1,rn=1;while(rn){if(wreck[ri]==='{')rn++;if(wreck[ri]==='}')rn--;ri++;}wreck=wreck.slice(0,ra)+wreck.slice(ri);
 vm.runInContext(knock+'\n'+functionSource(raceSrc,'export function damage(r, amount, attacker')+'\n'+wreck,ctx);
 vm.runInContext(functionSource(aimSrc,'function aimAngle(r, maxDist)')+'\n'+functionSource(aimSrc,'export function updateLock(dt)')+'\n'+functionSource(aimSrc,'function leadDir(from, r, speed)')+'\n'+functionSource(aimSrc,'export function playerWeapons(dt, inp)'),ctx);
 weapons.onHit=(shot,r)=>ctx.damage(r,shot.damage??(r.isPlayer?DAMAGE.RIVAL_SHOT_ON_PLAYER:DAMAGE.RIVAL_SHOT),shot.owner,shot.kind);
 const resetStatement=main.match(/Object\.assign\(g, \{ heat: 0, overheated: false, boltCD: 0,[^\n]+\}\);/)[0];
 weapons.racers=racers;
 const frames=[];seed(traceSeed);let worldCalls=0;
 for(let frame=0;frame<180;frame++){
  const restart=frame===60||frame===120;if(restart){reset(racers);vm.runInContext(resetStatement,ctx);weapons.clear();}
  const dt=[.01,1/60,1/30,.05][frame%4];for(const r of racers){r.car.rollT=Math.max(0,r.car.rollT-dt);r.car.surge=Math.max(0,r.car.surge-dt);}
  const yaw=.2*Math.sin(frame*.03);car.yaw=yaw;car.pitch=.1*Math.cos(frame*.02);car.roll=.2*Math.sin(frame*.04);car.t+=dt;car.nitro=frame%50<20?.8:0;car.kick=frame%37<8?.5:0;
  car.model.position.y=Math.sin(car.t*2.3)*.12*(1-Math.min(1,Math.abs(car.speed)/95)*.7);car.model.rotation.x=car.nitro*.03+car.kick*car.kick*.07;Car.prototype.sync.call(car);const modelWorld=car.model.matrixWorld.toArray();flashes=[];
  worldCalls=0;const worldHit=frame%53===0;weapons.colliders.segment=()=>{worldCalls++;return worldHit?new THREE.Vector3(3,80,-20):null;};
  const inject=frame%19===0;if(inject)weapons.fire(racers[2],racers[2].car.pos,new THREE.Vector3(0,0,1),0,player,{});
  const smoke=frame%41===0;if(smoke)weapons.smoke(racers[2].car.pos);
  inp={fire:frame%100<70,steer:frame%3===0?0:frame%2?-.5:.5};hit={Space:frame%31===0,KeyF:frame%83===0,KeyX:frame%97===0};
  const raceActive=frame<140;g.state=raceActive?'race':'finished';const active=player.alive&&raceActive;if(active){ctx.updateLock(dt);ctx.playerWeapons(dt,inp);}
  weapons.update(dt,new THREE.Vector3(70,130,80));
  frames.push({dt,model,accent,modelWorld,yaw,flashes,restart,inject,smoke,worldHit,raceActive,active,input:[inp.fire,hit.Space,hit.KeyF,hit.KeyX,inp.steer],rng:rngState,draws,worldCalls,arsenal:state(g),lock:racers.indexOf(g.lock),charge:g.lockT,actors:racers.map(actorOut),shots:weapons.shots.map(s=>shotOut(s,racers)),fx:weapons.fx.map(f=>[f.t,f.life])});
 }traces.push({seed:traceSeed,model,accent,frames});
}
for(const s of sources){const buf=await readFile(path.join(browser,s.path));if(sha(buf)!==s.sha256)throw Error('source changed');const dest=path.join(path.dirname(out),'source-snapshot',s.path);await import('node:fs/promises').then(fs=>fs.mkdir(path.dirname(dest),{recursive:true}));await writeFile(dest,buf);}
const summary={frames:720,maxShots:Math.max(...traces.flatMap(t=>t.frames.map(f=>f.shots.length))),maxFx:Math.max(...traces.flatMap(t=>t.frames.map(f=>f.fx.length))),draws:traces.map(t=>t.frames.at(-1).draws)};
await writeFile(out,JSON.stringify({sources,version:JSON.parse(await readFile(path.join(browser,'package.json'),'utf8')).version,scope:'Actual current playerWeapons/updateLock/Car.muzzle/Weapons/projectiles/FX and nonreward damage/wreck, with supplied preweapon model matrices and owner paint. Warm Sprite geometry/cached texture stand-in, actual Sound ctx=null; no movement/AI/pack/rewards/network/render/audio-device integration.',summary,traces},null,2)+'\n');console.log(summary);
}finally{Math.random=nativeRandom;}
