import fs from 'node:fs';import crypto from 'node:crypto';import vm from 'node:vm';import {pathToFileURL} from 'node:url';import path from 'node:path';
const [reference,current,threePath,out]=process.argv.slice(2);
const sha=s=>crypto.createHash('sha256').update(s).digest('hex');
function extract(s){const begin=s.indexOf('function chaseCamera(dt) {');if(begin<0)throw Error('No chaseCamera');let i=s.indexOf('{',begin),n=1;for(i++;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}if(n)throw Error('Unclosed camera');return s.slice(begin,i);}
const referenceBytes=fs.readFileSync(reference),currentBytes=fs.readFileSync(current);
const fn=extract(referenceBytes.toString()),currentFn=extract(currentBytes.toString());
const normalized=s=>s.replace(/\/\/[^\n]*/g,'').replace(/\s+/g,'');
if(normalized(fn)!==normalized(currentFn))throw Error('Current camera behavior differs from pinned reference');
const THREE=await import(pathToFileURL(path.resolve(threePath)));
const rows=[];let poses=0;
for(const mode of [0,1,2])for(const alive of [true,false])for(const yaw of [0,Math.PI/2,-1.2,2.7])for(const pitch of [-.45,0,.4])for(const roll of [-.55,0,.55])for(const [ly,lp] of [[0,0],[.012499999,0],[.012500001,0],[0,-.2],[Math.PI*2+.01,0]]){
 const fwd=new THREE.Vector3(-Math.sin(yaw)*Math.cos(pitch),Math.sin(pitch),-Math.cos(yaw)*Math.cos(pitch));
 const car={pos:new THREE.Vector3(3,100,4),fwd,yaw,pitch,roll,speed:80,nitro:0,model:{visible:true}};
 const camera=new THREE.PerspectiveCamera();
 const uniforms=Object.fromEntries(['uSpeed','uCA','uLines','uAspect'].map(k=>[k,{value:0}]));
 const ctx={THREE,car,CAR:{MAX:95},cam:{yaw:0,pitch:0,lookYaw:ly,lookPitch:lp,looking:true,fov:68},g:{camMode:mode,interior:false,shake:0,kick:0},CAMS:[{d:9,h:2.7},{d:15,h:4.8},{d:0,h:0}],player:{alive},camera,dash:{show(){}},lens:{uniforms},performance:{now:()=>0},Math};
 vm.createContext(ctx);vm.runInContext(fn+';chaseCamera(1/60);',ctx);
 const active=!car.model.visible,dir=camera.getWorldDirection(new THREE.Vector3()),up=new THREE.Vector3(0,1,0).applyQuaternion(camera.quaternion);
 const values=[...camera.position.toArray(),...dir.toArray(),...up.toArray()];if(active)poses++;
 rows.push(`{${mode},${alive},${ly},${lp},${roll},{${fwd.toArray().join(',')}},${active},{${values.join(',')}}}`);
}
if(sha(fs.readFileSync(current))!==sha(currentBytes))throw Error('Current camera changed during fixture capture');
fs.mkdirSync(out,{recursive:true});
fs.writeFileSync(path.join(out,'first_person_fixture.h'),'/* Actual pinned source chaseCamera; full Three camera pose. */\nstatic const SeatCase seat_cases[]={\n'+rows.join(',\n')+'\n};\n');
const receipt={reference,referenceSHA256:sha(referenceBytes),current,currentSHA256:sha(currentBytes),normalizedChaseCameraEqual:true,cases:rows.length,rawSeatPoses:poses,scope:'Actual chaseCamera with real Three PerspectiveCamera; no graphics/audio/account. Alive/dead, three modes, yaw/pitch/roll and orbit threshold/wrap cases. Shake/nitro/punch disabled; no human input or pixel acceptance.'};
fs.writeFileSync(path.join(out,'first-person-oracle.json'),JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify(receipt));
