import fs from 'node:fs';import vm from 'node:vm';import path from 'node:path';import crypto from 'node:crypto';import {pathToFileURL} from 'node:url';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),game=path.resolve(root,'../game'),sha=b=>crypto.createHash('sha256').update(b).digest('hex');fs.mkdirSync(out,{recursive:true});
const names=['package.json','src/camera.js','src/hud.js','src/aim.js','src/main.js','src/style.css','docs/manual/04-flying.md'];const originals=Object.fromEntries(names.map(p=>[p,fs.readFileSync(path.join(game,p))]));
const THREE=await import(pathToFileURL(path.join(game,'node_modules/three/build/three.module.js')));
function cut(s,needle){const a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}if(n)throw Error('unclosed');return s.slice(a,i);}
const fn=cut(originals['src/camera.js'].toString(),'function chaseCamera(dt)'),hudsrc=originals['src/hud.js'].toString();
const screen=cut(hudsrc,'  screen(p, camera)').trim(),place=cut(hudsrc,'  place(el, s)').trim();const a=hudsrc.indexOf('    // mouse aim ring ('),b=hudsrc.indexOf('    // lock-on bracket',a);if(a<0||b<a)throw Error('HUD branch');
const reticleCtx={innerWidth:1280,innerHeight:720};vm.createContext(reticleCtx);vm.runInContext('this.screen=function'+screen.slice(6)+';this.place=function'+place.slice(5)+';this.reticles=function(g,camera){const e=this.el,P=g.player,car=P.car;'+hudsrc.slice(a,b)+'};',reticleCtx);
const cameraRows=[],reticleRows=[];
function reticle(car,camera,captured,alive,aimDir,zoom){
 const e={cross:{style:{},classList:{toggle(k,v){this[k]=v;}}},aim:{style:{}},hitmark:{style:{}}},h={el:e,z:zoom,_v:new THREE.Vector3(),screen:reticleCtx.screen,place:reticleCtx.place};
 reticleCtx.reticles.call(h,{player:{car,alive},mouseLocked:captured,aimDir},camera);
 const cp=h.screen(car.pos.clone().addScaledVector(car.fwd,90),camera),ap=h.screen(car.pos.clone().addScaledVector(aimDir,90),camera);const up=new THREE.Vector3(0,1,0).applyQuaternion(camera.quaternion),fwd=camera.getWorldDirection(new THREE.Vector3());
 const V=v=>'{'+v.join(',')+'}';
 reticleRows.push('{'+[V(camera.position.toArray()),V(camera.position.clone().addScaledVector(fwd,20).toArray()),V(up.toArray()),V(car.pos.clone().addScaledVector(car.fwd,90).toArray()),V(car.pos.clone().addScaledVector(aimDir,90).toArray()),camera.fov,zoom,+captured,+alive,+(e.cross.style.display==='block'),+(e.aim.style.display==='block'),+!!e.cross.classList.nose,V(cp?[cp[0]*zoom,cp[1]*zoom]:[0,0]),V(ap?[ap[0]*zoom,ap[1]*zoom]:[0,0])].join(',')+'}');
}
for(let scenario=0;scenario<12;scenario++){
 const car={pos:new THREE.Vector3(3,100,4),fwd:new THREE.Vector3(0,0,-1),yaw:0,pitch:0,roll:0,speed:95,nitro:0,model:{visible:true}},camera=new THREE.PerspectiveCamera(68,1280/720,.3,22000);
 const cam={yaw:[7.1,-4,Math.PI][scenario%3],pitch:.3,lookYaw:scenario%2?.32:0,lookPitch:scenario%2?-.16:0,looking:false,fov:68};
 const g={camMode:scenario%3,interior:false,shake:0,kick:0,camAim:false,state:'countdown',autopilot:false,aim:{yaw:0,pitch:0}},ctx={THREE,car,CAR:{MAX:95},cam,g,CAMS:[{d:9,h:2.7},{d:15,h:4.8},{d:0,h:0}],player:{alive:scenario%4!==3},camera,dash:{show(){}},lens:{uniforms:Object.fromEntries(['uSpeed','uCA','uLines','uAspect'].map(k=>[k,{value:0}]))},performance:{now:()=>0},Math};vm.createContext(ctx);vm.runInContext(fn+';this.run=chaseCamera;',ctx);
 for(let frame=0;frame<180;frame++){
  const dt=[1/60,1/144,.05,1/30][frame%4],before=[cam.yaw,cam.pitch,cam.lookYaw,cam.lookPitch];
  const phase=frame<20?3:frame<140?1:frame<160?4:5;g.state=phase===1?'race':phase===3?'countdown':'finished';g.camAim=frame%80<60;g.autopilot=frame>=110&&frame<125;cam.looking=frame>=45&&frame<80;
  car.yaw=6.1+frame*.013+scenario*.21;car.pitch=.33*Math.sin(frame*.03);car.speed=[0,95,-15,138][scenario%4];car.roll=.4*Math.sin(frame*.04);car.pos.set(3+frame*.03,100+Math.sin(frame*.03)*8,4-frame*.2);car.fwd.set(-Math.sin(car.yaw)*Math.cos(car.pitch),Math.sin(car.pitch),-Math.cos(car.yaw)*Math.cos(car.pitch));g.aim.yaw=car.yaw+.9*Math.sin(frame*.026);g.aim.pitch=.6*Math.cos(frame*.022);
  ctx.run(dt);camera.updateMatrixWorld();const fwd=camera.getWorldDirection(new THREE.Vector3()),up=new THREE.Vector3(0,1,0).applyQuaternion(camera.quaternion),pose=[...camera.position.toArray(),...fwd.toArray(),...up.toArray()];
  cameraRows.push('{'+[+(frame===0),phase,+g.camAim,+g.autopilot,g.camMode,+ctx.player.alive,+cam.looking,dt,car.yaw,car.pitch,g.aim.yaw,g.aim.pitch,car.speed,car.roll].join(',')+',{'+car.pos.toArray().join(',')+'},{'+before.join(',')+'},{'+[cam.yaw,cam.pitch,cam.lookYaw,cam.lookPitch].join(',')+'},{'+pose.join(',')+'}}');
  if(frame%6===0){const d=new THREE.Vector3(-Math.sin(g.aim.yaw)*Math.cos(g.aim.pitch),Math.sin(g.aim.pitch),-Math.cos(g.aim.yaw)*Math.cos(g.aim.pitch));reticle(car,camera,g.camAim,ctx.player.alive,d,[.55,.8,1][scenario%3]);}
 }
}
// Off-viewport aim still makes the nose small; behind/far-clipped aim does not.
const camera=new THREE.PerspectiveCamera(68,1280/720,.3,22000);camera.lookAt(0,0,-1);camera.updateMatrixWorld();const c={pos:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1)};
for(const aim of [new THREE.Vector3(0,0,-1),new THREE.Vector3(20,0,-1),new THREE.Vector3(0,0,1)])for(const captured of [false,true])for(const alive of [false,true])reticle(c,camera,captured,alive,aim,1);
fs.writeFileSync(out+'/aim_camera_fixture.h','/* Actual current browser chaseCamera sequential motion and real Three poses. No GPU/effects. */\nstatic const AimCameraCase camera_cases[]={\n'+cameraRows.join(',\n')+'\n};\n');
fs.writeFileSync(out+'/nose_reticle_fixture.h','/* Actual current HUD.screen, reticle branch/classList and real Three matrices. */\nstatic const NoseCase nose_cases[]={\n'+reticleRows.join(',\n')+'\n};\n');
const css=originals['src/style.css'].toString(),noseRule=css.match(/\n#crosshair\.nose \{[^}]+\}/)?.[0],hideRule=css.match(/\n#crosshair\.nose::before[^}]+\}/)?.[0];if(!noseRule?.includes('width: 8px; height: 8px; margin: -4px;')||!noseRule.includes('rgba(25, 230, 255, 0.8)')||!hideRule?.includes('display: none'))throw Error('Nose CSS changed');
for(const [name,bytes] of Object.entries(originals)){if(sha(bytes)!==sha(fs.readFileSync(path.join(game,name))))throw Error('Source changed '+name);}
const oracle={version:JSON.parse(originals['package.json']).version,sources:Object.entries(originals).map(([name,bytes])=>({path:path.join(game,name),sha256:sha(bytes)})),camera_sequences:12,camera_steps:cameraRows.length,reticle_states:reticleRows.length,nose_css:[noseRule,hideRule],scope:'Actual source chaseCamera and HUD reticle/screen methods in inert VM with real Three math. Includes camera modes, capture request changes, countdown/active/finished/results, autopilot, live/dead and RMB hold/release. No runtime graphics/window/input/devices/accounts or raster/AA/glow tests. Effects/nitro/shake zeroed to isolate changed camera core.'};fs.writeFileSync(out+'/aim-camera-oracle.json',JSON.stringify(oracle,null,2));console.log(oracle);
