// Reference-only CPU execution. Does not import main.js or initialize graphics.
import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath,pathToFileURL} from 'node:url';
import path from 'node:path';
import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url));
const browser=process.argv[2]?path.resolve(process.argv[2]):path.resolve(root,'../game');
const inputs=['package.json','src/main.js','src/car.js','src/track.js','src/roads.js','src/maps.js','node_modules/three/package.json'];
const hashes=()=>Promise.all(inputs.map(async p=>({path:p,sha256:createHash('sha256').update(await readFile(path.join(browser,p))).digest('hex')})));
const before=await hashes();
const load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [THREE,{Car},{Track},{ROADS}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/car.js'),load('src/track.js'),load('src/roads.js')]);
const main=await readFile(path.join(browser,'src/main.js'),'utf8');
function section(start,end){const a=main.indexOf(start),b=main.indexOf(end,a);if(a<0||b<0)throw Error(`Changed source structure: ${start}`);return main.slice(a,b);}
const context={THREE,map:{floor:34},GATE:{halfW:23,y0:109,y1:142},completeLap(){throw Error('Unexpected lap in recovery fixture');}};
vm.createContext(context);
vm.runInContext(`${section('function respawn(r, keepHp = false) {','// how hard each hit')}\nconst _d=new THREE.Vector3();\n${section('function raceRules(r, dt) {','/** Rivals occasionally fire')}\nthis.step=raceRules;`,context);
const f=x=>{if(!Number.isFinite(x))throw Error('Nonfinite reference');return x.toPrecision(17);};
const v=p=>`{${[p.x,p.y,p.z].map(f).join(',')}}`;
const rows=[],scenarios=[];
const normal={along:30,x:0,y:118,dt:.05,message:0,roam:false};
function scenario(name,specs){
  const id=scenarios.length;let messages=[],respawns=0;
  const car=Object.create(Car.prototype);
  Object.assign(car,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),ribbons:[],sync(){},setVisible(){},boost:.4});
  const r={car,isPlayer:true,finished:true,prevPos:new THREE.Vector3(),passed:new Set(),nextCP:0,
    sample:0,wrongWay:0,lostTimer:0,lastSafe:{pos:new THREE.Vector3(0,118,12),yaw:0}};
  context.track={checkpoints:[],nearestLocal:()=>0,nearestGlobal:()=>{respawns++;return 0;},pts:[new THREE.Vector3(0,118,0)],tans:[new THREE.Vector3(0,0,-1)]};
  context.g={mode:'race',raceTime:10,aim:{yaw:0,pitch:0}};
  context.hud={msgTimer:0,message(text,seconds){messages.push(text);this.msgTimer=seconds;},flash(){}};
  for(const spec of specs){
    const p={...normal,...spec};messages=[];const oldRespawns=respawns;
    context.g.mode=p.roam?'roam':'race';context.hud.msgTimer=p.message;
    car.pos.set(p.x,p.y,0);r.prevPos.copy(car.pos);car.vel.set(0,0,-p.along);
    car.speed=42;car.boost=.4;car.stun=1;car.rollT=.5;car.surge=1;
    const off=car.pos.distanceTo(context.track.pts[0]);
    context.step(r,p.dt);
    const events=(messages.includes('WRONG WAY')?1:0)|(messages.includes('SIGNAL LOST')?2:0)|(respawns>oldRespawns?4:0);
    rows.push(`{${id},${p.roam?1:0},${[p.along,off,p.y,p.dt,p.message,r.wrongWay,r.lostTimer].map(f).join(',')},${events},${v(car.pos)},${f(car.speed)},${f(car.boost)}}`);
  }
  scenarios.push({name,steps:specs.length});
}
const repeat=(n,x)=>Array.from({length:n},()=>({...x}));
scenario('safe',repeat(40,{}));
scenario('off-distance-boundary',[{x:650},{x:650.001},{x:650},{x:649.999},...repeat(30,{x:651})]);
scenario('floor-boundary',[{y:34},{y:33.999},{y:34},...repeat(30,{y:33.999})]);
scenario('signal-lost-strict-threshold',[{x:651,dt:.05},{x:651,dt:.000001},...repeat(30,{x:651})]);
scenario('reverse-threshold-and-reset',[...repeat(30,{along:-15}),...repeat(30,{along:-15.001}),{along:-15},{along:-14.999}]);
scenario('reverse-message-suppression',[...repeat(35,{along:-20,message:.3}),{along:-20,message:0},...repeat(5,{along:0})]);
scenario('roam-no-warnings-or-recovery',repeat(40,{along:-30,x:900,y:0,roam:true}));
scenario('intermittent-signal-reset',[...repeat(10,{x:700}),{},...repeat(10,{x:700}),{},...repeat(25,{x:700})]);
scenario('simultaneous-warnings',[...repeat(26,{along:-30}),{along:-30,x:700,dt:.05},{along:-30,x:700,dt:.02}]);
const track=new Track(ROADS[0]),tangents=[],nearest=[];
for(let i=0;i<track.n;i+=17)tangents.push(`{${i},${v(track.tans[i])}}`);
for(const hint of [0,10,track.n-5,Math.floor(track.n/2)])for(const offset of [-60,-30,-5,0,10,30,60]){
  const index=(hint+offset+track.n)%track.n;
  const pos=track.pts[index].clone().add(new THREE.Vector3(.37,.19,-.23));
  nearest.push(`{${hint},${v(pos)},${track.nearestLocal(pos,hint)},${track.nearestGlobal(pos)}}`);
}
const after=await hashes();if(JSON.stringify(before)!==JSON.stringify(after))throw Error('Reference inputs changed; existing fixtures preserved.');
await writeFile(path.join(root,'tests/generated_recovery_reference.h'),`/* Actual browser player raceRules/respawn/Car.place and Skyline Track methods. */\nstatic const RecoveryFixture recovery_fixtures[]={\n${rows.join(',\n')}\n};\nstatic const TangentFixture tangent_fixtures[]={\n${tangents.join(',\n')}\n};\nstatic const NearestFixture nearest_fixtures[]={\n${nearest.join(',\n')}\n};\n`);
await writeFile(path.join(root,'reference/recovery-rules.json'),JSON.stringify({captured_utc:new Date().toISOString(),sources:after,content_hashes_verified_twice:true,scenarios,steps:rows.length,tangent_samples:tangents.length,nearest_cases:nearest.length,scope:'Actual player warning/recovery conditions and respawn placement; actual Skyline local/global nearest and arc-mapped tangents. No graphics, services, collisions, UI execution or AI.'},null,2)+'\n');
console.log(`${rows.length} browser recovery steps, ${scenarios.length} scenarios; ${tangents.length} tangents; ${nearest.length} local/global nearest cases. No graphics.`);
