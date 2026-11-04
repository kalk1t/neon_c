import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [racePath,hudPath,out]=process.argv.slice(2),rb=fs.readFileSync(racePath),hb=fs.readFileSync(hudPath),race=rb.toString(),hud=hb.toString();
function block(s,needle){const a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}return s.slice(a,i);}
const rules=block(race,'export function raceRules(r, dt)'),start=rules.indexOf('  r.sample = track.nearestLocal'),tail=rules.slice(start,rules.lastIndexOf('}')),message=block(hud,'message(text, dur = 1.2, warn = false)'),tick=hud.split('\n').find(s=>s.includes('if ((this.msgTimer -= dt)'));if(start<0||!tick)throw Error('source hooks');
let shown=false,warn=false,events=0,respawns=0;const h={msgTimer:0,el:{msg:{textContent:'',classList:{add(){shown=true;},remove(){shown=false;},toggle(k,v){warn=v;}}}}};
const ctx={Math,hud:h,track:{nearestLocal:()=>0,tans:[{}],pts:[{}]},map:{floor:34},g:{mode:'race'},r:{isPlayer:true,sample:0,car:{pos:{y:100,distanceTo(){return ctx.off;}},vel:{dot(){return ctx.along;}}}},along:0,off:0,respawn(r){events|=4;respawns++;r.lostTimer=0;r.stuck=0;}};vm.createContext(ctx);
vm.runInContext('this.message=function'+message.slice(7)+';this.tick=function(dt){const e=this.el;'+tick+'};this.rules=function(dt){const r=this.r,car=r.car;'+tail+'};',ctx);
h.message=(text,duration,warning=false)=>{if(text==='WRONG WAY')events|=1;if(text==='SIGNAL LOST')events|=2;ctx.message.call(h,text,duration,warning);};
const scenarios=[];
for(const phase of [1,4,5])for(const dt of [.016,.05]){
 scenarios.push({phase,dt,along:-20,off:0,height:100,wrong:1.19,lost:0,text:'FINISH',timer:3,warn:false,frames:220});
 scenarios.push({phase,dt,along:-20,off:700,height:100,wrong:1.2,lost:.05,text:'',timer:0,warn:false,frames:150});
 scenarios.push({phase,dt,along:0,off:700,height:100,wrong:0,lost:0,text:'FINISH',timer:3,warn:false,frames:90});
}
for(const phase of [0,2,3,6])scenarios.push({phase,dt:.05,along:-20,off:700,height:20,wrong:2,lost:1.09,text:'GO!',timer:.2,warn:false,frames:10});
for(const roam of [false,true])for(const along of [-15,-15.000001])for(const off of [650,650.000001])for(const height of [34,33.999999])for(const lost of [.05,.050000001,1.1])scenarios.push({phase:4,dt:.05,along,off,height,wrong:1.2,lost,text:'',timer:0,warn:false,roam,frames:2});
const rows=[];
for(let scenario=0;scenario<scenarios.length;scenario++){
 const s=scenarios[scenario];ctx.g.mode=s.roam?'roam':'race';ctx.r.wrongWay=s.wrong;ctx.r.lostTimer=s.lost;shown=s.timer>0;warn=s.warn;h.el.msg.textContent=s.text;h.msgTimer=s.timer;respawns=0;
 for(let frame=0;frame<s.frames;frame++){
   // Sequence input changes force loss/recovery/reset/expiry interactions.
   const off=frame>60&&frame<65?0:s.off,along=frame>100&&frame<104?0:s.along;
   ctx.off=off;ctx.along=along;ctx.r.car.pos.y=s.height;events=0;
   if([1,4,5].includes(s.phase))ctx.rules(s.dt);ctx.tick.call(h,s.dt);
   rows.push('{'+[scenario,s.phase,s.dt,along,off,s.height,+!!s.roam,s.wrong,s.lost,JSON.stringify(s.text),s.timer,+s.warn,events,ctx.r.wrongWay,ctx.r.lostTimer,JSON.stringify(h.el.msg.textContent),h.msgTimer,+shown,+warn,respawns].join(',')+'}');
 }
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(racePath))!==sha(rb)||sha(fs.readFileSync(hudPath))!==sha(hb))throw Error('Source changed');fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/recovery_feedback_fixture.h','/* Actual raceRules recovery tail + HUD.message / prepaint expiry; controlled geometry and respawn stub. */\nstatic const FeedbackCase feedback_cases[]={\n'+rows.join(',\n')+'\n};\n');const receipt={sources:[{path:racePath,sha256:sha(rb)},{path:hudPath,sha256:sha(hb)}],scenarios:scenarios.length,frames:rows.length,scope:'Actual raceRules recovery tail and HUD.message/expiry. Finished native/results map to browser finished; controlled nearest/along/off geometry and respawn stub (clears lost timer, counts calls), no car movement, storage, account, network, audio, graphics or world-collision execution. Initial FINISH/GO text supplied as fixture state.'};fs.writeFileSync(out+'/recovery-feedback-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
