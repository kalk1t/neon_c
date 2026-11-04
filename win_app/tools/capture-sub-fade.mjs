import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';import {pathToFileURL} from 'node:url';
const root='C:/Users/User/Projects/neon_c',pack='C:/Users/User/Documents/Codex/2026-11-03/task/RoadstersSharedAssets-v3',out=process.argv[2];fs.mkdirSync(out,{recursive:true});const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const hudBytes=fs.readFileSync(root+'/game/src/hud.js'),cssBytes=fs.readFileSync(root+'/game/src/style.css'),css=cssBytes.toString();const tick=hudBytes.toString().match(/if \(\(this\.subTimer -= dt\) <= 0\) e\.sub\.classList\.remove\('show'\);/)[0];
const {HUD}=await import(pathToFileURL(root+'/game/src/hud.js'));const tickSource=vm.runInNewContext('(function(dt){const e=this.el;'+tick+'})');
const manifestBytes=fs.readFileSync(pack+'/manifest.json');if(sha(manifestBytes)!=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443')throw Error('Manifest');const manifest=JSON.parse(manifestBytes),checked=[];
for(const file of ['assets.json','source/style.css']){const b=fs.readFileSync(pack+'/'+file);if(sha(b)!==manifest.assets.find(v=>v.file===file).sha256)throw Error(file);checked.push({path:pack+'/'+file,sha256:sha(b)});}
const prior=fs.readFileSync(pack+'/source/style.css').toString(),rules=['#submsg {','#submsg.show {','.hidden {'].map(prefix=>{const start=css.indexOf(prefix),line=css.slice(start,css.indexOf('}',start)+1);if(!prior.includes(line))throw Error(prefix);return line;});if(!rules[0].includes('transition: opacity 0.25s;')||!rules[0].includes('26px')||css.includes('@starting-style'))throw Error('Stylechanged');
const sample=JSON.parse(fs.readFileSync(pack+'/assets.json')).find(a=>a.id==='hud/callouts/submessage'&&a.scale===1);if(sample.observed.rect.height!==32||sample.observed.rect.y!==340)throw Error('Layout');
// Independent Newton solver with safeguarded bracket, versus C's fixed bisection.
function ease(x){if(x<=0)return 0;if(x>=1)return 1;let t=x,lo=0,hi=1;for(let i=0;i<60;i++){const y=3*.25*t*(1-t)**2+3*.25*t*t*(1-t)+t**3;if(y<x)lo=t;else hi=t;const slope=.75-1.5*t+3*t*t,n=t-(y-x)/slope;t=n>lo&&n<hi?n:(lo+hi)/2;}return 3*.1*t*(1-t)**2+3*t*t*(1-t)+t**3;}
class Transition {constructor(){this.visible=false;this.target=0;this.effect=null;this.ready=false;} observe(target,visible,now){if(!this.ready||!this.visible||!visible){this.ready=true;this.visible=visible;this.target=target;this.effect=null;return visible?target:0;}
 const old=this.effect;let value=this.target,progress=1;if(old){progress=Math.min(1,Math.max(0,(now-old.start)/old.duration));value=old.a+(old.b-old.a)*ease(progress);}
 if(target!==this.target){if(value===target)this.effect=null;else {let factor=1,logical=value;if(old&&progress<1&&old.logical===target){factor=Math.min(1,Math.abs(ease(progress)*old.factor+1-old.factor));logical=old.b;}this.effect={a:value,b:target,start:now,duration:.25*factor,factor,logical};}this.target=target;}
 return value;}}

function classes(){const set=new Set();return {add:v=>set.add(v),remove:v=>set.delete(v),toggle(v,on){if(on===undefined)on=!set.has(v);if(on)set.add(v);else set.delete(v);},contains:v=>set.has(v)};}
const cstr=s=>'"'+[...Buffer.from(s)].map(b=>b>=32&&b<127&&b!==34&&b!==92?String.fromCharCode(b):'\\'+b.toString(8).padStart(3,'0')).join('')+'"';
let rows=[],rng=92751,paintCount=0;const random=()=>{rng=(Math.imul(rng,1664525)+1013904223)>>>0;return rng/2**32;};
for(let scenario=0;scenario<16;scenario++){
 const h=Object.create(HUD.prototype);h.el={hud:{classList:classes()},sub:{classList:classes(),textContent:''}};h.subTimer=0;h.show(false);const transition=new Transition();let now=0;
 const record=(op,text='',duration=0,dt=0,visible=false)=>{
  if(op===0)h.sub(text,duration);else if(op===1)tickSource.call(h,dt);else if(op===2)h.show(visible);
  const shown=h.el.sub.classList.contains('show'),rendered=!h.el.hud.classList.contains('hidden'),paint=op===3,expected=paint?transition.observe(Number(shown),rendered,now):0;
  if(paint)paintCount++;
  rows.push('{'+[scenario,op,now,dt,cstr(text),duration,Number(visible),Number(shown),Number(rendered),Number(paint),expected,cstr(h.el.sub.textContent),h.subTimer,Math.round(expected*255),Math.round(expected*31)].join(',')+'}');
 };
 record(3);record(2,'',0,0,true);record(3);
 const fps=[30,60,144,240][scenario%4];
 for(let frame=0;frame<fps*4;frame++){
  now=frame/fps;
  if(frame===0||scenario<4&&frame%fps===0||scenario>=4&&random()<.16){const text=['CHECKPOINT 3/8 · 00:32.10','★ BEST LAP 01:00.12','LAP 01:10.24','SPIN · 1.5× SPEED','RIVAL <b>literal</b> & text',''][Math.floor(random()*6)];record(0,text,[0,.01,.04,.08,.1,.25,.9,1.4,1.6,2.4][Math.floor(random()*10)]);}
  if(scenario>=4&&random()<.04)record(2,'',0,0,random()<.5);
  record(1,'',0,1/fps);record(3);
 }
 now+=10;record(1,'',0,10);record(3);
}
fs.writeFileSync(out+'/sub_fade_fixture.h','/* Actual HUD.sub/show/timer classes; independent CSS model sampled once per recorded paint after logical mutations. */\nstatic const SubFadeCase sub_cases[]={\n'+rows.join(',\n')+'\n};\n');
if(sha(fs.readFileSync(root+'/game/src/hud.js'))!==sha(hudBytes)||sha(fs.readFileSync(root+'/game/src/style.css'))!==sha(cssBytes))throw Error('Sourcechanged');
const receipt={version:JSON.parse(fs.readFileSync(root+'/game/package.json')).version,sources:[{path:root+'/game/src/hud.js',sha256:sha(hudBytes)},{path:root+'/game/src/style.css',sha256:sha(cssBytes)}],package:pack,manifest_sha256:sha(manifestBytes),verified_files:checked,rules,tick_source:tick,states:rows.length,paint_states:paintCount,sequences:16,reused_submessage_box:sample.observed.rect,scope:'Actual HUD.sub/show and subTimer-removal statement with inert DOM; per-paint coalesced independent CSS Transitions model, not browser-engine timing/pixels. Literal UTF8 text, replacements, expiration/zero duration, hidden/re-rendered HUD at30/60/144/240Hz. Existing fallback art/geometry unchanged; no GPU/network/account or graphics stability measurement.'};fs.writeFileSync(out+'/sub-fade-oracle.json',JSON.stringify(receipt,null,2));console.log(JSON.stringify({states:rows.length,paintStates:paintCount,sequences:16}));
