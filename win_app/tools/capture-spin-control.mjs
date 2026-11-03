import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [aimPath,carPath,out]=process.argv.slice(2),ab=fs.readFileSync(aimPath),cb=fs.readFileSync(carPath),aim=ab.toString(),cs=cb.toString();
function block(s,needle){let a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}return s.slice(a,i);}
const start=aim.indexOf('  g.spinCD = Math.max(0, g.spinCD - dt);'),end=aim.indexOf('  // EMP:',start);if(start<0||end<0)throw Error('spin hook');
const constants=cs.match(/export const CAR = [^;]+;/)[0].replace('export ', '')+cs.match(/export const SPIN = [^;]+;/)[0].replace('export ','');
let drawn=0,accepted=false,denied=false,side=0,caption='',timer=0;const math=Object.create(Math);let randomValue=0;math.random=()=>{drawn++;return randomValue;};
const car={vel:{x:2,y:0,z:-3},pos:{}},ctx={Math:math,g:{},car,inp:{},input:{hit(){return ctx.hit;}},live:null,sound:{deny(){denied=true;},boostOn(){}},hud:{flash(){},sub(s,t){caption=s;timer=t;}},THREE:{Vector3:class {}}};vm.createContext(ctx);
vm.runInContext(constants+';car.barrelRoll=function'+block(cs,'  barrelRoll(side) {').trim().slice(10)+';car.rawDodge=function'+block(cs,'  dodge(side) {').trim().slice(5)+';this.spin=function(){'+aim.slice(start,end)+'};',ctx);
car.dodge=s=>{accepted=true;side=s;car.rawDodge(s);};const rows=[];
for(const active of [false,true])for(const cd of [0,.05,.050000001,1])for(const steer of [-1,0,1])for(const rv of [.1,.499999,.5,.9])for(const dt of [0,.05]){
 drawn=0;accepted=denied=false;side=0;caption='';timer=0;randomValue=rv;
 const kick=rows.length%3===0?.8:0,speed=[0,30,60][rows.length%3],yaw=(rows.length%5)*.37,hit=rows.length%7!==0;
 ctx.g={spinCD:cd,kick};ctx.inp={steer};ctx.dt=dt;ctx.hit=hit;car.speed=speed;car.yaw=yaw;car.vel={x:2,y:0,z:-3};car.rollT=0;car.surge=0;
 if(active)ctx.spin();const expected=[ctx.g.spinCD,ctx.g.kick,car.speed,car.vel.x,car.vel.z,car.rollT,car.surge];
 const quote=s=>'"'+[...Buffer.from(s)].map(b=>b>=32&&b<127&&b!==34&&b!==92?String.fromCharCode(b):'\\'+b.toString(8).padStart(3,'0')).join('')+'"';
 rows.push('{'+[+active,+hit,cd,dt,steer,rv,kick,speed,yaw,+accepted,+denied,side,drawn].join(',')+',{'+expected.join(',')+'},'+quote(caption)+','+timer+'}');
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(aimPath))!==sha(ab)||sha(fs.readFileSync(carPath))!==sha(cb))throw Error('Source changed');fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/spin_control_fixture.h','/* Actual playerWeapons spin block and Car.dodge/barrelRoll; other weapons inert. */\nstatic const SpinCase spin_cases[]={\n'+rows.join(',\n')+'\n};\n');const receipt={aimPath,aimSHA256:sha(ab),carPath,carSHA256:sha(cb),cases:rows.length,scope:'Actual playerWeapons spin block plus Car.dodge/barrelRoll and constants; supplied RNG at0.5 boundary, active/finish gating, cooldown/steer/kick variations. Audio/flash/live effects stubbed; no real graphics or account/network calls.'};fs.writeFileSync(out+'/spin-control-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
