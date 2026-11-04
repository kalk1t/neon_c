import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';import {pathToFileURL} from 'node:url';
const root='C:/Users/User/Projects/neon_c',out=process.argv[2];fs.mkdirSync(out,{recursive:true});const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const file=root+'/game/src/camera.js',bytes=fs.readFileSync(file),source=bytes.toString(),start=source.indexOf('export const show =');if(start<0)throw Error('Showcasehook');const fragment=source.slice(start);if(!fragment.includes("g.state !== 'paused'")||!fragment.includes('Math.sign(e.deltaY)'))throw Error('Sourcechanged');
const THREE=await import(pathToFileURL(root+'/game/node_modules/three/build/three.module.js'));
let rows=[],sequences=12;
for(let seq=0;seq<sequences;seq++){
 const handlers={},nodes=new Map();function $(id){if(!nodes.has(id))nodes.set(id,{addEventListener:(name,fn)=>{handlers[id+':'+name]=fn;}});return nodes.get(id);}
 const camera={position:new THREE.Vector3(20+seq,120-seq,-30),fov:68+seq,target:null,lookAt(v){this.target=v.clone();},updateProjectionMatrix(){}};
 const car={pos:new THREE.Vector3(seq*73-400,115+seq*.7,seq*35-200),yaw:seq*.57-3};const g={state:'paused'},lens={uniforms:{uSpeed:{value:1},uLines:{value:1},uCA:{value:1}}};
 const context={THREE,Math,g,car,camera,lens,D:{touch:false},$,addEventListener:(name,fn)=>{handlers[name]=fn;}};const api=new vm.Script(fragment.replaceAll('export ','')+'\n;({show,showcaseCamera});').runInNewContext(context);
 for(let frame=0;frame<240;frame++){
  const dt=[0,1/240,1/144,1/60,1/30,.05,.2][(frame+seq)%7];let event=0,x=300+Math.sin(frame*.31+seq)*700,y=240+Math.cos(frame*.17)*950,delta=0;
  if(frame%61===0)event=1;else if(frame%61===1)event=5;else if(frame%61<25)event=2;else if(frame%61===25)event=3;else if(frame%61<46){event=4;delta=(seq%2===0?-1:1)*(frame%3===0?10000:120);}else if(frame%61===46){event=4;delta=0;}
  if(event===1||event===5)handlers['pause:pointerdown']({clientX:x,clientY:y,target:{closest:()=>event===5?{}:null}});
  else if(event===2)handlers.pointermove({clientX:x,clientY:y});else if(event===3)handlers.pointerup();else if(event===4)handlers['pause:wheel']({deltaY:delta});
  api.showcaseCamera(dt);const s=api.show;
  const values=[seq,frame,dt,event,x,y,delta,car.pos.x,car.pos.y,car.pos.z,car.yaw,s.yaw,s.pitch,s.dist,Number(!!s.drag),camera.position.x,camera.position.y,camera.position.z,camera.target.x,camera.target.y,camera.target.z,camera.fov];
  if(values.some(v=>!Number.isFinite(v)))throw Error('Nonfinite');rows.push('{'+values.join(',')+'}');if(lens.uniforms.uSpeed.value!==0||lens.uniforms.uLines.value!==0||lens.uniforms.uCA.value!==.0015)throw Error('Lens');
 }
}
fs.writeFileSync(out+'/showcase_fixture.h','/* Actual browser showcaseCamera and registered pointer/wheel callbacks with Three Vector3; no renderer. */\nstatic const ShowcaseCase showcase_cases[]={\n'+rows.join(',\n')+'\n};\n');
if(sha(fs.readFileSync(file))!==sha(bytes))throw Error('Sourcechanged');const receipt={version:JSON.parse(fs.readFileSync(root+'/game/package.json')).version,sources:[{path:file,sha256:sha(bytes)}],fragment_sha256:sha(Buffer.from(fragment)),states:rows.length,sequences,scope:'Actual source paused showcase and callback execution with real Three Vector3 in inert event harness. No browser engine, DOM hit testing, graphics, input delivery, car idle animation or shader/lens effects. Native focus/minimize cancellation is a separate host safeguard.'};fs.writeFileSync(out+'/showcase-oracle.json',JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify({states:rows.length,sequences,source:receipt.version}));
