import fs from 'node:fs';import crypto from 'node:crypto';import vm from 'node:vm';import path from 'node:path';import {pathToFileURL} from 'node:url';
const [racePath,hudPath,threePath,out]=process.argv.slice(2),rb=fs.readFileSync(racePath),hb=fs.readFileSync(hudPath),race=rb.toString(),hs=hb.toString(),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
function block(s,needle){const a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}return s.slice(a,i);}
const rules=block(race,'function raceRules(r, dt)'),sub=block(hs,'sub(text, dur = 1.6)'),fmt=block(hs,'export const fmt = (t) =>').replace('export const fmt','this.fmt');
const lapA=race.indexOf('  hud.message(r.lap === g.laps'),lapB=race.indexOf('\n}',lapA);if(lapA<0||lapB<0)throw Error('lap caption hook');const lap=race.slice(lapA,lapB);
const ta=hs.indexOf('    if ((this.msgTimer -= dt)'),tb=hs.indexOf('    e.speed.textContent',ta);if(ta<0||tb<0)throw Error('HUD timers hook');
const THREE=await import(pathToFileURL(path.resolve(threePath)));const hud={msgTimer:0,subTimer:0,shown:false,el:{msg:{classList:{remove(){}}},sub:{textContent:'',classList:{add(){hud.shown=true;},remove(){hud.shown=false;}}}},message(){},sub:null};
const ctx={THREE,_d:new THREE.Vector3(),g:{mode:'race',raceTime:0,laps:2},map:{floor:34},GATE:{halfW:23,y0:109,y1:142},hud,sound:{checkpoint(){}},completeLap(){},respawn(){},setGateText(){},Math};vm.createContext(ctx);
vm.runInContext(fmt+';'+rules+';this.subFn=function'+sub.slice(3)+';this.tick=function(dt){const e=this.el;'+hs.slice(ta,tb)+'};this.lapCaption=function(lapT,isBest){const r={lap:2};'+lap+'};',ctx);hud.sub=ctx.subFn.bind(hud);
const rows=[],cstr=s=>'"'+[...Buffer.from(s)].map(b=>b>=32&&b<127&&b!==34&&b!==92?String.fromCharCode(b):'\\'+b.toString(8).padStart(3,'0')).join('')+'"';
function record(op,time,best,dt){rows.push(`{${op},2,8,${+best},${time},${dt},${hud.subTimer},${+hud.shown},${cstr(hud.el.sub.textContent)}}`);}
function checkpoint(time){ctx.track={checkpoints:Array.from({length:8},(_,index)=>({index,pos:new THREE.Vector3(0,115,100),forward:new THREE.Vector3(0,0,-1),yaw:0})),pts:[new THREE.Vector3(0,115,100)],tans:[new THREE.Vector3(0,0,-1)],nearestLocal:()=>0};ctx.r={isPlayer:true,car:{pos:new THREE.Vector3(0,115,99),vel:new THREE.Vector3(),boost:0,speed:0},prevPos:new THREE.Vector3(0,115,101),finished:false,nextCP:1,passed:new Set([0]),sample:0,wrongWay:0,lostTimer:0,lapTime:time};vm.runInContext('raceRules(r,1/60);',ctx);}
for(const time of [0,.005,1.125,59.995,60.015,6100.999])for(const op of [0,1,2]){
 if(op===0)checkpoint(time);else ctx.lapCaption(time,op===1);record(op===0?0:1,time,op===1,0);
 for(const dt of op===0?[0,.7,.7,.001]:[0,1.2,1.2,.001]){ctx.tick.call(hud,dt);record(2,time,false,dt);}
}
checkpoint(12.5);record(0,12.5,false,0);ctx.lapCaption(60.015,true);record(1,60.015,true,0);
if(sha(fs.readFileSync(racePath))!==sha(rb)||sha(fs.readFileSync(hudPath))!==sha(hb))throw Error('Source changed');fs.mkdirSync(out,{recursive:true});
fs.writeFileSync(path.join(out,'sub_notice_fixture.h'),'/* Actual source checkpoint/lap captions and HUD.sub/timers; no storage branch. */\nstatic const SubCase sub_cases[]={\n'+rows.join(',\n')+'\n};\n');
const receipt={racePath,raceSHA256:sha(rb),hudPath,hudSHA256:sha(hb),states:rows.length,scope:'Actual raceRules checkpoint and nonfinished completeLap caption tail, actual HUD.sub and timer prefix. No localStorage/account/reward branch executed; camera/render/audio inert. Unicode, decimal rounding, overwrite/expiry captured.'};fs.writeFileSync(path.join(out,'sub-notice-oracle.json'),JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify(receipt));
