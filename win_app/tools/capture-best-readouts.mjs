import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [mainPath,hudPath,out]=process.argv.slice(2),mb=fs.readFileSync(mainPath),hb=fs.readFileSync(hudPath),main=mb.toString(),hud=hb.toString();
function block(s,needle){const a=s.indexOf(needle);if(a<0)throw Error(needle);let i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}return s.slice(a,i);}
const fmt=block(hud,'export const fmt = (t) =>').replace('export const','const');
const homeStart=main.indexOf('  g.best = Number(localStorage.getItem(bestKey())) || null;'),homeEnd=main.indexOf('  showMapInfo();',homeStart);if(homeStart<0||homeEnd<0)throw Error('home best hook');
const line=main.split('\n').find(s=>s.includes('<span class="best">${locked ?'));if(!line)throw Error('row label hook');const expression=line.trim().slice(2,-1);
let stored='0';const menu={textContent:''},context={g:{},localStorage:{getItem(){return stored;}},bestKey:()=> 'fixture-road', $:id=>{if(id!=='menu-best')throw Error('unexpected DOM');return menu;},locked:false,r:{id:1},best:null,Math,isFinite};vm.createContext(context);
vm.runInContext(fmt+';this.home=function(){'+main.slice(homeStart,homeEnd)+'};this.row=function(){return ('+expression+').replace(/<[^>]*>/g,\'\');};',context);
const rows=[];
for(const road of [1,2,9])for(const locked of road===1?[false]:[false,true])for(const value of [0,.005,42.18,59.995,60.015,126.25,6100.999]){
 stored=String(value);context.r={id:road};context.locked=locked;context.best=Number(stored)||null;context.home();const row=context.row();
 rows.push('{'+[road,locked?1:road,value,JSON.stringify(menu.textContent),JSON.stringify(row)].join(',')+'}');
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(mainPath))!==sha(mb)||sha(fs.readFileSync(hudPath))!==sha(hb))throw Error('Source changed');fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/best_readout_fixture.h','/* Actual browser home-best assignments + road-row template + fmt; synthetic read-only storage. */\nstatic const BestCase best_cases[]={\n'+rows.join(',\n')+'\n};\n');const receipt={mainPath,mainSHA256:sha(mb),hudPath,hudSHA256:sha(hb),cases:rows.length,scope:'Actual home roadUI best assignments, road-row best template and HUD fmt, synthetic read-only storage values. Finite nonnegative records on roads1/2/9, locked/unlocked presentation; no real storage/account/profile writes or unlocks.'};fs.writeFileSync(out+'/best-readouts-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
