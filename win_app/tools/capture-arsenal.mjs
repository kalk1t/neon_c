// Execute actual playerWeapons with effect sinks; no renderer/network/accounts.
import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {pathToFileURL} from 'node:url';
import path from 'node:path';
import vm from 'node:vm';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game');
const sha=b=>createHash('sha256').update(b).digest('hex');
const names=['package.json','src/main.js','src/ai.js','src/car.js'];
const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
const load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [THREE,{WEAPON},{SPIN}]=await Promise.all([load('node_modules/three/build/three.module.js'),load('src/ai.js'),load('src/car.js')]);
const src=await readFile(path.join(browser,'src/main.js'),'utf8');
const a=src.indexOf('function playerWeapons('),b=src.indexOf('// Debug/demo pilot',a);
if(a<0||b<0)throw Error('playerWeapons hook changed');
const reset=src.match(/Object\.assign\(g, \{ heat: 0, overheated: false, boltCD: 0,[^\n]+\}\);/);
if(!reset)throw Error('startRace reset hook changed');
let order=[],draws=0,random=[],spread=[0,0],spin=0,homing=false,hits={};
const target={car:{pos:new THREE.Vector3(0,0,-100),vel:new THREE.Vector3()}};
const g={side:1,lock:null,lockT:0,aimDir:new THREE.Vector3(0,0,-1),racers:[],remotes:[],mode:'race',shake:0,kick:0};
const car={speed:160,pos:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),root:{quaternion:new THREE.Quaternion()},muzzle:()=>new THREE.Vector3(),dodge(s){spin=s;order.push(5);}};
const math=Object.create(Math);math.random=()=>{if(draws>=random.length)throw Error('RNG overrun');return random[draws++];};
const ctx={THREE,WEAPON,SPIN,g,car,player:{},live:null,_side:new THREE.Vector3(),_d:new THREE.Vector3(),Math:math,
 input:{hit(...keys){return keys.some(k=>hits[k]);}},
 weapons:{fire(p,from,dir,speed,tgt,o){if(o.kind==='bolt'){order.push(1);spread=[dir.x,dir.y];}else{order.push(3);homing=!!tgt;}},flash(){},shockwave(){order.push(6);},purge(){return 0;}},
 sound:{blaster(){},overheat(){order.push(2);},missile(){},deny(){order.push(4);},boostOn(){},emp(){}},hud:{sub(){},flash(){}},leadDir(){return new THREE.Vector3(0,0,-1);},damage(){throw Error('outside trigger-only scope');}};
vm.createContext(ctx);vm.runInContext(src.slice(a,b),ctx);
const keys=['heat','boltCD','missiles','spinCD','empCD','overheated','side'];
const snap=()=>keys.map(k=>g[k]);const put=v=>keys.forEach((k,i)=>g[k]=v[i]);
const initial=[0,0,3,0,0,false,1],cases=[];
function run(before,dt,input,rand,load=true,action=0){if(load)put(before);const actualBefore=snap();g.lock=input[4]?target:null;g.lockT=input[6];hits={Space:input[1],KeyF:input[2],KeyX:input[3]};order=[];draws=0;random=rand;spread=[0,0];spin=0;homing=false;
 if(action) {if(action===2)g.side=1;vm.runInContext(reset[0],ctx);}else ctx.playerWeapons(dt,{fire:input[0],steer:input[5]});
 cases.push({load,action,before:actualBefore,dt,input,random:rand,after:snap(),order,draws,spread,spin,homing});
}
// Exact zero/cooldown, overheat release, recharge and threshold probes.
for(const dt of[0,1e-9,1/144,.05,.09,.5,1,6,11])for(const heat of[0,.25,.250000001,.94,1])for(const over of[false,true])for(let mask=0;mask<8;mask++){
 const i=[!!(mask&1),!!(mask&2),!!(mask&4),true,mask%2===0,[-1,0,1][mask%3],[.999999999,1,1.000000001][mask%3]];
 run([heat,[0,dt,dt+1e-9][mask%3],[0,1-dt/4.5,1,3][mask%4],[0,dt,dt+1e-9][mask%3],[0,dt,dt+1e-9][(mask+1)%3],over,mask%2?-1:1],dt,i,[.1,.9,.5]);
}
// Persistent held/released streams, no catch-up firing on large dt, restarts.
for(let t=0;t<4;t++)for(let f=0;f<900;f++){
 const dt=[1/144,1/60,1/30,.05][t],i=[f%400<300,f%7===0,f%67===0,f%79===0,f%3!==0,f%5===0?0:f%2?-.1:.2,f%11?1:.4];
 run(initial,dt,i,[(f%17)/17,(f%23)/23,(f%2)*.5],f===0,f===450?1:0);
}
for(const side of[-1,1])for(const action of[1,2])run([1,-9,.1,4,8,true,side],.05,[false,false,false,false,false,0,0],[.1,.9,.5],true,action);
for(const s of sources)if(sha(await readFile(path.join(browser,s.path)))!==s.sha256)throw Error('source changed');
await writeFile(out,JSON.stringify({sources,constants:{WEAPON,SPIN},scope:'Actual playerWeapons resource and trigger outputs, with actual startRace resource-reset statement. Car aim/muzzle, effects/projectiles/EMP targets are sinks; no projectile/damage/dodge integration or input edge detection. RNG logs only actual demand.',cases},null,2)+'\n');
console.log({cases:cases.length,ticks:cases.filter(c=>!c.action).length,draws:cases.reduce((n,c)=>n+c.draws,0),events:cases.reduce((n,c)=>n+c.order.length,0)});
