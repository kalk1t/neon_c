import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const root='C:/Users/User/Projects/neon_c',out=process.argv[2];fs.mkdirSync(out,{recursive:true});const sha=b=>crypto.createHash('sha256').update(b).digest('hex');const file=root+'/game/src/main.js',bytes=fs.readFileSync(file),source=bytes.toString();
const a=source.indexOf("if (g.state !== 'paused') {",source.indexOf('showcaseCamera(dt);')),b=source.indexOf('sky.update(time, camera.position);',a);if(a<0||b<=a)throw Error('PausedFXhook');const block=source.slice(a,b);if(!block.includes('course.update(time,')||!block.includes('scenery.update(time);')||!block.includes('ads.update(time);'))throw Error('Sourcechanged');
let rows=[],clock=0;const stateNames=['menu','race','paused','countdown','finished','results'];
for(let seq=0;seq<8;seq++){
 clock=0;let courseTime=0,sceneryTime=0,adsTime=0;
 const run=new vm.Script('(function(g,dt,time,playing,course,scenery,ads){'+block+'})').runInNewContext({camera:{position:{}},weapons:{update(){}},coins:{update(){}},player:{}});
 for(let frame=0;frame<160;frame++){
  const phase=frame===0?0:frame%57<28?2:[0,1,3,4,5][(frame+seq)%5],time=10+seq*100+frame*[1/240,1/60,1/30,.2][seq%4];const g={state:stateNames[phase],mode:'race',racers:[],remotes:[]};run(g,.02,time,g.state==='race'||g.state==='finished',{update:t=>{courseTime=t;}},{update:t=>{sceneryTime=t;}},{update:t=>{adsTime=t;}});
  if(courseTime!==sceneryTime||courseTime!==adsTime)throw Error('Clockcoupling');rows.push('{'+[seq,frame,phase,time,sceneryTime].join(',')+'}');
 }
}
fs.writeFileSync(out+'/pause_scene_fixture.h','/* Actual browser main.js paused course/scenery/ads update gate with inert side-effect callbacks. */\nstatic const SceneTimeCase scene_time_cases[]={\n'+rows.join(',\n')+'\n};\n');
if(sha(fs.readFileSync(file))!==sha(bytes))throw Error('Sourcechanged');const receipt={version:JSON.parse(fs.readFileSync(root+'/game/package.json')).version,sources:[{path:file,sha256:sha(bytes)}],block,block_sha256:sha(Buffer.from(block)),states:rows.length,sequences:8,scope:'Actual browser update guard: course/scenery/ads retain last absolute update time while paused and catch up to current time on resume. Native frozen time currently affects distant traffic in rr_scene_world. No actual rendering, shader/sky/car idle effects or browser engine run.'};fs.writeFileSync(out+'/pause-scene-oracle.json',JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify({states:rows.length,sequences:8}));
