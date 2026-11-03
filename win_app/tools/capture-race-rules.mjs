// Executes only extracted pure race rules and Three.js vectors/track geometry.
// Never imports main.js or constructs a renderer/window; browser files are read-only.
import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath,pathToFileURL} from 'node:url';
import path from 'node:path';
import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url));
const browser=process.argv[2]?path.resolve(process.argv[2]):path.resolve(root,'../game');
const inputs=['package.json','src/main.js','src/roads.js','src/track.js','src/maps.js','src/ai.js','node_modules/three/package.json'];
const hashSources=()=>Promise.all(inputs.map(async p=>({path:p,sha256:createHash('sha256').update(await readFile(path.join(browser,p))).digest('hex')})));
const before=await hashSources();
const load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [THREE,{ROADS},{Track}]=await Promise.all([
  load('node_modules/three/build/three.module.js'),load('src/roads.js'),load('src/track.js')]);
const main=await readFile(path.join(browser,'src/main.js'),'utf8');
function section(start,end){
  const a=main.indexOf(start),b=main.indexOf(end,a);
  if(a<0 || b<0)throw Error(`Browser source structure changed: ${start}`);
  return main.slice(a,b);
}
const deck=main.match(/^const DECK_Y = .*;$/m)?.[0];
const gate=main.match(/^const GATE = .*;$/m)?.[0];
if(!deck || !gate)throw Error('Browser finish bounds unavailable');
const context={THREE,map:{floor:34},respawn(){throw Error('Fixture entered out-of-scope respawn');}};
vm.createContext(context);
vm.runInContext(`${deck}\n${gate}\n${section('function completeLap(r) {','const _d =')}\nconst _d=new THREE.Vector3();\n${section('function raceRules(r, dt) {','/** Rivals occasionally fire')}\nthis.step=raceRules;`,context);
const vec=p=>[p.x,p.y,p.z];
const number=x=>{if(!Number.isFinite(x))throw Error('Nonfinite reference');return x.toPrecision(17);};
const v=p=>`{${vec(p).map(number).join(',')}}`;
const allGates=[],rows=[],scenarios=[];
function scenario(name,mode,gates,steps){
  const id=scenarios.length,offset=allGates.length;
  allGates.push(...gates);
  let clock=0,adds=0;
  const passed=new Set();const add=passed.add.bind(passed);
  passed.add=x=>{adds++;return add(x);};
  const spawn=new THREE.Vector3(0,118,70);
  const r={isPlayer:false,car:{pos:spawn.clone(),vel:new THREE.Vector3(),speed:100,boost:0.1},
    prevPos:spawn.clone(),passed,lap:1,nextCP:0,lapTime:0,lapTimes:[],finished:false,finishTime:null,
    lastSafe:{pos:spawn.clone(),yaw:0},sample:0,stuck:0,lostTimer:0};
  context.g={mode:['race','time','roam'][mode],laps:mode===0?2:Infinity,raceTime:0};
  context.track={checkpoints:gates,nearestLocal:()=>0,pts:[spawn],tans:[new THREE.Vector3(0,0,-1)]};
  for(const [previous,current] of steps){
    const dt=0.017+(rows.length%3)*0.003;clock+=dt;
    context.g.raceTime=clock;r.prevPos.copy(previous);r.car.pos.copy(current);
    context.track.pts[0]=current;
    const oldLaps=r.lapTimes.length,oldAdds=adds,wasFinished=r.finished;
    // Matches updateRacer before raceRules in browser main.js.
    if(!r.finished)r.lapTime+=dt;
    context.step(r,dt);
    const events=(adds>oldAdds?1:0)|(r.lapTimes.length>oldLaps?2:0)|(!wasFinished&&r.finished?4:0);
    rows.push(`{${id},${mode},${offset},${gates.length},${v(previous)},${v(current)},${number(dt)},${number(clock)},${r.lap},${r.nextCP},${r.lapTimes.length},${r.finished?1:0},${[r.lapTime,r.lapTimes.at(-1)??0,r.lapTimes.length?Math.min(...r.lapTimes):0,r.finishTime??0,r.car.boost].map(number).join(',')},${v(r.lastSafe.pos)},${number(r.lastSafe.yaw)},${events}}`);
  }
  scenarios.push({name,mode,steps:steps.length});
}
const V=(x,y,z)=>new THREE.Vector3(x,y,z);
const finish=[V(0,118,10),V(0,118,-10)];
const cross=cp=>[cp.pos.clone().addScaledVector(cp.forward,-20),cp.pos.clone().addScaledVector(cp.forward,20)];
for(const road of ROADS){
  const track=new Track(road),gates=track.checkpoints;
  for(const mode of [0,1,2]){
    const steps=[finish,cross(gates[1]),cross(gates[0]),cross(gates[0])];
    for(let lap=0;lap<3;lap++){
      for(const cp of gates)steps.push(cross(cp));
      steps.push(finish);
    }
    scenario(`road-${road.id}-${['two-lap','time-attack','free-roam'][mode]}`,mode,gates,steps);
  }
}
const cp={index:0,pos:V(0,118,-100),forward:V(0,0,-1),yaw:0};
scenario('checkpoint-aperture-direction-endpoint',0,[cp],[
  [V(0,118,-100),V(0,118,-120)], // previous on plane: rejected
  [V(0,118,-120),V(0,118,-80)], // reverse: rejected
  [V(0,118,-80),V(15,118,-120)], // strict radius: rejected
  [V(0,118,-80),V(30,118,-120)], // segment intersects aperture, endpoint outside: rejected
  [V(40,118,-80),V(14.999,118,-120)], // endpoint inside despite segment outside: accepted
  finish,finish]);
for(const [name,end] of [['x-edge',V(23,118,-10)],['lower-edge',V(0,109,-10)],['upper-edge',V(0,142,-10)]])
  scenario(`finish-${name}`,0,[cp],[cross(cp),[finish[0],end],finish]);
scenario('finish-reverse-and-plane',0,[cp],[cross(cp),[finish[1],finish[0]],[V(0,118,0),finish[1]],finish]);
const sources=await hashSources();
if(JSON.stringify(before)!==JSON.stringify(sources))throw Error('Browser inputs changed during capture; existing fixtures preserved.');
const pkg=JSON.parse(await readFile(path.join(browser,'package.json'),'utf8'));
await writeFile(path.join(root,'tests/generated_race_reference.h'),`/* Actual extracted browser raceRules/completeLap; generated by tools/capture-race-rules.mjs. */\nstatic const RRGate race_gates[]={\n${allGates.map(cp=>`{${v(cp.pos)},${v(cp.forward)},${number(cp.yaw)}}`).join(',\n')}\n};\nstatic const RaceFixture race_fixtures[]={\n${rows.join(',\n')}\n};\n`);
await writeFile(path.join(root,'reference/race-rules.json'),JSON.stringify({captured_utc:new Date().toISOString(),package_version:pkg.version,source_identity:'Working file hashes; Git metadata unavailable. Not a frozen/deployed release.',content_hashes_verified_twice:true,scope:'Ordered checkpoints, endpoint apertures, boost, last-safe transform, two-lap finish/time attack/free roam. No rendering, collisions, warnings, respawn, rewards, UI or services.',sources,scenarios,steps:rows.length},null,2)+'\n');
console.log(`${rows.length} actual browser rule steps; ${scenarios.length} scenarios; ${ROADS.length} roads; browser ${pkg.version}. No graphics or browser writes.`);
