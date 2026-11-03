import fs from 'node:fs';import vm from 'node:vm';import path from 'node:path';import crypto from 'node:crypto';import {pathToFileURL} from 'node:url';
const [source,threePath,out]=process.argv.slice(2),bytes=fs.readFileSync(source),text=bytes.toString(),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
let a=text.indexOf('function chaseCamera(dt) {'),i=text.indexOf('{',a)+1,n=1;if(a<0)throw Error('Missing chaseCamera');for(;i<text.length&&n;i++){if(text[i]==='{')n++;if(text[i]==='}')n--;}const fn=text.slice(a,i);
const THREE=await import(pathToFileURL(path.resolve(threePath))),rows=[],endStates=[];
for(let scenario=0;scenario<6;scenario++){
 const car={pos:new THREE.Vector3(3,100,4),fwd:new THREE.Vector3(0,0,-1),yaw:0,pitch:0,roll:.4,speed:95,nitro:0,model:{visible:true}},camera=new THREE.PerspectiveCamera();
 const cam={yaw:[7.1,-4,Math.PI][scenario%3],pitch:.3,lookYaw:[7.4,-7.4,.4][scenario%3],lookPitch:scenario%2?-.2:.3,looking:false,fov:68};
 const ctx={THREE,car,CAR:{MAX:95},cam,g:{camMode:0,interior:false,shake:0,kick:0},CAMS:[{d:9,h:2.7},{d:15,h:4.8},{d:0,h:0}],player:{alive:true},camera,dash:{show(){}},lens:{uniforms:Object.fromEntries(['uSpeed','uCA','uLines','uAspect'].map(k=>[k,{value:0}]))},performance:{now:()=>0},Math};
 vm.createContext(ctx);vm.runInContext(fn,ctx);
 for(let frame=0;frame<160;frame++){
  const before=[cam.yaw,cam.pitch,cam.lookYaw,cam.lookPitch],dt=[0,1/60,1/30,.05][frame%4];
  car.yaw=Math.sin(frame*.05+scenario)*4.2;car.pitch=Math.cos(frame*.03)*.6;car.fwd.set(-Math.sin(car.yaw)*Math.cos(car.pitch),Math.sin(car.pitch),-Math.cos(car.yaw)*Math.cos(car.pitch));
  // First 120 ticks are active input; later ticks model finished/autopilot,
  // which does not call updateAim and therefore retains the last looking flag.
  if(frame<120)cam.looking=frame<30||(frame>=60&&frame<90)||(scenario%2===1&&frame>=90);
  ctx.dt=dt;vm.runInContext('chaseCamera(dt);',ctx);
  const expected=[cam.yaw,cam.pitch,cam.lookYaw,cam.lookPitch],dir=camera.getWorldDirection(new THREE.Vector3()),up=new THREE.Vector3(0,1,0).applyQuaternion(camera.quaternion);
  rows.push(`{${frame===0?1:0},${cam.looking},${dt},${car.yaw},${car.pitch},{${before.join(',')}},{${expected.join(',')}},{${[...camera.position.toArray(),...dir.toArray(),...up.toArray()].join(',')}}}`);
 }
 endStates.push({scenario,looking:cam.looking,lookYaw:cam.lookYaw,lookPitch:cam.lookPitch});
}
if(sha(fs.readFileSync(source))!==sha(bytes))throw Error('Source changed');
fs.mkdirSync(out,{recursive:true});fs.writeFileSync(path.join(out,'camera_motion_fixture.h'),'/* Actual source chaseCamera sequential timing, with resulting Three poses. */\nstatic const MotionCase motion_cases[]={\n'+rows.join(',\n')+'\n};\n');
const receipt={source,sourceSHA256:sha(bytes),cases:rows.length,sequences:6,ticksPerSequence:160,endStates,scope:'Actual v0.2.6 chaseCamera timing plus real Three camera poses. 120 active-input ticks then 40 finished ticks retaining the last processed looking flag. Delta times are already capped host values; no graphics, mouse devices, pause showcase, effects or accounts.'};fs.writeFileSync(path.join(out,'camera-motion-oracle.json'),JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify(receipt));
