import fs from 'node:fs';import crypto from 'node:crypto';import path from 'node:path';import vm from 'node:vm';import {pathToFileURL} from 'node:url';
const [mainPath,cameraPath,threePath,out]=process.argv.slice(2),mainBytes=fs.readFileSync(mainPath),cameraBytes=fs.readFileSync(cameraPath),main=mainBytes.toString(),camSrc=cameraBytes.toString(),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
function block(text,needle){const a=text.indexOf(needle);if(a<0)throw Error('Missing source hook '+needle);let i=text.indexOf('{',a)+1,n=1;for(;i<text.length&&n;i++){if(text[i]==='{')n++;if(text[i]==='}')n--;}return text.slice(a,i);}
const ignite=block(main,'if (car.boosting && !wasBoost'),cameraCode=block(camSrc,'function chaseCamera(dt)');
const restart=main.match(/g\.lastNitro = -9;/)?.[0];if(!restart)throw Error('Restart hook changed');
const THREE=await import(pathToFileURL(path.resolve(threePath))),rows=[];let events=0;
for(let scenario=0;scenario<3;scenario++){
 const initial={lastNitro:scenario===1?0:-9,kick:scenario===2?.7:0,shake:scenario===2?1.18:scenario===1?.001:0};
 const car={pos:new THREE.Vector3(3,100,4),fwd:new THREE.Vector3(0,0,-1),model:{visible:true},yaw:0,pitch:0,roll:0,speed:0,nitro:0,boosting:false};
 const cam={yaw:.4,pitch:-.1,lookYaw:.2,lookPitch:.1,looking:false,fov:70};
 const initialMotion=[cam.yaw,cam.pitch,cam.lookYaw,cam.lookPitch],g={camMode:scenario,interior:false,shake:initial.shake,kick:initial.kick,lastNitro:initial.lastNitro,raceTime:0};
 let timeMs=1000,event=false;
 const camera=new THREE.PerspectiveCamera(),ctx={THREE,car,cam,g,CAR:{MAX:95},CAMS:[{d:9,h:2.7},{d:15,h:4.8},{d:0,h:0}],player:{alive:true},camera,dash:{show(){}},lens:{uniforms:Object.fromEntries(['uSpeed','uCA','uLines','uAspect'].map(k=>[k,{value:0}]))},performance:{now:()=>timeMs},sound:{boostOn(){event=true;},boostOff(){}},hud:{flash(){},nitro(){}},Math};
 vm.createContext(ctx);vm.runInContext(cameraCode+';this.ignite=function(wasBoost){'+ignite+'};this.restart=function(){'+restart+'};',ctx);
 let raceTime=0;
 for(let frame=0;frame<240;frame++){
  const dt=[1/60,.05,0,1/30][frame%4];timeMs+=[1000/60,200,0,1000/30][frame%4];if(frame===100)timeMs+=3000;
  const doRestart=frame===150;if(doRestart){raceTime=0;ctx.restart();}
  raceTime+=dt;if(scenario===1&&frame===0)raceTime=1;if(scenario===1&&frame===1)raceTime=1.000001;
  g.raceTime=raceTime;let was=frame%60>5&&frame%60<25,boost=frame%60>=5&&frame%60<25;
  if(scenario===1&&frame<2){was=false;boost=true;}
  car.boosting=boost;car.speed=[0,95,138,190][Math.floor(frame/40)%4];car.nitro=frame===0&&scenario===1?0:(Math.sin(frame*.05)+1)*.5;
  car.yaw=Math.sin(frame*.03)*3;car.pitch=Math.cos(frame*.02)*.55;car.roll=Math.sin(frame*.07)*.55;
  car.fwd.set(-Math.sin(car.yaw)*Math.cos(car.pitch),Math.sin(car.pitch),-Math.cos(car.yaw)*Math.cos(car.pitch));
  event=false;ctx.ignite(was);if(event)events++;ctx.dt=dt;vm.runInContext('chaseCamera(dt);',ctx);
  const dir=camera.getWorldDirection(new THREE.Vector3()),up=new THREE.Vector3(0,1,0).applyQuaternion(camera.quaternion);
  const expected=[g.lastNitro,g.kick,g.shake,cam.fov,camera.fov,cam.yaw,cam.pitch,cam.lookYaw,cam.lookPitch,...camera.position.toArray(),...dir.toArray(),...up.toArray()];
  rows.push(`{${[+(frame===0),+doRestart,scenario,+was,+boost,+event,dt,timeMs,raceTime,car.speed,car.nitro,car.yaw,car.pitch,car.roll].join(',')},{${[initial.lastNitro,initial.kick,initial.shake].join(',')}},{${initialMotion.join(',')}},70,{${expected.join(',')}}}`);
 }
}
if(sha(fs.readFileSync(mainPath))!==sha(mainBytes)||sha(fs.readFileSync(cameraPath))!==sha(cameraBytes))throw Error('Source changed during capture');fs.mkdirSync(out,{recursive:true});
fs.writeFileSync(path.join(out,'camera_fx_fixture.h'),'/* Actual ignition branch plus full Three chaseCamera; no graphics/audio/services. */\nstatic const FXCase fx_cases[]={\n'+rows.join(',\n')+'\n};\n');
const receipt={mainPath,mainSHA256:sha(mainBytes),cameraPath,cameraSHA256:sha(cameraBytes),frames:rows.length,sequences:3,ignitions:events,scope:'Actual Nitro ignition branch + full chaseCamera with real Three math, matched initial FOV and external car/boost states. Three views, strict1-second guard, restart preserving camera history, mixed dt and wall-clock gap. No actual car/input/audio/particles/lens rendering, collision-damage trigger or account execution.'};fs.writeFileSync(path.join(out,'camera-fx-oracle.json'),JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify(receipt));
