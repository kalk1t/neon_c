import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [source,out]=process.argv.slice(2),buf=fs.readFileSync(source),s=buf.toString();let a=s.indexOf('export function updateAim(inp)'),i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}if(a<0||n)throw Error('updateAim hook');
const rate=Number(s.match(/const MOUSE_RAD_PER_PX = ([0-9.]+);/)?.[1]);if(!rate)throw Error('mouse rate');
const ctx={MOUSE_RAD_PER_PX:rate,THREE:{MathUtils:{clamp:(x,l,h)=>Math.max(l,Math.min(h,x))}},Math,settings:{sens:1,invert:false},g:{aim:{yaw:0,pitch:0},aimDir:{set(x,y,z){this.values=[x,y,z];}}},player:{car:{yaw:0,pitch:0}},cam:{lookYaw:0,lookPitch:0,looking:false},input:{takeMouse(){return ctx.motion;}}};vm.createContext(ctx);vm.runInContext(s.slice(a,i).replace('export function','function')+';this.run=updateAim;',ctx);
const rows=[];
for(const sensitivity of [.3,1,2.5])for(const invert of [false,true])for(const look of [false,true]){
 for(let j=0;j<32;j++){
  const reset=j===0,initial=[6.1,.15,.3,.1];if(reset){[ctx.g.aim.yaw,ctx.g.aim.pitch,ctx.cam.lookYaw,ctx.cam.lookPitch]=initial;}
  ctx.settings={sens:sensitivity,invert};ctx.player.car={yaw:6.1+j*.17,pitch:.3*Math.sin(j)};
  ctx.motion={dx:[0,1,-1,700,-900,10,-10,3500][j%8],dy:[1,-1,0,700,-900,30,-30,-3500][j%8]};
  const steer=[0,-1,1,.2][j%4],climb=[0,.4,-1][j%3],inp={steer,climb,keySteer:steer!==0,keyClimb:climb!==0,look};ctx.run(inp);
  const expected=[ctx.g.aim.yaw,ctx.g.aim.pitch,ctx.cam.lookYaw,ctx.cam.lookPitch,inp.steer,inp.climb,...ctx.g.aimDir.values];
  rows.push('{'+[+reset,+look,+invert,sensitivity,ctx.player.car.yaw,ctx.player.car.pitch,ctx.motion.dx,ctx.motion.dy,steer,climb].join(',')+',{'+initial.join(',')+'},{'+expected.join(',')+'}}');
 }
}
fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/mouse_aim_fixture.h','/* Actual browser updateAim, accumulated sequential mouse input and settings. */\nstatic const AimCase aim_cases[]={\n'+rows.join(',\n')+'\n};\n');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(source))!==sha(buf))throw Error('Source changed');const receipt={source,sha256:sha(buf),states:rows.length,sequences:12,scope:'Actual updateAim with source clamp semantics, consumed pixel deltas, sensitivity/invert and sequential RMB/keyboard/gamepad-like axes. No browser mouse-event filtering, OS capture, GPU or real hardware input.'};fs.writeFileSync(out+'/mouse-aim-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
