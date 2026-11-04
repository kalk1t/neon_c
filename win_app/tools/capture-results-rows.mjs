import fs from'node:fs';import path from'node:path';import vm from'node:vm';import crypto from'node:crypto';
const root=process.argv[2],game=path.resolve(root,'../game'),out=path.join(root,'reference/ui-parity-20261103/results-rows-capture'),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const sources=['src/race.js','src/hud.js','src/style.css','package.json'].map(p=>({path:p,sha256:sha(fs.readFileSync(path.join(game,p)))}));
const race=fs.readFileSync(path.join(game,'src/race.js'),'utf8'),hud=fs.readFileSync(path.join(game,'src/hud.js'),'utf8'),css=fs.readFileSync(path.join(game,'src/style.css'),'utf8');
const a=race.indexOf('export function showResults()'),b=race.indexOf('  const coinPos =',a),f=hud.indexOf('export const fmt ='),z=hud.indexOf('\n};',f);
if(a<0||b<0||f<0||z<0)throw Error('Source boundary changed');
const esc=race.split('\n').find(l=>l.startsWith('const esc =')),ordinal=race.split('\n').find(l=>l.startsWith('export const ordinal ='))?.replace('export const','const');if(!esc||!ordinal)throw Error('Source helpers changed');
for(const s of ['color: #cfefff','color: #9bd','color: #f3e0b0','color: #ffd25a','text-shadow: 0 0 8px #ffa012','#res-standings, #res-coins { line-height: 1.42; }'])if(!css.includes(s))throw Error('Source style changed '+s);
const fn=hud.slice(f,z+3).replace('export const','const')+'\n'+esc+'\n'+ordinal+'\n'+race.slice(a,b).replace('export function','function')+'capture({times:rows.map(v=>({racer:g.racers.indexOf(v.r),seconds:v.t,estimated:v.est})),coins:coinRows.map(r=>({racer:g.racers.indexOf(r),coins:r.coins}))});}\nshowResults();';
const names=['Pilot & Co',"D'Art",'<Fox>','Q"R','R-4','S_5','T.6','U/7','V8','W9','Z10'];const boards=[];
const decode=s=>s.replace(/&#(\d+);/g,(_,n)=>String.fromCodePoint(Number(n)));
for(let seed=0;seed<12;seed++)for(const count of[1,3,11]){
 const nodes={};let model=null;const racers=Array.from({length:count},(_,i)=>({name:seed===11&&i===0?'Q\u2014R':names[i],isPlayer:i===0,takedowns:0,lapTimes:i===0?[30+seed/10,40+seed/10]:[],finished:i===0||(i+seed)%3===0,finishTime:65+((i*17+seed)%23),coins:(i+seed)%5,progress:()=>100+((i*173+seed*19)%4000)}));
 const ctx={g:{state:'finished',racers,finishPos:1,laps:2,raceTime:70+seed/5},player:racers[0],track:{n:4096},Math,capture:v=>model=v,$:id=>nodes[id]??={textContent:'',innerHTML:''}};
 vm.runInNewContext(fn,ctx,{timeout:1000});if(!model)throw Error('No model');
 const parse=html=>[...html.matchAll(/<li class="([^"]*)">([\s\S]*?)<span>([\s\S]*?)<\/span><\/li>/g)].map(m=>({me:m[1]==='me',name:decode(m[2]),value:decode(m[3])}));
 const times=parse(nodes['res-standings'].innerHTML),coins=parse(nodes['res-coins'].innerHTML);if(times.length!==model.times.length||coins.length!==count)throw Error('Unknown markup');
 boards.push({seed,names:racers.map(r=>r.name),model,expected:{times,coins}});
}
for(const s of sources)if(sha(fs.readFileSync(path.join(game,s.path)))!==s.sha256)throw Error('Source changed');
fs.writeFileSync(path.join(out,'reference.json'),JSON.stringify({version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,scope:'Actual showResults guard/display prefix through coin rows, fmt/esc/ordinal helpers and stable JS display sorting in isolated virtual DOM. Scripted actor progress inputs. Stops before coin bonus/banking/account/unlocks/telemetry. No browser/game/graphics execution.',boards})+'\n');
console.log({boards:boards.length,rows:boards.reduce((n,b)=>n+b.model.times.length+b.model.coins.length,0)});
