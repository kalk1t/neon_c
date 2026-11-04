import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';import {pathToFileURL} from 'node:url';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),game=path.resolve(root,'../game'),sha=b=>crypto.createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(game,p)).href);fs.mkdirSync(out,{recursive:true});
const names=['package.json','src/car.js','src/physics.js','src/camera.js','src/roads.js','src/race.js','docs/changelog.md','docs/manual/04-flying.md'];const sources=names.map(p=>({path:p,sha256:sha(fs.readFileSync(path.join(game,p)))}));
const [{Car,CAR},THREE,{OBB,Cyl,Torus,Colliders}]=await Promise.all([load('src/car.js'),load('node_modules/three/build/three.module.js'),load('src/physics.js')]);
function cut(s,needle){const a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;while(n){if(s[i]==='{')n++;if(s[i]==='}')n--;i++;}return s.slice(a,i).replace('export function','function');}
const chase=cut(fs.readFileSync(path.join(game,'src/camera.js'),'utf8'),'export function chaseCamera(dt)');
function car(speed,yaw,mul){const c=Object.create(Car.prototype);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),t:0,boost:1,boosting:false,maxMul:mul,nitro:0,thrust:0,kick:0,sputter:0,model:{position:{},rotation:{}},pods:Array.from({length:4},()=>({rotation:{x:0,z:0}})),podTilt:.45,fans:[],ribbons:[],sync(){}});c.place(new THREE.Vector3(0,400,0),yaw,speed);return c;}
const traces=[];
for(let scenario=0;scenario<10;scenario++){
 const speed=[0,135,0,-15,196,35,60,60,60,0][scenario],yaw=scenario===4?.7:0,mul=[1,1,1,1,1,120/135,1,1,1,120*1.02**8/135*1.07][scenario],c=car(speed,yaw,mul),world=new Colliders();
 if(scenario===6)world.add(new OBB(0,400,-20,20,20,2));if(scenario===7)world.add(new Cyl(0,-20,6,380,420));if(scenario===8)world.add(new Torus(new THREE.Vector3(0,400,-20),0,15.5,2.45));
 const frames=[];let hit90=null;
 for(let i=0;i<360;i++){
  const dt=[1/60,1/144,.05][scenario%3],reset=i===210&&scenario===2,stun=i===90&&scenario===2,dodge=i===100&&(scenario===3||scenario===4),side=scenario===3?-1:1;
  if(reset)c.place(new THREE.Vector3(0,400,0),yaw,0);if(stun)c.stun=.7;if(dodge)c.dodge(side);
  const inp={throttle:scenario===3?0:scenario>=6&&scenario<=8?.8:1,brake:scenario===3?1:scenario===4&&i%80<15?.3:0,steer:scenario===3?-.7:scenario===2?Math.sin(i*.03):0,climb:scenario===2?Math.cos(i*.02)*.5:0,boost:scenario===1||scenario===4?i<210:false,strafe:scenario===2?i<150?1:0:0,aimSteer:scenario===3};
  const report=c.update(dt,inp,world);if(hit90===null&&c.speed>=135*.9)hit90=(i+1)*dt;
  const values=[...c.pos.toArray(),...c.vel.toArray(),...c.fwd.toArray(),c.yaw,c.pitch,c.roll,c.speed,c.boost,c.side,c.t,c.stun,c.rollT,c.surge,c.thrust,c.nitro,c.kick,c.sputter,...c.pods.flatMap(p=>[p.rotation.x,p.rotation.z]),Math.hypot(c.speed,c.side),report.impact];
  let camera=null;if(i%20===0){const oldNitro=c.nitro;c.nitro=0;const eye=new THREE.PerspectiveCamera(),ctx={THREE,CAR,car:c,cam:{yaw:.2,pitch:-.1,lookYaw:.1,lookPitch:.2,looking:true,fov:68},g:{camMode:i%40?1:0,interior:false,shake:0,kick:0,camAim:scenario%2===0,state:'race',autopilot:false,aim:{yaw:.2,pitch:-.1}},player:{alive:true},CAMS:[{d:9,h:2.7},{d:15,h:4.8}],camera:eye,dash:{show(){}},lens:{uniforms:Object.fromEntries(['uSpeed','uCA','uLines','uAspect'].map(k=>[k,{value:0}]))},performance:{now:()=>0},Math};vm.createContext(ctx);vm.runInContext(chase+';chaseCamera(0);',ctx);camera=[...eye.position.toArray(),...eye.getWorldDirection(new THREE.Vector3()).toArray(),...new THREE.Vector3(0,1,0).applyQuaternion(eye.quaternion).toArray()];c.nitro=oldNitro;}
  frames.push({dt,reset,stun,dodge,side,input:[inp.throttle,inp.brake,inp.steer,inp.climb,inp.boost,inp.strafe,inp.aimSteer],values,boosting:c.boosting,collision:!!report.n,normal:report.n?.toArray()??[0,0,0],camera,mode:i%40?1:0,aim:scenario%2===0});
 }traces.push({scenario,speed,yaw,mul,hit90,frames});
}
for(const s of sources){const buf=fs.readFileSync(path.join(game,s.path));if(sha(buf)!==s.sha256)throw Error('source changed '+s.path);const dest=path.join(out,'source-snapshot',s.path);fs.mkdirSync(path.dirname(dest),{recursive:true});fs.writeFileSync(dest,buf);}
const summary={frames:3600,scenarios:10,cameras:traces.flatMap(t=>t.frames).filter(f=>f.camera).length,collisions:traces.flatMap(t=>t.frames).filter(f=>f.collision).length,first90:traces[0].hit90};fs.writeFileSync(path.join(out,'speed-current-reference.json'),JSON.stringify({version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,CAR,summary,traces})+'\n');console.log({CAR,summary});
