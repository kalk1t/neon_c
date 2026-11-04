import fs from'node:fs';import path from'node:path';import crypto from'node:crypto';import{pathToFileURL}from'node:url';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),game=path.resolve(root,'../game'),sha=b=>crypto.createHash('sha256').update(b).digest('hex'),load=p=>import(pathToFileURL(path.join(game,p)).href);fs.mkdirSync(out,{recursive:true});
const names=['package.json','src/car.js','src/physics.js','public/models/car.glb','node_modules/three/build/three.core.js'];const sources=names.map(p=>({path:p,sha256:sha(fs.readFileSync(path.join(game,p)))}));const[{Car,CAR_MODELS},THREE]=await Promise.all([load('src/car.js'),load('node_modules/three/build/three.module.js')]);
const glb=fs.readFileSync(path.join(game,'public/models/car.glb')),gltf=JSON.parse(glb.subarray(20,20+glb.readUInt32LE(12)).toString('utf8'));
const nodes=gltf.nodes.filter(n=>n.name?.startsWith('hi_'));if(nodes.length!==5||nodes.some(n=>n.rotation||n.scale||n.matrix))throw Error('Unexpected baked-node transform');
const bodyPoints=[[0,0,0],[1.1,.7,-2.5],[-1.1,.7,2.7],[.8,1.5,1.9]],podOffsets=[[.2,.4,-.3],[-.3,-.2,.6]];
function car(){const c=Object.create(Car.prototype);c.root=new THREE.Object3D();c.model=new THREE.Object3D();c.root.add(c.model);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),t:0,rollDir:1,boost:1,boosting:false,maxMul:1,nitro:0,thrust:0,kick:0,sputter:0,podTilt:.45,pods:[],fans:[],ribbons:[]});const sculpt=new THREE.Group();sculpt.userData.spec=CAR_MODELS.roadster;
for(const n of nodes){const child=new THREE.Object3D();child.name=n.name;child.position.fromArray(n.translation??[0,0,0]);sculpt.add(child);}
c.flames=[new THREE.Object3D(),new THREE.Object3D(),new THREE.Object3D()];c.useModel(sculpt);c.renderBody=sculpt;return c;}
const state=c=>[c.pos.toArray(),c.vel.toArray(),c.fwd.toArray(),c.yaw,c.pitch,c.roll,c.speed,c.boost,c.t,c.stun,c.rollT,c.surge,c.maxMul,c.thrust,c.nitro,c.kick,c.sputter,c.rollDir,c.boosting];const traces=[];
for(let scenario=0;scenario<6;scenario++){
 const c=car(),speed=[0,80,135,196,-15,35][scenario],yaw=scenario*.4;c.place(new THREE.Vector3(3,200,5),yaw,speed);const frames=[];
 for(let i=0;i<240;i++){
  const dt=[1/60,1/144,.05][scenario%3],beforeSpeed=c.speed;let action=0;
  const input={throttle:i%70<50?1:0,brake:i%53<8?.3:0,steer:Math.sin(i*.03),climb:Math.cos(i*.05)*.5,boost:i%90<30,strafe:Math.sin(i*.07),aimSteer:true};
  if(i%53===0){action=1;c.place(new THREE.Vector3(3+i*.3,200+i*.2,5-i*.5),yaw+.1*i,35);}
  else if(i%41===0){action=2;c.pos.x+=4;c.vel.z-=3;c.stun=.7;c.speed*=.8;}
  else if(i%37===0){action=3;c.dodge(i%2?-1:1);}
  else if(i%31===0){action=4;c.idle(dt);}
  else if(i%23===0){action=5;c.yaw+=.3;c.pitch=-.2;c.roll=.15;c.stun=.4;c.sync();}
  else c.update(dt,input,{collide:()=>null});
  c.model.updateWorldMatrix(true,false);const model=(scenario+i)%4,side=i%3-1;c.muzzleAt=model?CAR_MODELS[['roadster','viper','titan'][model-1]].muzzle:undefined;const muzzle=c.muzzle(side).toArray();
  c.renderBody.updateWorldMatrix(true,true);
  const render={fitted:c.renderBody.matrixWorld.toArray(),body:bodyPoints.map(v=>new THREE.Vector3(...v).applyMatrix4(c.renderBody.matrixWorld).toArray()),fallback:bodyPoints.map(v=>new THREE.Vector3(...v).applyMatrix4(c.model.matrixWorld).toArray()),pods:c.pods.map(p=>({pitch:p.rotation.x,roll:p.rotation.z,points:podOffsets.map(v=>p.localToWorld(new THREE.Vector3(...v)).toArray())}))};
  frames.push({render,action,dt,beforeSpeed,input:[input.throttle,input.brake,input.steer,input.climb,input.boost,input.strafe,input.aimSteer],state:state(c),root:c.root.matrixWorld.toArray(),matrix:c.model.matrixWorld.toArray(),bob:c.model.position.y,lift:c.model.rotation.x,model,side,muzzle});
 }traces.push({scenario,speed,yaw,frames});
}
for(const s of sources){const bytes=fs.readFileSync(path.join(game,s.path));if(sha(bytes)!==s.sha256)throw Error('source changed');const dest=path.join(out,'source-snapshot',s.path);fs.mkdirSync(path.dirname(dest),{recursive:true});fs.writeFileSync(dest,bytes);}
const summary={frames:1440,actions:[0,1,2,3,4,5].map(k=>traces.reduce((n,t)=>n+t.frames.filter(f=>f.action===k).length,0))};fs.writeFileSync(path.join(out,'render-pose-reference.json'),JSON.stringify({version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,scope:'Actual Car.useModel/update/sync/place/idle/dodge and Three world points with original Roadster GLB pivots; hierarchy uses Object3D nodes, no mesh/texture decoder, GPU or real world. Code-built branch tests parent transform only.',summary,traces})+'\n');console.log(summary);
