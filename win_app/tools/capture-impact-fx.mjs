import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [mainPath,cameraPath,out]=process.argv.slice(2),mb=fs.readFileSync(mainPath),cb=fs.readFileSync(cameraPath),main=mb.toString(),camera=cb.toString();
function block(s,needle){const a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}return s.slice(a,i);}
const impact=block(main,'if (res && res.impact > 4)'),ignition=block(main,'if (car.boosting && !wasBoost'),decay=camera.split('\n').find(s=>s.includes('g.shake = Math.max(0, g.shake - dt * 2.2)'));
if(!decay)throw Error('decay');
const position={clone(){return {addScaledVector(){return this;}}}};
const ctx={g:{},car:{pos:position},wasBoost:false,res:{},hud:{flash(){},nitro(){}},sound:{boostOn(){},hit(){}},weapons:{impact(){}},Math,dt:0};vm.createContext(ctx);
const rows=[];
for(const shake of [0,.1,.85,1.19,1.2,1.4])for(const hit of [0,3.999999999,4,4.000000001,12,30,100])for(const dt of [0,.016,.05,1])for(const ignite of [false,true]){
 ctx.g={shake,lastNitro:-9,raceTime:2};ctx.car.boosting=ignite;ctx.res={impact:hit,n:{}};ctx.dt=dt;
 vm.runInContext(ignition+';'+impact,ctx);const before=ctx.g.shake,event=!!ctx.g.hits;vm.runInContext(decay,ctx);
 rows.push('{'+[shake,hit,dt,+ignite,+event,before,ctx.g.shake].join(',')+'}');
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(mainPath))!==sha(mb)||sha(fs.readFileSync(cameraPath))!==sha(cb))throw Error('Source changed');
fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/impact_fx_fixture.h','/* Actual main Nitro->impact shake branches and camera decay, stubbed audio/effects. */\nstatic const ImpactCase impact_cases[]={\n'+rows.join(',\n')+'\n};\n');const receipt={mainPath,mainSHA256:sha(mb),cameraPath,cameraSHA256:sha(cb),cases:rows.length,scope:'Actual Nitro ignition then collision shake branch, strict4 threshold, cap1.2 and camera decay. Audio/particles stubbed, supplied impact inputs; no graphics or performance evidence.'};fs.writeFileSync(out+'/impact-fx-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
