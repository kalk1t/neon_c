// Headless source fixtures only: real fit/update methods, DOM string/style stubs, Three math; no browser, GL, auth or storage.
// node tools/capture-cockpit.mjs [cockpit.js] [reference.json] [reference.h]
import fs from 'node:fs';
import crypto from 'node:crypto';
import {pathToFileURL,fileURLToPath} from 'node:url';
const source=process.argv[2]??fileURLToPath(new URL('../../game/src/cockpit.js',import.meta.url));
const s=fs.readFileSync(source,'utf8');const prefix=s.split('export class Cockpit')[0],cls=s.split('export class Cockpit')[1].split('\nfunction el')[0].replace('import.meta.env.BASE_URL','""'); // module URL plumbing only; constructor never invoked
const {Cockpit,LAYOUTS}=new Function(prefix+'class Cockpit'+cls+';return {Cockpit,LAYOUTS};')();
const THREE=await import(new URL('../node_modules/three/build/three.module.js',pathToFileURL(source)));
const seats=[];for(const yaw of [0,Math.PI/2,Math.PI,-1.2,.8,2.7])for(const pitch of [-.45,0,.4]){
 const pos=new THREE.Vector3(3,100,4),fwd=new THREE.Vector3(-Math.sin(yaw)*Math.cos(pitch),Math.sin(pitch),-Math.cos(yaw)*Math.cos(pitch));
 const cam=new THREE.PerspectiveCamera();cam.position.copy(pos).addScaledVector(fwd,.6).add(new THREE.Vector3(0,1.5,0));cam.lookAt(cam.position.clone().addScaledVector(fwd,20));cam.rotateX(-.2);const dir=new THREE.Vector3();cam.getWorldDirection(dir);seats.push({fwd:fwd.toArray(),position:cam.position.toArray(),direction:dir.toArray()});
}
const ids=['roadster','viper','titan'];const sizes=[[960,720],[1280,720],[1920,1080],[1440,960],[844,390],[390,844]];
const fit=[];for(let id=0;id<3;id++)for(const [W,H] of sizes){const d=Object.create(Cockpit.prototype);Object.assign(d,{canvas:{style:{}},layer:{clientWidth:W,clientHeight:H},visible:true});d.fit();fit.push({car:id,W,H,w:parseFloat(d.canvas.style.width),h:parseFloat(d.canvas.style.height),bottom:parseFloat(d.canvas.style.bottom),font:parseFloat(d.canvas.style.fontSize)});}
const states=[];
for(let car=0;car<3;car++)for(let i=0;i<12;i++){
 const input={speed:[0,1.49,1.5,120.51][i%4],boost:[0,.004,.005,.555,.999,1][i%6],lap:i+1,laps:i%3===0?0:i%3===1?Infinity:2,time:[0,.005,1.125,59.999,60.015,119.995][i%6],heat:[0,.6,.60001,.995][i%4],overheated:i%5===0,steer:[-1,-.01,0,.01,.031,1][i%6]};
 const d=Object.create(Cockpit.prototype),node=()=>({style:{},dataset:{},set textContent(v){this.text=String(v);},get textContent(){return this.text??'';}});Object.assign(d,{visible:true,built:true,last:{},speed:node(),boostVal:node(),boostFill:node(),boostPanel:node(),attackPanel:node(),atkIcon:node(),warnIcon:node(),atkVal:node(),lap:node(),time:node(),controls:LAYOUTS[ids[car]].controls.map(c=>({d:node(),deg:c.rotation}))});
 d.update(input);states.push({car,input:{...input,laps:input.laps===Infinity?-1:input.laps},speed:d.speed.textContent,boost:d.boostVal.textContent,time:d.time.textContent,lap:d.lap.textContent,heat:d.atkVal.textContent,charge:d.boostPanel.dataset.charge,hot:d.attackPanel.dataset.state==='hot',angle:parseFloat(d.controls[0].d.style.transform.slice(7))});
}
const jsonOut=process.argv[3]??fileURLToPath(new URL('../reference/ui-parity-20261103/cockpit-source-reference.json',import.meta.url));
const headerOut=process.argv[4]??fileURLToPath(new URL('../tests/cockpit-reference.h',import.meta.url));
fs.writeFileSync(jsonOut,JSON.stringify({source,sha256:crypto.createHash('sha256').update(s).digest('hex'),layouts:LAYOUTS,fit,states,seats},null,2));
let header='static const struct {int car;double W,H,w,h,bottom,font;} fit_cases[]={\n'+fit.map(x=>`{${[x.car,x.W,x.H,x.w,x.h,x.bottom,x.font].join(',')}}`).join(',\n')+'\n};\nstatic const struct {int car;double speed,boost,time,heat,steer;int lap,laps,hot;const char *speed_out,*boost_out,*time_out,*lap_out,*heat_out,*charge;double angle;} cockpit_cases[]={\n'+states.map(x=>`{${[x.car,x.input.speed,x.input.boost,x.input.time,x.input.heat,x.input.steer,x.input.lap,x.input.laps,+x.input.overheated].join(',')},${[x.speed,x.boost,x.time,x.lap,x.heat,x.charge].map(JSON.stringify).join(',')},${x.angle}}`).join(',\n')+'\n};\n';
header+='static const double seat_reference[][9]={\n'+seats.map(x=>'{'+[...x.fwd,...x.position,...x.direction].join(',')+'}').join(',\n')+'\n};\n';
header+='static const double layout_reference[3][33]={\n'+ids.map(id=>{const l=LAYOUTS[id];const c=l.controls;const panels=['speed','boost','attack','race'].map(n=>l.panels[n]);return '{'+[...c[0].box,...(c[1]?.box??[0,0,0,0]),...panels.flatMap(p=>p.box),c[0].rotation,...c[0].origin.split(' ').map(x=>parseFloat(x)/100),...panels.map(p=>p.font??1),c.length,+(id==='viper')].join(',')+'}';}).join(',\n')+'\n};\n';
fs.writeFileSync(headerOut,header);
console.log(JSON.stringify({fit:fit.length,states:states.length,layouts:ids}));
