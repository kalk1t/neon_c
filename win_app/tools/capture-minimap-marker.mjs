import fs from 'node:fs';import crypto from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';
const [source,reference,out]=process.argv.slice(2),bytes=fs.readFileSync(source),old=fs.readFileSync(reference),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
function cut(s){const a=s.indexOf('  drawMap(g) {');let i=s.indexOf('{',a)+1,n=1;if(a<0)throw Error('No drawMap');for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}return s.slice(a,i);}
const normalize=s=>s.replace(/\/\/[^\n]*/g,'').replace(/\s+/g,'');if(normalize(cut(bytes.toString()))!==normalize(cut(old.toString())))throw Error('Race minimap changed from pinned source');
const {HUD}=await import(pathToFileURL(path.resolve(source))),rows=[];
for(const [x,z] of [[0,0],[200,100],[-400,600]])for(const yaw of [0,.3,Math.PI/2,Math.PI,-1.2,Math.PI*2+.3])for(const [vx,vy,w,h] of [[0,0,192,192],[100,22,240,240],[12,18,360,200]]){
 let tx=0,ty=0,a=0,points=[],filled=[];
 const transform=(x,y)=>[tx+x*Math.cos(a)-y*Math.sin(a),ty+x*Math.sin(a)+y*Math.cos(a)];
 const ctx={clearRect(){},beginPath(){points=[];},moveTo(x,y){points.push(transform(x,y));},lineTo(x,y){points.push(transform(x,y));},closePath(){},stroke(){},fillRect(){},arc(){},fill(){filled=points.map(p=>[...p]);},save(){},translate(x,y){tx=x;ty=y;},rotate(v){a=v;},restore(){}};
 const hud=Object.create(HUD.prototype);hud.setTrack({bounds:{min:{x:-100,z:-200},max:{x:500,z:400}},pts:[],checkpoints:[]},{race:'TRACK',roam:'CITY'});hud.ctx=ctx;hud.mapLabel='TRACK';
 const player={isPlayer:true,car:{pos:{x,z},yaw}};hud.drawMap({mode:'race',player,racers:[player],remotes:[]});
 if(filled.length!==4)throw Error('Arrow topology changed');const output=filled.map(([x,y])=>[vx+x*w/440,vy+y*h/440]);
 rows.push(`{${[x,z,yaw,vx,vy,w,h].join(',')},{${output.map(p=>'{'+p.join(',')+'}').join(',')}}}`);
}
if(sha(fs.readFileSync(source))!==sha(bytes))throw Error('HUD source changed during capture');fs.mkdirSync(out,{recursive:true});
fs.writeFileSync(path.join(out,'minimap_marker_fixture.h'),'/* Actual HUD.drawMap Canvas command transforms; no rendering. */\ntypedef struct MarkerCase {double x,z,yaw,vx,vy,w,h;double points[4][2];} MarkerCase;\nstatic const MarkerCase marker_cases[]={\n'+rows.join(',\n')+'\n};\n');
const receipt={source,sourceSHA256:sha(bytes),reference,referenceSHA256:sha(old),raceMethodUnchanged:true,cases:rows.length,scope:'Actual HUD.drawMap Canvas path/transform commands with inert context, no graphics; square/non-square viewports, heading wrap and off-map positions. Does not establish circular clipping, raster/glow or native appearance.'};fs.writeFileSync(path.join(out,'minimap-marker-oracle.json'),JSON.stringify(receipt,null,2)+'\n');console.log(JSON.stringify(receipt));
