import fs from 'node:fs';import path from 'node:path';import vm from 'node:vm';import crypto from 'node:crypto';
const root=process.argv[2],game=path.resolve(root,'../game'),out=path.join(root,'reference/ui-parity-20261103/results-summary-capture');fs.mkdirSync(out,{recursive:true});
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),race=fs.readFileSync(path.join(game,'src/race.js'),'utf8');
const ordinal=race.split('\n').find(l=>l.startsWith('export const ordinal ='))?.replace('export const','const');
const place=race.split('\n').find(l=>l.trimStart().startsWith("$('res-place').textContent ="));
if(!ordinal||!place||!place.includes('player.takedowns === 1'))throw Error('Source summary boundary changed');
const program=ordinal+'\n'+place+'\ncapture(ordinal(g.coinPos));',script=new vm.Script(program),cases=[];
for(let count=1;count<=11;count++)for(let finish=1;finish<=count;finish++)for(let coin=1;coin<=count;coin++)for(const takedowns of [0,1,2,11,2147483647]){
 let captured=null;const node={textContent:''};
 script.runInNewContext({g:{racers:Array(count),finishPos:finish,coinPos:coin},multi:count>1,player:{takedowns},$:id=>{if(id!=='res-place')throw Error('Unexpected node');return node;},capture:v=>captured=v,Math},{timeout:1000});
 if(captured===null)throw Error('Missing ordinal');cases.push({count,finish,coin,takedowns,place:node.textContent,coinOrdinal:captured});
}
const ordinalCases=[];
for(const n of [...Array.from({length:131},(_,i)=>i+1),2147483647]){let expected=null;vm.runInNewContext(ordinal+'\ncapture(ordinal(value));',{value:n,capture:v=>expected=v,Math},{timeout:1000});ordinalCases.push({n,expected});}
if(cases.length!==2530||ordinalCases.length!==132)throw Error('Unexpected count');
const sources=['src/race.js','package.json'].map(file=>({path:file,sha256:sha(fs.readFileSync(path.join(game,file)))}));
const data={version:JSON.parse(fs.readFileSync(path.join(game,'package.json'))).version,sources,program,programSHA256:sha(program),cases,ordinalCases,scope:'Actual source res-place assignment and ordinal helper only, isolated VM with local text node/scripted counts. No source game imports, simulation, DOM renderer, account/bonus/bank/progression/network or graphics execution.'};
fs.writeFileSync(path.join(out,'reference.json'),JSON.stringify(data)+'\n');console.log({cases:cases.length,ordinals:ordinalCases.length,sourceOnly:true});
