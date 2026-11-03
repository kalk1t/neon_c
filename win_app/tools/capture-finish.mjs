import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath,pathToFileURL} from 'node:url';
import path from 'node:path';
import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url)),browser=path.resolve(process.argv[2]??path.join(root,'../game'));
const files=['package.json','src/main.js','src/roads.js','src/track.js'];
const hashes=()=>Promise.all(files.map(async p=>({path:p,sha256:createHash('sha256').update(await readFile(path.join(browser,p))).digest('hex')})));
const before=await hashes(),main=await readFile(path.join(browser,'src/main.js'),'utf8');
const load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [{ROADS},{Track},THREE]=await Promise.all([load('src/roads.js'),load('src/track.js'),load('node_modules/three/build/three.module.js')]);
function section(a,b){const start=main.indexOf(a),end=main.indexOf(b,start);if(start<0||end<0)throw Error('Browser structure changed');return main.slice(start,end);}
const pilot=section('function autopilot() {','// ----------------',main.indexOf('function autopilot() {'));
// Extract a single function by balanced braces; comments before the next section can vary.
function func(name){const start=main.indexOf(`function ${name}() {`);let n=0,end=start;for(let i=main.indexOf('{',start);i<main.length;i++){if(main[i]==='{')n++;if(main[i]==='}'&&!--n){end=i+1;break;}}return main.slice(start,end);}
let header='/* Actual current browser autopilot/solo-results fixtures; isolated VM only. */\n';
header+='typedef struct FinishPilotFixture {int road,sample;RRVec3 pos;double yaw;RRInput control;} FinishPilotFixture;\nstatic const FinishPilotFixture finish_pilots[]={\n';
let count=0;
for(const road of ROADS){const track=new Track(road);
 for(const sample of [0,1,16,Math.floor(track.n/2),track.n-17,track.n-1])for(const yaw of [-7,-.5,0,.5,7]){
  const pos=track.pts[sample].clone().add(new THREE.Vector3(3,11,-5)),car={pos,yaw,pitch:.2,fwd:new THREE.Vector3(0,0,-1)};
  const context={THREE,track,player:{sample},car,g:{aim:{},aimDir:new THREE.Vector3()}};vm.createContext(context);vm.runInContext(`${func('autopilot')}\nthis.result=autopilot();`,context);
  const c=context.result;header+=`{${road.id},${sample},{${pos.x},${pos.y},${pos.z}},${yaw},{${c.throttle},${c.brake},${c.steer},${c.climb},${c.boost}}},\n`;count++;
 }
}
header+='};\ntypedef struct FinishResultFixture {double laps[2],total;int best;} FinishResultFixture;\nstatic const FinishResultFixture finish_results[]={\n';
let resultCount=0;
for(const laps of [[61,59],[59,61],[60,60],[.123456,.654321],[3600,3599],[.00001,.00001]]){
 const elements=new Map();const $=id=>{if(!elements.has(id))elements.set(id,{classList:{toggle(){},remove(){}},textContent:'',innerHTML:''});return elements.get(id);};
 const player={lapTimes:laps,coins:0,name:'Fixture',isPlayer:true,takedowns:0};
 const context={g:{state:'finished',racers:[player],mode:'race',finishPos:1},player,$,fmt:x=>x,ordinal:String,esc:x=>x,account:{recordRace(){}},unlockNext:()=>null,road:{id:1},roadsOpen:()=>1,trackEvent(){}};
 vm.createContext(context);vm.runInContext(`${func('showResults')}\nshowResults();`,context);
 const best=Number(/class="best">LAP (\d)/.exec($('res-laps').innerHTML)[1])-1;
 header+=`{{${laps.join(',')}},${$('res-total').textContent},${best}},\n`;resultCount++;
}
header+='};\n';const after=await hashes();if(JSON.stringify(before)!==JSON.stringify(after))throw Error('Reference changed');
await writeFile(path.join(root,'tests/generated_finish_reference.h'),header);
await writeFile(path.join(root,'reference/finish.json'),JSON.stringify({captured_utc:new Date().toISOString(),sources:before,content_hashes_verified_twice:true,pilot_cases:count,solo_result_cases:resultCount,scope:'Actual autopilot and solo showResults executed in isolated VM; account/telemetry/unlock handlers are no-op adapters, no storage/network/renderer.'},null,2)+'\n');
console.log({pilot_cases:count,solo_result_cases:resultCount});
