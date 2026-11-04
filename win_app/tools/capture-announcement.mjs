import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';import {pathToFileURL} from 'node:url';
const root='C:/Users/User/Projects/neon_c',pack='C:/Users/User/Documents/Codex/2026-11-03/task/RoadstersSharedAssets-v3',out=process.argv[2];fs.mkdirSync(out,{recursive:true});const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const hudBytes=fs.readFileSync(root+'/game/src/hud.js'),cssBytes=fs.readFileSync(root+'/game/src/style.css'),css=cssBytes.toString();const tick=hudBytes.toString().match(/if \(\(this\.msgTimer -= dt\) <= 0\) e\.msg\.classList\.remove\('show'\);/)[0];
const {HUD}=await import(pathToFileURL(root+'/game/src/hud.js'));const tickSource=vm.runInNewContext('(function(dt){const e=this.el;'+tick+'})');
const manifestBytes=fs.readFileSync(pack+'/manifest.json');if(sha(manifestBytes)!=='e8eca0c29f6f392ca1a81ef602663de6438847db12306594011bb4677dfd0443')throw Error('Manifest');const manifest=JSON.parse(manifestBytes),checked=[];
for(const file of ['assets.json','source/style.css']){const b=fs.readFileSync(pack+'/'+file);if(sha(b)!==manifest.assets.find(v=>v.file===file).sha256)throw Error(file);checked.push({path:pack+'/'+file,sha256:sha(b)});}
const prior=fs.readFileSync(pack+'/source/style.css').toString(),rules=['#msg {','#msg.show {','#msg.warn {','.hidden {'].map(prefix=>{const start=css.indexOf(prefix),line=css.slice(start,css.indexOf('}',start)+1);if(!prior.includes(line))throw Error(prefix);return line;});if(!rules[0].includes('transition: opacity 0.25s;')||!rules[0].includes('900 84px')||!rules[1].includes('opacity: 1'))throw Error('Stylechanged');
const assets=JSON.parse(fs.readFileSync(pack+'/assets.json'));const measured=['hud/callouts/message-normal','hud/callouts/message-warning'].map(id=>assets.find(a=>a.id===id&&a.scale===1));if(measured.some(v=>v.observed.rect.height!==105))throw Error('Layout');
// Independent Newton solver with safeguarded bracket, versus C's fixed bisection.
function ease(x){if(x<=0)return 0;if(x>=1)return 1;let t=x,lo=0,hi=1;for(let i=0;i<60;i++){const y=3*.25*t*(1-t)**2+3*.25*t*t*(1-t)+t**3;if(y<x)lo=t;else hi=t;const slope=.75-1.5*t+3*t*t,n=t-(y-x)/slope;t=n>lo&&n<hi?n:(lo+hi)/2;}return 3*.1*t*(1-t)**2+3*t*t*(1-t)+t**3;}
class Transition {constructor(){this.visible=false;this.target=0;this.effect=null;this.ready=false;} observe(target,visible,now){if(!this.ready||!this.visible||!visible){this.ready=true;this.visible=visible;this.target=target;this.effect=null;return visible?target:0;}
 const old=this.effect;let value=this.target,progress=1;if(old){progress=Math.min(1,Math.max(0,(now-old.start)/old.duration));value=old.a+(old.b-old.a)*ease(progress);}
 if(target!==this.target){if(value===target)this.effect=null;else {let factor=1,logical=value;if(old&&progress<1&&old.logical===target){factor=Math.min(1,Math.abs(ease(progress)*old.factor+1-old.factor));logical=old.b;}this.effect={a:value,b:target,start:now,duration:.25*factor,factor,logical};}this.target=target;}
 return value;}}
function classes(){const set=new Set();return {add:v=>set.add(v),remove:v=>set.delete(v),toggle(v,on){if(on===undefined)on=!set.has(v);if(on)set.add(v);else set.delete(v);},contains:v=>set.has(v)};}
let rows=[],rng=123456789;const random=()=>{rng=(Math.imul(rng,1664525)+1013904223)>>>0;return rng/2**32;};
for(let scenario=0;scenario<24;scenario++){
 const h=Object.create(HUD.prototype);h.el={hud:{classList:classes()},msg:{classList:classes(),textContent:''}};h.msgTimer=0;h.show(false);const transition=new Transition();let now=0;
 const record=(op,text='',duration=0,warn=false,dt=0,visible=false)=>{
  if(op===0)h.message(text,duration,warn);else if(op===1)tickSource.call(h,dt);else if(op===2)h.show(visible);
  const shown=h.el.msg.classList.contains('show'),rendered=!h.el.hud.classList.contains('hidden'),expected=transition.observe(Number(shown),rendered,now);
  rows.push('{'+[scenario,op,now,dt,JSON.stringify(text),duration,Number(warn),Number(visible),Number(shown),Number(rendered),expected,JSON.stringify(h.el.msg.textContent),Number(h.el.msg.classList.contains('warn'))].join(',')+'}');
 };
 record(3);record(2,'',0,false,0,true);
 const fps=[30,60,144,240][scenario%4];
 for(let frame=0;frame<fps*4;frame++){
  now=frame/fps;
  if(frame===0||scenario<8&&frame%fps===0||scenario>=8&&random()<.15){const text=['3','2','1','GO','FINAL LAP','WRONG WAY','MISSED GATE','FINISH'][Math.floor(random()*8)];record(0,text,[0,.01,.04,.08,.1,.25,.9,1.6,3][Math.floor(random()*9)],random()<.3);}
  if(scenario>=8&&random()<.025)record(2,'',0,false,0,random()<.5);
  record(1,'',0,false,1/fps);record(3);
 }
 now+=10;record(1,'',0,false,10);record(3);
}
const geometry=measured.map((v,i)=>'{'+[1600,1000,1,i,v.observed.rect.y,v.observed.rect.height,v.observed.styles.color==='rgb(255, 255, 255)'?'0xffffffffu':'0xffd0f6ffu'].join(',')+'}');
fs.writeFileSync(out+'/announcement_fixture.h','/* Actual HUD message/show/timer DOM classes, independent CSS spec timing model. Not browser-engine transition measurements. */\nstatic const NoticeCase notice_cases[]={\n'+rows.join(',\n')+'\n};\nstatic const GeometryCase geometry_cases[]={'+geometry.join(',')+'};\n');
if(sha(fs.readFileSync(root+'/game/src/hud.js'))!==sha(hudBytes)||sha(fs.readFileSync(root+'/game/src/style.css'))!==sha(cssBytes))throw Error('Sourcechanged');
const receipt={version:JSON.parse(fs.readFileSync(root+'/game/package.json')).version,sources:[{path:root+'/game/src/hud.js',sha256:sha(hudBytes)},{path:root+'/game/src/style.css',sha256:sha(cssBytes)}],package:pack,manifest_sha256:sha(manifestBytes),verified_files:checked,rules,tick_source:tick,states:rows.length,sequences:24,measured_boxes:2,timing_reference:'https://www.w3.org/TR/css-transitions-1/',scope:'Actual HUD.message/show and timer-removal statement with inert DOM; independent CSS Transitions1 binary opacity model, default ease and interruption shortening. Two verified prepared desktop boxes reused. No fresh Chromium transition, group-opacity pixels, style-event coalescing, native font/GPU or graphics stability measurement.'};fs.writeFileSync(out+'/announcement-oracle.json',JSON.stringify(receipt,null,2));console.log(JSON.stringify({states:rows.length,sequences:24,boxes:2}));
