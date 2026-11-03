import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath,pathToFileURL} from 'node:url';
import path from 'node:path';
import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url)),browser=path.resolve(process.argv[2]??path.join(root,'../game'));
const files=['package.json','src/physics.js','src/car.js','src/course.js','src/props.js','src/roads.js','src/track.js'];
const hashes=()=>Promise.all(files.map(async p=>({path:p,sha256:createHash('sha256').update(await readFile(path.join(browser,p))).digest('hex')})));
const before=await hashes(),load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [{OBB,Cyl,Torus,Colliders},{Car},THREE,{Track},{ROADS},{RING}]=await Promise.all([load('src/physics.js'),load('src/car.js'),load('node_modules/three/build/three.module.js'),load('src/track.js'),load('src/roads.js'),load('src/props.js')]);
const v=a=>new THREE.Vector3(...a),vector=a=>`{${a.x},${a.y},${a.z}}`,f=x=>Number.isFinite(x)?String(x):'0';
const specs=[
 [0,[0,0,0],[4,6,8],0,0,0,0,0],[0,[-161,118,121],[6,12,2],.73,0,0,0,0],
 [0,[120,115,-120],[4,4,4],-1.2,0,0,0,0],
 [1,[0,0,0],[0,0,0],0,7,0,-5,12],[1,[-120,0,120],[0,0,0],0,4,0,100,130],
 [2,[0,118,0],[0,0,0],0,15.5,2.45,0,0],[2,[161,126,-161],[0,0,0],.7,15.5,2.45,0,0],
 [0,[0,118,-20],[20,20,2],0,0,0,0,0],
];
function shape(s){const[k,c,h,yaw,r,t,y0,y1]=s;return k===0?new OBB(...c,...h,yaw):k===1?new Cyl(c[0],c[2],r,y0,y1):new Torus(v(c),yaw,r,t);}
const literal=s=>`{${s[0]},${vector(v(s[1]))},${vector(v(s[2]))},${s.slice(3).join(',')}}`;
let header='/* Actual browser physics and car fixtures; no renderer/network/storage. */\nstatic const ShapeSpec physics_specs[]={\n'+specs.map(literal).join(',\n')+'};\nstatic const ShapeCase shape_cases[]={\n',shapeCases=0,degenerates=0;
for(let id=0;id<specs.length;id++){
 const s=shape(specs[id]),c=v(specs[id][1]);let points=[];
 for(let i=0;i<160;i++){const a=i*.173;points.push(c.clone().add(new THREE.Vector3(Math.cos(a)*(i%29),Math.sin(a*.7)*(i%23),Math.sin(a)*(i%21))));}
 for(const d of [0,1e-7,1e-6,.001,2.1-1e-9,2.1,2.1+1e-9])points.push(c.clone().add(v([specs[id][2][0]+d,0,0])));
 if(specs[id][0]===2){points.push(c.clone().add(v([specs[id][4],0,0])));for(const z of [-1,1,5])points.push(c.clone().add(v([Math.sin(specs[id][3])*z,0,Math.cos(specs[id][3])*z])));}
 for(const p of points){const out={n:new THREE.Vector3(),depth:0},hit=s.resolve(p,2.1,out);const finite=out.n.toArray().every(Number.isFinite);if(hit&&!finite)degenerates++;
  header+=`{${id},${vector(p)},2.1,${hit},${vector(v(out.n.toArray().map(x=>Number.isFinite(x)?x:0)))},${f(out.depth)},${!finite}},\n`;shapeCases++;
 }
}
header+='};\nstatic const WorldCase world_cases[]={\n';let worldCases=0;
const worlds=[[0,3],[1,4],[5,6],[7,0,3]];
function world(id){const w=new Colliders();for(const index of worlds[id])w.add(shape(specs[index]));return w;}
for(let id=0;id<worlds.length;id++)for(let i=0;i<100;i++){
 const p=id===1?v([-120+(i%15)-7,110+(i%21),120+(i%13)-6]):v([(i%19)-9,id===2?118+(i%13)-6:id===3?118+(i%7):i%17-8,(i%17)-8]);
 const q=p.clone(),hit=world(id).collide(q,2.1);if(!q.toArray().every(Number.isFinite))throw Error('Nonfinite world fixture');
 header+=`{${id},${vector(p)},${vector(q)},2.1,${!!hit},${hit?vector(hit.n):'{0,0,0}'},${hit?hit.depth:0}},\n`;worldCases++;
}
header+='};\nstatic const SegmentCase segment_cases[]={\n';let segments=0;
for(let id=0;id<worlds.length;id++)for(let i=0;i<30;i++){
 const a=id===1?v([-150,100+i,120]):v([-30,id===2?118+i*.2:id===3?118:i*.5,-20]),b=a.clone().add(v([60,0,40]));
 const hit=world(id).segment(a,b,.6);header+=`{${id},${vector(a)},${vector(b)},${!!hit},${hit?vector(hit):'{0,0,0}'}},\n`;segments++;
}
header+='};\nstatic const CollisionFlightCase collision_flight_cases[]={\n';let flightSteps=0;
for(let scenario=0;scenario<4;scenario++){
 const c=Object.create(Car.prototype);Object.assign(c,{pos:new THREE.Vector3(),vel:new THREE.Vector3(),fwd:new THREE.Vector3(0,0,-1),t:0,boost:1,boosting:false,maxMul:1,nitro:0,thrust:0,kick:0,sputter:0,model:{position:{},rotation:{}},pods:[],fans:[],ribbons:[],sync(){}});
 c.place(v([0,118,0]),0,60);const w=new Colliders();
 if(scenario===0)w.add(new OBB(0,118,-20,20,20,2));
 if(scenario===1)w.add(new Cyl(0,-20,6,100,140));
 if(scenario===2)w.add(new Torus(v([0,118,-20]),0,15.5,2.45));
 for(let i=0;i<180;i++){
 const dt=[1/30,1/60,1/144,.05][scenario],inp={throttle:1,brake:i>120?.7:0,steer:scenario===2?.5:scenario===3?-.4:0,climb:scenario===2?.3:0,boost:i<30};
 const result=c.update(dt,inp,w);const expected=[...c.pos.toArray(),...c.vel.toArray(),c.yaw,c.pitch,c.roll,c.speed,c.boost,c.rollT,c.surge,c.thrust,c.nitro,c.stun,result.impact];
 if(!expected.every(Number.isFinite))throw Error('Nonfinite car');header+=`{${scenario},${dt},{${inp.throttle},${inp.brake},${inp.steer},${inp.climb},${inp.boost}},{${expected.join(',')}}},\n`;flightSteps++;
 }
}
header+='};\nstatic const CourseShapeCase course_shape_cases[]={\n';let courseShapes=0;
const course=await readFile(path.join(browser,'src/course.js'),'utf8');
const a=course.indexOf('colliders.add(new OBB(0, DECK_Y - 4'),b=course.indexOf('// ---------- one road',a);if(a<0||b<0)throw Error('Start source changed');
const ringLines=course.split('\n').filter(l=>/roadCol\(new (Torus|Cyl)/.test(l)&&(/c\.pos|base/.test(l))).slice(0,3).join('\n');if(!ringLines.includes('Torus')||!ringLines.includes('480'))throw Error('Ring source changed');
function fromShape(s){return s instanceof OBB?[0,s.c.toArray(),s.h.toArray(),Math.atan2(s.sin,s.cos),0,0,0,0]:s instanceof Cyl?[1,[s.x,0,s.z],[0,0,0],0,s.r,0,s.y0,s.y1]:[2,s.c.toArray(),[0,0,0],Math.atan2(s.sin,s.cos),s.R,s.r,0,0];}
for(const road of ROADS){const track=new Track(road),list=[];const col={add:c=>{list.push(c);return c;}};
 const context={OBB,Cyl,Torus,RING,DECK_Y:111,colliders:col,raceCol:c=>col.add(c),roadCol:c=>col.add(c),track};vm.createContext(context);
 vm.runInContext(course.slice(a,b)+`\nfor(const c of track.checkpoints){const base=c.pos.y-RING.H;${ringLines}}`,context);
 for(let i=0;i<list.length;i++){header+=`{${road.id},${i},${literal(fromShape(list[i]))}},\n`;courseShapes++;}
}
header+='};\n';const after=await hashes();if(JSON.stringify(before)!==JSON.stringify(after))throw Error('Source changed');
await writeFile(path.join(root,'tests/generated_physics_reference.h'),header);
await writeFile(path.join(root,'reference/collision.json'),JSON.stringify({captured_utc:new Date().toISOString(),sources:before,content_hashes_verified_twice:true,shape_cases:shapeCases,nonfinite_browser_cases:degenerates,world_cases:worldCases,segment_cases:segments,flight_steps:flightSteps,course_shapes:courseShapes,scope:'Actual browser shape resolution, cell-list order, segment sampling, Car.update collision response and extracted start/ring collider construction. Native prototype city layout remains distinct; no rendering, accounts or progress.'},null,2)+'\n');
console.log({shapeCases,degenerates,worldCases,segments,flightSteps,courseShapes});
