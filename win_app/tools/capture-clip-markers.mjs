import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [source,out]=process.argv.slice(2),b=fs.readFileSync(source),s=b.toString();let a=s.indexOf('  drawMap(g) {'),i=s.indexOf('{',a)+1,n=1;for(;i<s.length&&n;i++){if(s[i]==='{')n++;if(s[i]==='}')n--;}if(a<0||n)throw Error('drawMap hook');const method=s.slice(a,i).trim();
const ctx={Math};vm.createContext(ctx);vm.runInContext('this.draw=function'+method.slice(7)+';',ctx);const rows=[];
for(const radius of [94.4,118,236])for(const offset of [-18,-8,-1,0,1,8,18])for(const angle of [0,Math.PI/8,Math.PI/2,Math.PI+.17])for(const yaw of [0,.3,Math.PI/2]){
 let tx=0,ty=0,rotation=0,points=[],filled=[];const at=(x,y)=>[tx+x*Math.cos(rotation)-y*Math.sin(rotation),ty+x*Math.sin(rotation)+y*Math.cos(rotation)];
 const c={clearRect(){},beginPath(){points=[];},moveTo(x,y){points.push(at(x,y));},lineTo(x,y){points.push(at(x,y));},closePath(){},stroke(){},fillRect(){},arc(){},fill(){filled=points.map(p=>[...p]);},save(){},translate(x,y){tx=x;ty=y;},rotate(a){rotation=a;},restore(){}};
 const distance=radius+offset*radius/118,cx=Math.cos(angle)*distance,cy=Math.sin(angle)*distance;
 const hud={ctx:c,track:{pts:[],checkpoints:[]},map:()=>[220+cx*440/(2*radius),220+cy*440/(2*radius)],mapLabel:'TRACK',names:{race:'TRACK'}};
 const player={isPlayer:true,car:{pos:{x:0,z:0},yaw}};ctx.draw.call(hud,{mode:'race',player,racers:[player],remotes:[]});if(filled.length!==4)throw Error('arrow changed');
 const p=filled.map(([x,y])=>[(x-220)*2*radius/440,(y-220)*2*radius/440]);
 for(const indices of [[0,2,1],[0,3,2]])rows.push({radius,triangle:indices.map(i=>p[i]),marker:true});
}
for(const r of [1,118,236])for(const triangle of [[[0,0],[1,0],[0,1]],[[2,2],[3,2],[2,3]],[[-2,-2],[0,3],[2,-2]],[[1,0],[2,1],[2,-1]]])rows.push({radius:r,triangle:triangle.map(p=>p.map(x=>x*r)),marker:false});
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(source))!==sha(b))throw Error('Source changed');fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/clip-marker-inputs.json',JSON.stringify({source,sha256:sha(b),method_sha256:sha(Buffer.from(method)),rows}));console.log(JSON.stringify({actual_source_arrow_triangles:rows.filter(r=>r.marker).length,analytic_controls:rows.filter(r=>!r.marker).length}));
