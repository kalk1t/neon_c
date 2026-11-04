import {readFile,writeFile,mkdir} from 'node:fs/promises';import {createHash} from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';import vm from 'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game');
const sha=b=>createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const files=['package.json','src/race.js','src/ai.js','src/car.js','src/fx.js','src/audio.js','node_modules/three/build/three.core.js'];const sources=await Promise.all(files.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const [THREE,FX,{Car,CAR_MODELS},{WEAPON,Racer}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/fx.js'),load('src/car.js'),load('src/ai.js')]);
const race=await readFile(path.join(browser,'src/race.js'),'utf8'),fx=await readFile(path.join(browser,'src/fx.js'),'utf8'),audio=await readFile(path.join(browser,'src/audio.js'),'utf8');
function cut(src,needle){const a=src.indexOf(needle);if(a<0)throw Error(needle);let i=src.indexOf('{',a)+1,n=1;while(n){if(src[i]==='{')n++;if(src[i]==='}')n--;i++;}return src.slice(a,i).replace('export function','function');}
let state=1,draws=0;const random=()=>{state=(Math.imul(state,1664525)+1013904223)>>>0;draws++;return state/4294967296;},math=Object.create(Math);math.random=random;
const fc={THREE,Math:math,axisBillboard:FX.axisBillboard,additive:FX.additive,tex:()=>null,_rel:new THREE.Vector3(),_dir:new THREE.Vector3(),_seg:new THREE.Vector3()};vm.createContext(fc);vm.runInContext(fx.slice(fx.indexOf('export class Weapons')).replace('export class Weapons','class Weapons')+';this.Weapons=Weapons;',fc);
const ac={Math:math};vm.createContext(ac);vm.runInContext(audio.replace('export class Sound','class Sound')+';this.Sound=Sound;',ac);
function car(){const root=new THREE.Object3D(),model=new THREE.Object3D();root.add(model);return {root,model,pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),speed:0,muzzle:Car.prototype.muzzle,setVisible(){},place(p){this.pos.copy(p);}};}
const original=Math.random;const traces=[];try{Math.random=random;
new THREE.Sprite(new THREE.SpriteMaterial());
for(let model=0;model<4;model++){
 const r=new Racer(car(),'rival',0xffffff),a=new Racer(car(),'a',0xffffff,true),b=new Racer(car(),'b',0xffffff);const roster=[r,a,b];r.car.muzzleAt=model?CAR_MODELS[['roadster','viper','titan'][model-1]].muzzle:undefined;
 const accent=[0xff26d4,0xffa020,0x9a5cff,0x3a8cff][model];r.shotColor=new THREE.Color(accent).lerp(new THREE.Color(1,1,1),.35).multiplyScalar(3);r.shotMat=new THREE.MeshBasicMaterial({color:r.shotColor});
 const weapons=new fc.Weapons(new THREE.Scene(),{}),sound=new ac.Sound(),g={racers:roster,state:'race',raceTime:7},road={fire:1};let cues=[];const zap=sound.zap.bind(sound);sound.zap=(...args)=>{zap(...args);cues.push([state,draws,r.heat,r.boltCD,r.side]);};
 const ctx={THREE,Math:math,WEAPON,g,road,weapons,sound,_d:new THREE.Vector3(),_lead:new THREE.Vector3(),volAt(){return 0;},panAt(){return 0;}};vm.createContext(ctx);vm.runInContext(race.match(/export const RIVAL_AIM = [^;]+;/)[0].replace('export const','const')+'\n'+cut(race,'export function rivalCombat(r, dt)')+';this.step=rivalCombat;',ctx);
 const seed=12345+model*77777;state=seed;draws=0;r.side=0;const frames=[];
 for(let i=0;i<1000;i++){
  const reset=i%200===0;if(reset)Racer.prototype.reset.call(r,new THREE.Vector3(3,104,-8),0,{nearestGlobal(){return 0;}});
  const dt=[.01,1/60,1/30,.05,.2][i%5];road.fire=[1,.8,.6][Math.floor(i/100)%3];g.raceTime=i%137===0?5.999:7;g.state=i%101===0?'finished':'race';r.wreck=i%139===0?1:0;r.finished=i%149===0;r.car.speed=i%71===0?-35:135;r.fireCD=i%113===0?r.fireCD+3:r.fireCD;
  if(i%233===150){r.heat=1;r.overheated=true;}
  const distance=[14.999,15,1125,1125.001,200,500][Math.floor(i/10)%6],angle=[0,.3,.3000001,.1][Math.floor(i/30)%4];
  a.car.pos.set(3+Math.sin(angle)*distance,104,-8-Math.cos(angle)*distance);b.car.pos.copy(a.car.pos);b.car.pos.z-=i%7===0?0:10;a.car.vel.set(i%9-4,2,-35);b.car.vel.set(-20,0,15);a.wreck=i%67===0?1:0;a.finished=i%79===0;b.wreck=i%89===0?1:0;b.finished=i%97===0;
  r.car.root.position.copy(r.car.pos);r.car.root.rotation.set(.12*Math.sin(i*.1),.15*Math.cos(i*.1),.2*Math.sin(i*.03),'YXZ');r.car.model.position.y=Math.sin(i*.1)*.1;r.car.model.rotation.x=.03;r.car.root.updateMatrixWorld(true);
  const before=[r.fireCD,r.heat??0,r.boltCD??0,r.side??0,!!r.overheated];cues=[];weapons.clear();ctx.step(r,dt);
  frames.push({reset,dt,road:road.fire,time:g.raceTime,active:g.state==='race',wreck:r.wreck,finished:r.finished,speed:r.car.speed,matrix:r.car.model.matrixWorld.toArray(),targets:[a,b].map(o=>[o.car.pos.toArray(),o.car.vel.toArray(),o.wreck,o.finished]),before,after:[r.fireCD,r.heat,r.boltCD,r.side??0,!!r.overheated],rng:state,draws,cues,shots:weapons.shots.map(s=>[s.pos.toArray(),s.vel.toArray(),s.speed,s.life,s.turn,s.damage,s.len,s.wid,s.radius,s.target===null,s.kind==='bolt',s.m.material.color.toArray()])});
 }
 traces.push({model,accent,seed,frames});
}
await mkdir(path.dirname(out),{recursive:true});for(const s of sources){const buf=await readFile(path.join(browser,s.path));if(sha(buf)!==s.sha256)throw Error('source changed '+s.path);const dest=path.join(path.dirname(out),'source-snapshot',s.path);await mkdir(path.dirname(dest),{recursive:true});await writeFile(dest,buf);}
const summary={frames:traces.reduce((n,t)=>n+t.frames.length,0),shots:traces.reduce((n,t)=>n+t.frames.reduce((v,f)=>v+f.shots.length,0),0),draws:traces.map(t=>t.frames.at(-1).draws)};await writeFile(out,JSON.stringify({sources,version:JSON.parse(await readFile(path.join(browser,'package.json'),'utf8')).version,summary,traces})+'\n');console.log(summary);
}finally{Math.random=original;}
