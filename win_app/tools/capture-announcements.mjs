import fs from 'node:fs';import crypto from 'node:crypto';import vm from 'node:vm';
const [racePath,hudPath,out]=process.argv.slice(2),rb=fs.readFileSync(racePath),hb=fs.readFileSync(hudPath),race=rb.toString(),hud=hb.toString();
function block(s,needle){let a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}return s.slice(a,i);}
const message=block(hud,'message(text, dur = 1.2, warn = false)'),lap=race.split('\n').find(s=>s.includes("hud.message(r.lap === g.laps")),finish=race.split('\n').find(s=>s.includes('hud.message(g.racers.length > 1'));
const timer=hud.split('\n').find(s=>s.includes('if ((this.msgTimer -= dt)'));
if(!lap||!finish||!timer)throw Error('source hook');
let shown=false,warn=false;const m={msgTimer:0,el:{msg:{textContent:'',classList:{add(){shown=true;},remove(){shown=false;},toggle(k,v){warn=v;}}}}};
const ctx={hud:m,g:{laps:2,racers:[{}]},r:{lap:2},Math,ordinal:n=>String(n)};vm.createContext(ctx);
vm.runInContext('this.messageFn=function'+message.slice(7)+';this.tick=function(dt){const e=this.el;'+timer+'};this.lapFn=function(){'+lap+'};this.finishFn=function(){'+finish+'};',ctx);m.message=ctx.messageFn.bind(m);
const rows=[];const quoted=s=>JSON.stringify(s);function record(op,lapN=0,total=0,dt=0,input='',duration=0,inputWarn=false){rows.push([op,lapN,total,dt,quoted(input),duration,+inputWarn,quoted(m.el.msg.textContent),m.msgTimer,+shown,+warn].join(','));}
{
 for(const [lapN,total] of [[2,2],[3,0],[10,0],[2,3]]){
  m.message('GO!',.9);record(0,0,0,0,'GO!',.9);
  ctx.r.lap=lapN;ctx.g.laps=total||Infinity;ctx.lapFn();record(1,lapN,total);
  ctx.tick.call(m,.4);record(3,0,0,.4);
  record(4); // restart/menu retain HUD content/timer
  for(const [text,dur] of [['MISSED GATE',1.4],['WRONG WAY',.8],['SIGNAL LOST',1.2]]){
   m.message(text,dur,true);record(0,0,0,0,text,dur,true);
   ctx.tick.call(m,dur);record(3,0,0,dur);
   ctx.tick.call(m,.0001);record(3,0,0,.0001);
  }
  ctx.finishFn();record(2);
  for(const dt of [0,1,1,1,.01]){ctx.tick.call(m,dt);record(3,0,0,dt);}
 }
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(racePath))!==sha(rb)||sha(fs.readFileSync(hudPath))!==sha(hb))throw Error('Source changed');
fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/hud_message_fixture.h','/* Actual source HUD.message/update timer and lap/solo-finish announcement statements. */\nstatic const MessageCase message_cases[]={\n'+rows.map(s=>'{'+s+'}').join(',\n')+'\n};\n');
const receipt={racePath,raceSHA256:sha(rb),hudPath,hudSHA256:sha(hb),states:rows.length,scope:'Actual HUD.message/timer, nonfinished lap and solo-finish announcement statements; warning text/durations supplied from raceRules. No completeLap account/storage/reward execution, DOM pixels or real input/world simulation.'};fs.writeFileSync(out+'/hud-message-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
