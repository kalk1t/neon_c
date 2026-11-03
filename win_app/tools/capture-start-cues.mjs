import fs from 'node:fs';import crypto from 'node:crypto';import path from 'node:path';import vm from 'node:vm';
const [mainPath,hudPath,out]=process.argv.slice(2),mainBytes=fs.readFileSync(mainPath),hudBytes=fs.readFileSync(hudPath),main=mainBytes.toString(),hudSrc=hudBytes.toString(),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const start=main.indexOf("  if (g.state === 'countdown') {",main.indexOf('  const playing = g.state'));
let i=main.indexOf('{',start)+1,n=1;if(start<0)throw Error('Countdown source changed');for(;i<main.length&&n;i++){if(main[i]==='{')n++;if(main[i]==='}')n--;}const branch=main.slice(start,i);
const ta=hudSrc.indexOf('    if ((this.msgTimer -= dt)'),tb=hudSrc.indexOf('    e.speed.textContent',ta);if(ta<0||tb<0)throw Error('HUD timer hook changed');const timerCode=hudSrc.slice(ta,tb);
const rows=[];let published=[];
for(let scenario=0;scenario<4;scenario++){
 const g={state:'countdown',countdown:3.999,lastCount:4,racers:[]},hud={msgTimer:0,subTimer:0,shown:false,text:''};
 hud.message=(text,duration)=>{hud.text=text;hud.msgTimer=duration;hud.shown=true;published.push(text);};
 hud.el={msg:{classList:{remove(){hud.shown=false;}}},sub:{classList:{remove(){}}}};
 const ctx={g,hud,sound:{countdown(){}},input:{takeMouse(){}},touch:null,IDLE:{},colliders:null,Math};vm.createContext(ctx);vm.runInContext('this.step=function(dt){'+branch+'};this.tick=function(dt){const e=this.el;'+timerCode+'};',ctx);
 let resume='countdown';
 for(let frame=0;frame<420;frame++){
  if(scenario===3&&frame===100){g.state="countdown";g.countdown=3.999;g.lastCount=4;}
  const paused=scenario===2&&frame>=100&&frame<140;
  if(paused&&g.state!=='paused'){resume=g.state;g.state='paused';}
  if(!paused&&g.state==='paused')g.state=resume;
  const raw=scenario===1?[0,.05,.2,1/30][frame%4]:1/60,dt=Math.min(raw,.05);
  ctx.step(dt);ctx.tick.call(hud,dt);
  const phase=g.state==='countdown'?3:g.state==='paused'?2:1;
  rows.push(`{${[(frame===0?1:scenario===3&&frame===100?2:0),+paused,phase,g.lastCount,+hud.shown,raw,hud.msgTimer].join(',')},${JSON.stringify(hud.text)}}`);
 }
}
if(sha(fs.readFileSync(mainPath))!==sha(mainBytes)||sha(fs.readFileSync(hudPath))!==sha(hudBytes))throw Error('Source changed during capture');fs.mkdirSync(out,{recursive:true});
fs.writeFileSync(path.join(out,'start_cue_fixture.h'),'/* Actual browser countdown branch and HUD timer prefix; no renderer/services. */\nstatic const CueCase cue_cases[]={\n'+rows.join(',\n')+'\n};\n');
const receipt={mainPath,mainSHA256:sha(mainBytes),hudPath,hudSHA256:sha(hudBytes),frames:rows.length,sequences:4,published,scope:'Actual countdown branch plus HUD.update timer statements, coupled to native session in C. First blank interval,3/2/1/GO!, already-clamped dt including raw stall and pause/resume plus countdown restart retaining its prior visible cue. No real input, graphics, sounds, touch, network/accounts or restart with stale other HUD messages.'};fs.writeFileSync(path.join(out,'start-cue-oracle.json'),JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify(receipt));
