import fs from 'node:fs';import crypto from 'node:crypto';import path from 'node:path';import vm from 'node:vm';import {pathToFileURL} from 'node:url';
const [source,threePath,out]=process.argv.slice(2),bytes=fs.readFileSync(source),text=bytes.toString(),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
let a=text.indexOf('function raceRules(r, dt) {'),i=text.indexOf('{',a)+1,n=1;if(a<0)throw Error('Missing raceRules');for(;i<text.length&&n;i++){if(text[i]==='{')n++;if(text[i]==='}')n--;}const fn=text.slice(a,i);
const THREE=await import(pathToFileURL(path.resolve(threePath))),rows=[];let countWarnings=0;
const coursePath=path.join(path.dirname(source),"course.js"),courseBytes=fs.readFileSync(coursePath);const deck=Number(courseBytes.toString().match(/export const DECK_Y = ([0-9.]+);/)[1]);const gateExpr=text.match(/export const GATE = (\{[^\n]+\});/)[1];const gateCtx={THREE,DECK_Y:deck};vm.createContext(gateCtx);vm.runInContext("this.GATE="+gateExpr,gateCtx);const gate=gateCtx.GATE;
const positions=[[0,115.5,1,0],[22.99999,115.5,1,0],[23,115.5,1,0],[-23,115.5,1,0],[-22.99999,115.5,1,0],[0,109,1,0],[0,109.00001,1,0],[0,141.99999,1,0],[0,142,1,0],[0,115.5,0,-1],[0,115.5,-1,1],[0,115.5,1,.00001],[0,115.5,1,-1]];
for(const mode of [0,1,2])for(const finished of [false,true])for(const next of [0,1,2,3])for(const time of [4.99999,5,5.00001,20])for(const [x,y,pz,z] of positions){
 const messages=[],car={pos:new THREE.Vector3(x,y,z),vel:new THREE.Vector3(),boost:.5,speed:0};
 const r={isPlayer:true,car,prevPos:new THREE.Vector3(x,y,pz),finished,nextCP:next,passed:new Set(Array.from({length:next},(_,i)=>i)),sample:0,wrongWay:0,lostTimer:0,lapTime:2};
 const track={checkpoints:Array.from({length:3},(_,index)=>({index,pos:new THREE.Vector3(1000,115,1000),forward:new THREE.Vector3(0,0,-1),yaw:0})),pts:[new THREE.Vector3(0,115,0)],tans:[new THREE.Vector3(0,0,-1)],nearestLocal:()=>0};
 const ctx={THREE,_d:new THREE.Vector3(),g:{mode:mode===2?'roam':mode===1?'time':'race',raceTime:time},track,map:{floor:34},GATE:gate,hud:{msgTimer:0,message:(text,duration)=>{messages.push({text,duration});ctx.hud.msgTimer=duration;},sub(){}},sound:{checkpoint(){}},completeLap(){},respawn(){},Math,r};
 vm.createContext(ctx);vm.runInContext(fn+';raceRules(r,1/60);',ctx);
 const event=messages.find(m=>m.text==='MISSED GATE');if(event)countWarnings++;
 rows.push(`{${[mode,+finished,next,3,time].join(',')},{${[x,y,pz].join(',')}},{${[x,y,z].join(',')}},${!!event},${event?.duration??0}}`);
}
if(sha(fs.readFileSync(source))!==sha(bytes))throw Error('Race source changed');fs.mkdirSync(out,{recursive:true});
fs.writeFileSync(path.join(out,'missed_gate_fixture.h'),'/* Actual source raceRules with inert HUD recording, no account/lap completion side effects. */\nstatic const WarningCase warning_cases[]={\n'+rows.join(',\n')+'\n};\n');
const receipt={source,sourceSHA256:sha(bytes),coursePath,courseSHA256:sha(courseBytes),gate:{halfW:gate.halfW,y0:gate.y0,y1:gate.y1},cases:rows.length,warnings:countWarnings,scope:'Actual raceRules boundary behavior and1.4-second message payload. Ordered passed.size equals nextCP. Other checkpoint/motion/recovery behavior held neutral; completeLap/respawn inert. No graphics, rewards, network or account execution.'};fs.writeFileSync(path.join(out,'missed-gate-oracle.json'),JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify(receipt));
