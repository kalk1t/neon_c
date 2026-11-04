import fs from 'node:fs';import path from 'node:path';import {pathToFileURL} from 'node:url';import crypto from 'node:crypto';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),game=path.resolve(root,'../game');fs.mkdirSync(out,{recursive:true});
const load=p=>import(pathToFileURL(path.join(game,p)).href),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const [{Car},THREE]=await Promise.all([load('src/car.js'),load('node_modules/three/build/three.module.js')]);
const names=['package.json','src/car.js','src/aim.js','src/input.js','src/hud.js','src/main.js','index.html','docs/changelog.md','docs/manual/02-controls.md','docs/manual/04-flying.md'];
const sources=names.map(p=>({path:path.join(game,p),sha256:sha(fs.readFileSync(path.join(game,p)))}));
for(const p of ['04-components.md','08-testing.md','11-constants.md','12-coverage-open-questions.md'])sources.push({path:path.join(game,'../techplan',p),sha256:sha(fs.readFileSync(path.join(game,'../techplan',p)))});
function car(speed,yaw){const c=Object.create(Car.prototype);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),t:0,boost:1,boosting:false,maxMul:1,nitro:0,thrust:0,kick:0,sputter:0,model:{position:{},rotation:{}},pods:Array.from({length:4},()=>({rotation:{x:0,z:0}})),podTilt:.45,fans:[],ribbons:[],sync(){}});c.place(new THREE.Vector3(0,400,0),yaw,speed);return c;}
const rows=[];const specs=[
 {speed:3,yaw:0,inp:i=>({steer:i>80?1:0})},
 {speed:-.07,yaw:1,inp:i=>({})},
 {speed:0,yaw:.7,inp:i=>({strafe:i<150?1:0})},
 {speed:80,yaw:-.5,inp:i=>({throttle:1,strafe:-1})},
 {speed:-15,yaw:0,inp:i=>({brake:1,steer:-.7,aimSteer:true,strafe:1})},
 {speed:-15,yaw:0,inp:i=>({brake:1,steer:-.7})},
 {speed:55,yaw:1.3,inp:i=>({throttle:.8,brake:i%90<15?.4:0,steer:Math.sin(i*.03),climb:Math.cos(i*.02),strafe:Math.sin(i*.07),boost:i%70<30})},
 {speed:0,yaw:0,inp:i=>({strafe:1})}
];
for(let scenario=0;scenario<specs.length;scenario++){
 const s=specs[scenario],c=car(s.speed,s.yaw);for(let i=0;i<300;i++){
  const reset=i===210&&scenario===2,stun=scenario===7&&i===90;if(reset)c.place(new THREE.Vector3(0,400,0),s.yaw,0);if(stun)c.stun=.7;
  const inp={throttle:0,brake:0,steer:0,climb:0,boost:false,strafe:0,aimSteer:false,...s.inp(i)},dt=[1/60,1/144,.05][scenario%3];
  c.update(dt,inp,{collide:()=>null});
  const vals=[...c.pos.toArray(),...c.vel.toArray(),c.yaw,c.pitch,c.roll,c.speed,c.side,...c.pods.flatMap(p=>[p.rotation.x,p.rotation.z]),Math.hypot(c.speed,c.side)];
  rows.push('{'+[scenario,i===0?1:0,+reset,+stun,s.speed,s.yaw,dt].join(',')+',{'+[inp.throttle,inp.brake,inp.steer,inp.climb,!!inp.boost,inp.strafe,!!inp.aimSteer].join(',')+'},{'+vals.join(',')+'}}');
 }
}
// Exercise actual Input.state, both capture states and every keyboard mask, gamepad overrides.
globalThis.addEventListener=()=>{};globalThis.document={addEventListener(){},pointerLockElement:null};let pads=[];Object.defineProperty(globalThis,'navigator',{value:{getGamepads:()=>pads},configurable:true});
const {Input}=await load('src/input.js'),input=new Input(),codes=['KeyW','KeyS','KeyD','ArrowRight','KeyA','ArrowLeft','ArrowUp','ArrowDown','KeyE','KeyQ','ShiftLeft','ShiftRight'];let inputs=[];
for(const captured of [false,true])for(let keys=0;keys<4096;keys++)for(const pad of [false,true]){
 document.pointerLockElement=captured?{}:null;input.keys=new Set(codes.filter((k,i)=>keys&(1<<i)));
 const x=[0,.149999,.15,-.15,.72,-1][keys%6],y=[0,-.15,.5][keys%3],buttons=Array.from({length:10},()=>({value:0,pressed:false}));buttons[7].value=.4;buttons[6].value=.2;buttons[0].pressed=true;
 pads=pad?[null,{axes:[x,y],buttons}]:[];const s=input.state();inputs.push('{'+[keys,captured].join(',')+',{'+[pad,pad,x,y,.4,.2].join(',')+'},{'+[s.throttle,s.brake,s.steer,s.climb,s.boost,s.strafe,false].join(',')+'}}');
}
fs.writeFileSync(out+'/slide_fixture.h','/* Actual browser Car.update / Input.state, no renderer or real devices. */\nstatic const SlideCase slide_cases[]={\n'+rows.join(',\n')+'\n};\nstatic const SlideInputCase slide_inputs[]={\n'+inputs.join(',\n')+'\n};\n');
const glb=fs.readFileSync(path.join(root,'assets/models/roadster.glb')),gltf=JSON.parse(glb.subarray(20,20+glb.readUInt32LE(12)));let podRows=[];
for(const [index,node] of gltf.nodes.filter(n=>n.name.startsWith('hi_pod_')).entries())for(let i=0;i<100;i++){
 const pivot=new THREE.Vector3(...node.translation),offset=new THREE.Vector3(Math.sin(i)*.8,Math.cos(i)*.4,Math.sin(i*.3)*.9),baked=pivot.clone().add(offset),pitch=Math.sin(i*.1)*.3,roll=Math.cos(i*.3)*.2;
 const expected=offset.clone().applyEuler(new THREE.Euler(pitch,0,roll,'XYZ')).add(pivot);
 podRows.push('{'+[index,pitch,roll].join(',')+',{'+baked.toArray().join(',')+'},{'+expected.toArray().join(',')+'}}');
}
fs.writeFileSync(out+'/pod_fixture.h','/* Actual Three Euler XYZ about existing GLB pivots; vertices already baked by raylib. */\nstatic const PodCase pod_cases[]={\n'+podRows.join(',\n')+'\n};\n');
const receipt={version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,steps:rows.length,input_states:inputs.length,scenarios:specs.length,scope:'Actual source Car.update free-space callback (no world collision), Roadster podTilt .45; actual Input.state inert events/gamepad data. No game window, GPU, OS input, storage/network.'};fs.writeFileSync(out+'/slide-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
