import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';
const root=process.argv[2],game=path.resolve(root,'../game'),out=path.join(root,'reference/ui-parity-20261103/results-times-capture');fs.mkdirSync(out,{recursive:true});
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),sources=['src/race.js','src/hud.js','src/style.css','package.json'].map(p=>({path:p,sha256:sha(fs.readFileSync(path.join(game,p)))}));
const race=fs.readFileSync(path.join(game,'src/race.js'),'utf8'),hud=fs.readFileSync(path.join(game,'src/hud.js'),'utf8');
const a=race.indexOf('export function showResults()'),z=race.indexOf('  // rivals still racing',a);if(a<0||z<0)throw Error('Display prefix changed');
const f=hud.indexOf('export const fmt ='),fend=hud.indexOf('\n};',f);if(f<0||fend<0)throw Error('fmt changed');
const code=hud.slice(f,fend+3).replace('export const','const')+'\n'+race.slice(a,z).replace('export function','function')+'}\nshowResults();';
const base=[0,.001,.004,.005,.015,.125,1.005,1.015,1.125,2.675,9.995,59.994999999,59.995,59.999,60,60.005,60.015,119.995,120,3599.995,3600,3600.125,99999.995,1e8,5e11];
for(let i=0;i<60;i++)base.push((i*37.19)+.005);
const inputs=[[],[0],[0,0],[30,40],[40,30],[30,30]];
for(let i=0;i<base.length;i++){const t=base[i];inputs.push([t],[t,0],[0,t],[t,t],[t,base[(i+1)%base.length]]);}
const cases=inputs.map(laps=>{const nodes={};const ctx={g:{state:'finished',racers:[{}],finishPos:1},player:{lapTimes:laps,takedowns:0},Math,ordinal:String,$:id=>nodes[id]??=( {textContent:'',innerHTML:''})};vm.runInNewContext(code,ctx,{timeout:1000});
 const labels=[...nodes['res-laps'].innerHTML.matchAll(/<li class="([^"]*)">([^<]*)<\/li>/g)];if(labels.length!==laps.length)throw Error('Unknown results markup');
 return{laps,total:nodes['res-total'].textContent,labels:labels.map(m=>m[2]),best:labels.findIndex(m=>m[1]==='best')};});
for(const s of sources)if(sha(fs.readFileSync(path.join(game,s.path)))!==s.sha256)throw Error('Source changed');
fs.writeFileSync(path.join(out,'reference.json'),JSON.stringify({version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,scope:'Actual showResults total/place/lap display prefix and actual fmt, isolated virtual DOM strings only; stopped before rivals/banking/services. No browser/game/graphics launch.',cases})+'\n');console.log({cases:cases.length,source_version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version});
