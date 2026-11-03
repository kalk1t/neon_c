import {readFile,writeFile} from 'node:fs/promises';import {createHash} from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';import vm from 'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game'),sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const names=['package.json','src/main.js','src/fx.js','src/audio.js','src/ai.js','src/car.js','src/coins.js','src/track.js','src/roads.js','src/maps.js','src/physics.js','node_modules/three/build/three.core.js'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const [THREE,FX,{Car,SPIN},{Racer,LIVERIES,aiInput,WEAPON,DAMAGE,AI_FIRE_INTERVAL,COIN},{Coins},{Track,GRID},{ROADS}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/fx.js'),load('src/car.js'),load('src/ai.js'),load('src/coins.js'),load('src/track.js'),load('src/roads.js')]);
const {Colliders,OBB,Cyl,Torus}=await load('src/physics.js');
const main=await readFile(path.join(browser,'src/main.js'),'utf8'),fxSrc=await readFile(path.join(browser,'src/fx.js'),'utf8'),audioSrc=await readFile(path.join(browser,'src/audio.js'),'utf8'),coinSrc=await readFile(path.join(browser,'src/coins.js'),'utf8');
const cut=(a,b)=>{const i=main.indexOf(a),j=main.indexOf(b,i+a.length);if(i<0||j<0)throw Error('source hook changed '+a);return main.slice(i,j);};
let rngState=1,draws=0;const random=()=>{rngState=(Math.imul(rngState,1664525)+1013904223)>>>0;draws++;return rngState/4294967296;};const math=Object.create(Math);math.random=random;
const fxCtx={THREE,axisBillboard:FX.axisBillboard,additive:FX.additive,tex:()=>null,Math:math,_rel:new THREE.Vector3(),_dir:new THREE.Vector3(),_seg:new THREE.Vector3()};vm.createContext(fxCtx);vm.runInContext(fxSrc.slice(fxSrc.indexOf('export class Weapons')).replace('export class Weapons','class Weapons')+';this.Weapons=Weapons;',fxCtx);
const audioCtx={Math:math};vm.createContext(audioCtx);vm.runInContext(audioSrc.replace('export class Sound','class Sound')+';this.Sound=Sound;',audioCtx);new THREE.Sprite(new THREE.SpriteMaterial());
const coinCtx={THREE,COIN};vm.createContext(coinCtx);const ca=coinSrc.indexOf('    this.track = track;'),cb=coinSrc.indexOf("    // the shop's gold wheel coin",ca);vm.runInContext('this.make=function(scene,track){'+coinSrc.slice(ca,cb)+'}',coinCtx);
function coinField(track){const c=Object.create(Coins.prototype);coinCtx.make.call(c,new THREE.Group(),track);c.mesh={setMatrixAt(){},instanceMatrix:{}};c.halos=c.list.map(()=>({scale:{setScalar(){}}}));return c;}
function car(){const c=Object.create(Car.prototype);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),root:new THREE.Object3D(),t:0,rollDir:1,boost:1,boosting:false,maxMul:1,nitro:0,thrust:0,kick:0,sputter:0,model:{position:{},rotation:{}},pods:[],fans:[],ribbons:[],setVisible(v){this.visible=v;}});c.place(new THREE.Vector3(0,118,0),0,0);return c;}
const rs=r=>({pos:r.car.pos.toArray(),vel:r.car.vel.toArray(),fwd:r.car.fwd.toArray(),yaw:r.car.yaw,pitch:r.car.pitch,roll:r.car.roll,speed:r.car.speed,boost:r.car.boost,max:r.car.maxMul,boosting:r.car.boosting,stun:r.car.stun,t:r.car.t,roll_time:r.car.rollT,surge:r.car.surge,roll_direction:r.car.rollDir,sample:r.sample??0,dodge:r.dodge,lap:r.lap??1,next:r.nextCP??0,lap_time:r.lapTime??0,lap_count:r.lapTimes?.length??0,finished:!!r.finished,finish_time:r.finishTime??0,hp:r.hp??100,wreck:r.wreck??0,lost:r.lostTimer??0,stuck:r.stuck??0,coins:r.coins??0,visible:!!r.car.visible,passed:[...(r.passed??[])].reduce((n,k)=>n+2**k,0),fire_cd:r.fireCD,react:r.react,smoke:r.smokeT,wrong_way:r.wrongWay??0,previous:r.prevPos.toArray(),safe_pos:r.lastSafe.pos.toArray(),safe_yaw:r.lastSafe.yaw,lap_times:r.lapTimes});
const shotOut=(s,roster)=>[roster.indexOf(s.owner),roster.indexOf(s.target),s.pos.toArray(),s.vel.toArray(),s.speed,s.accel,s.maxSpeed,s.life,s.turn,s.damage??0,s.radius,s.trailT,['shot','bolt','missile','emp'].indexOf(s.kind),s.always,s.trail,s.damage!==undefined];
const traces=[],rivalProbes=[],nativeRandom=Math.random;
try{Math.random=random;
for(const road of ROADS.filter(r=>[1,9].includes(r.id))){
 const track=new Track(road);let ordinal=0;Math.random=()=>((++ordinal*17+11)%100)/100;
 const player=new Racer(car(),'YOU',0xffffff,true),rivals=LIVERIES.map(L=>new Racer(car(),L.name,L.accent,false,L.skill));Math.random=random;
 const colliders=new Colliders();const shapes=road.id===9?[[0,8,118,55,12,10,1.2,.15],[1,-8,45,4,100,135],[2,0,126,10,0,12,1.4]]:[];for(const v of shapes){if(v[0]===0)colliders.add(new OBB(...v.slice(1)));if(v[0]===1)colliders.add(new Cyl(...v.slice(1)));if(v[0]===2)colliders.add(new Torus(new THREE.Vector3(...v.slice(1,4)),...v.slice(4)));}
 const drivers=rivals.map(r=>[r.seed,r.lane,r.laneY,r.dodge,r.skill]),coins=coinField(track),weapons=new fxCtx.Weapons(new THREE.Scene(),colliders),sound=new audioCtx.Sound();weapons.floor=24;
 const g={state:'race',mode:'race',racers:[player,...rivals],remotes:[],laps:2,raceTime:0,shake:.1,lastNitro:-9,aim:{},side:1,lock:null,lockT:0,heat:0,overheated:false,boltCD:0,missiles:3,spinCD:0,empCD:0,aimDir:new THREE.Vector3(0,0,-1),rankOf(r){return this.ranking.indexOf(r)+1;}};
 const hud=new Proxy({msgTimer:0},{get:(o,k)=>k in o?o[k]:(()=>{})});let hits={},coinEvents=[];const ctx={THREE,Math:math,SPIN,WEAPON,DAMAGE,AI_FIRE_INTERVAL,GRID,DECK_Y:111,PLAYER_SLOT:7,mode:'race',track,road,player,rivals,car:player.car,coins,g,aiInput,IDLE:{throttle:0,brake:0,steer:0,climb:0,boost:false},colliders,GATE:{pos:new THREE.Vector3(0,126,0),halfW:23,y0:109,y1:142},map:{floor:34},_d:new THREE.Vector3(),_tmp:new THREE.Vector3(),_side:new THREE.Vector3(),sound,hud,weapons,input:{hit(...ks){return ks.some(k=>hits[k]);},unlock(){}},touch:null,live:null,roamSpawn(){throw Error('roam');},setGateText(){},fmt(){return'';},ordinal(){return'';},localStorage:{setItem(){}},setTimeout(){},volAt(){return 0;},panAt(){return 0;},dodged(){}};
 Object.assign(ctx,{account:{isKiller:false},bestKey(){return 'fixture-only';},showResults(){},settings:{sens:1,invert:false},MOUSE_RAD_PER_PX:.0021,cam:{lookYaw:0,lookPitch:0},});ctx.input.takeMouse=()=>({dx:0,dy:0});
 let reset=cut('function startRace(mode)','function toMenu()');reset=reset.slice(reset.indexOf('  coins.reset();'),reset.indexOf('  centreAim();')).replace('player.reset(gridPos(PLAYER_SLOT), 0, track);','player.reset(gridPos(PLAYER_SLOT), 0, track);this.resetPlayer=serialize(player);');
 const raw=cut('const KNOCK =','function completeLap'),wc=raw.indexOf('  if (attacker === player && !r.isPlayer)');const damage=raw.slice(0,wc)+raw.slice(raw.indexOf('  if (r.isPlayer) {',wc));
 const functions=cut('function respawn(r, keepHp = false)','// how hard each hit type')+damage+cut('function completeLap(r)','const _d =')+cut('function raceRules(r, dt)','function showResults()')+cut('function stepRacer(r, dt, ctl)','function frame()')+cut('function targetOf(r)','function setGateText')+cut('function centreAim()','/** Angle between')+cut('function aimAngle(','const _side =')+cut('function playerWeapons(','// Debug/demo pilot')+cut('function autopilot()','// ---------------- camera');
 const frameA=main.indexOf('  const playing = g.state',main.indexOf('function frame()')),frameB=main.indexOf('  // debug/screenshot hook:',frameA),cpu=main.slice(frameA,frameB);
 vm.createContext(ctx);ctx.serialize=rs;vm.runInContext('let time=17.25,rankT=.13;const gridPos=slot=>new THREE.Vector3(GRID[slot].x,DECK_Y+4.5,GRID[slot].z);'+functions+';this.resetPack=function(){'+reset+'updateRanking();g.raceTime=0;g.lastNitro=-9;g.countdown=3.999;g.lastCount=4;g.state="countdown";centreAim();};this.step=function(raw,inp){const dt=Math.min(raw,.05);time+=dt;'+cpu+'if(g.state==="paused")car.idle(dt);if(g.state!=="paused"){weapons.update(dt,new THREE.Vector3(70,130,80));coins.update(dt,time,g.racers,playing&&g.mode==="race");}};this.meta=()=>({ranking:g.ranking.map(r=>g.racers.indexOf(r)),position:g.position,finish_position:g.finishPos??0,rank_timer:rankT,shake:g.shake,time,last_nitro:g.lastNitro,countdown:g.countdown,last_count:g.lastCount,phase:g.state});',ctx);
 let currentControl=null,currentMotion=null,segmentCalls=0,segmentHits=0,currentPlayerEvents=0,insidePlayerStep=false;
 const originalUpdate=player.car.update;player.car.update=function(dt,ctl,world){currentControl={...ctl};const res=originalUpdate.call(this,dt,ctl,world);currentMotion=res;return res;};
 const originalStep=ctx.stepRacer;ctx.stepRacer=(r,dt,ctl)=>{if(r===player){currentControl={...ctl};insidePlayerStep=true;}try{return originalStep(r,dt,ctl);}finally{insidePlayerStep=false;}};
 const originalRespawn=ctx.respawn;ctx.respawn=(r,keep)=>{if(r===player)currentPlayerEvents|=16|(insidePlayerStep?0:32);return originalRespawn(r,keep);};
 const originalSmoke=weapons.smoke;weapons.smoke=function(...args){if(insidePlayerStep)currentPlayerEvents|=64;return originalSmoke.apply(this,args);};
 const originalLap=ctx.completeLap;ctx.completeLap=r=>{const value=originalLap(r);if(r===player)currentPlayerEvents|=2|(r.finished?4:0);return value;};
 const originalCheckpoint=sound.checkpoint.bind(sound);sound.checkpoint=()=>{currentPlayerEvents|=1;return originalCheckpoint();};
 const originalSegment=colliders.segment.bind(colliders);colliders.segment=(a,b,radius)=>{segmentCalls++;const hit=originalSegment(a,b,radius);if(hit)segmentHits++;return hit;};
 weapons.onHit=(s,r)=>ctx.damage(r,s.damage??(r.isPlayer?DAMAGE.RIVAL_SHOT_ON_PLAYER:DAMAGE.RIVAL_SHOT),s.owner,s.kind);
 coins.onCollect=(r,c)=>{coinEvents.push([coins.list.indexOf(c),g.racers.indexOf(r)]);weapons.impact(c.pos,false,r.isPlayer?12:8);sound.coin();};
 const resetResources=main.match(/Object\.assign\(g, \{ heat: 0, overheated: false, boltCD: 0,[^\n]+\}\);/)[0];
 function resetAll(){vm.runInContext(resetResources,ctx);ctx.resetPack();return {player:ctx.resetPlayer,after:g.racers.map(rs),rng:rngState,draws,meta:ctx.meta()};}
 rngState=(12345+road.id*98765)>>>0;draws=0;const seed=rngState,start=resetAll(),frames=[];
 let pausedFrom='race';
 for(let frame=0;frame<320;frame++){
  let resetState=null;if(frame===260)resetState=resetAll();
  let mask=0;
  if(frame===120){player.car.place(new THREE.Vector3(8,115.5,64),0,95);mask|=1;}
  if(frame===130){player.hp=40;player.wreck=0;player.car.setVisible(true);mask|=2;}
  if(frame===140){player.hp=0;player.wreck=.1;player.car.setVisible(false);mask|=4;}
  if(frame===150){pausedFrom=g.state;g.state='paused';}if(frame===165)g.state=pausedFrom;
  if(frame===170){player.car.pos.y=25;player.lostTimer=1.09;mask|=8;}
  if(frame===185){const cp=track.checkpoints[0];player.car.place(cp.pos.clone().addScaledVector(cp.forward,-.5),cp.yaw,45);player.nextCP=0;player.passed.clear();mask|=16;}
  if(frame===200||frame===220){player.car.place(new THREE.Vector3(0,126,.2),0,35);player.nextCP=track.checkpoints.length;player.passed=new Set(track.checkpoints.map(c=>c.index));mask|=32;}
  const raw=[1/60,.05,.2,1/30][frame%4];hits={Space:frame%37===0,KeyF:frame%89===0,KeyX:frame%101===0,KeyR:frame===175};
  const inp={throttle:1,brake:frame%40>30?.2:0,steer:Math.sin(frame*.07)*.35,climb:Math.cos(frame*.03)*.2,boost:frame%60<15,fire:frame%80<55,keySteer:true,keyClimb:true,look:false};
  currentControl=null;currentMotion=null;segmentCalls=segmentHits=currentPlayerEvents=0;coinEvents=[];const entered=g.state,previousNitro=g.lastNitro;ctx.step(raw,inp);const c=currentControl??inp;
  if(entered==='countdown'&&g.state==='race')currentPlayerEvents|=8;
  if(g.lastNitro!==previousNitro)currentPlayerEvents|=256;
  if((entered==='race'||entered==='finished')&&(currentMotion?.impact??0)>4)currentPlayerEvents|=128;
  frames.push({playerEvents:currentPlayerEvents,reset:resetState,mask,raw,entered,control:[c.throttle,c.brake,c.steer,c.climb,c.boost],weapons:[c.fire??inp.fire,hits.Space,hits.KeyF,hits.KeyX,c.steer],manual:hits.KeyR,aim:g.aimDir.toArray(),after:g.racers.map(rs),rng:rngState,draws,race_time:g.raceTime,meta:ctx.meta(),arsenal:[g.heat,g.boltCD,g.missiles,g.spinCD,g.empCD,g.overheated,g.side],lock:g.racers.indexOf(g.lock),charge:g.lockT,shots:weapons.shots.map(s=>shotOut(s,g.racers)),fx:weapons.fx.map(f=>[f.t,f.life]),coins:coins.list.map(c=>[c.active,c.timer,c.pop]),coinEvents,motion:{impact:currentMotion?.impact??0,normal:currentMotion?.n?.toArray()??[0,0,0],collided:!!currentMotion?.n},segmentCalls,segmentHits});
 }
 traces.push({road:road.id,seed,drivers,shapes,start,frames});
}
for(const s of sources)if(sha(await readFile(path.join(browser,s.path)))!==s.sha256)throw Error('source changed');
await writeFile(out,JSON.stringify({sources,scope:'Actual full local frame CPU branch including player movement/rules/respawn/manual reset, ten-rival combat, source primitive collisions, lock/weapons/projectiles/FX/coins. Processed controls/aim replayed to C; CPU warm Three/cached textures, Sound ctx=null, no rewards/accounts/render/vehicle updateFX.',traces},null,1)+'\n');console.log({roads:traces.map(t=>t.road),frames:640,maxShots:Math.max(...traces.flatMap(t=>t.frames.map(f=>f.shots.length))),maxFx:Math.max(...traces.flatMap(t=>t.frames.map(f=>f.fx.length))),draws:traces.map(t=>t.frames.at(-1).draws),phases:traces.map(t=>[...new Set(t.frames.map(f=>f.meta.phase))]),segmentHits:traces.map(t=>t.frames.reduce((n,f)=>n+f.segmentHits,0)),playerImpacts:traces.map(t=>t.frames.filter(f=>f.motion.impact>4).length)});
}finally{Math.random=nativeRandom;}

