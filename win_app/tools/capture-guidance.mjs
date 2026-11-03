import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [src,out]=process.argv.slice(2),buf=fs.readFileSync(src),s=buf.toString();
const a=s.indexOf('    const behind = v.z > 1;'),b=s.indexOf('    this.drawMap(g);',a);if(a<0||b<0)throw Error('guidance hook');
const fn=new vm.Script('(function(v,W,H,dist,e){'+s.slice(a,b)+'})').runInNewContext({Math});
const rows=[];
for(const [w,h,z] of [[960,720,.8],[1920,1080,1],[640,480,.8],[390,844,.55]])
for(const x of [-2,-.920000001,-.92,-.919999999,0,.919999999,.92,.920000001,2])
for(const y of [-1,-.900000001,-.9,-.899999999,0,.899999999,.9,.900000001,1])
for(const nz of [-2,1,1.000000001,2]){
 const dist=[0,.49,.5,100.5,9999.999][rows.length%5];const e={target:{style:{}},arrow:{style:{}},tdist:{}};fn({x,y,z:nz},w/z,h/z,dist,e);const on=e.target.style.display==='block';
 const elem=on?e.target:e.arrow;const angle=on?0:Number(elem.style.transform.slice(7,-4));
 rows.push([x,y,nz,w,h,z,dist,+on,parseFloat(elem.style.left)*z,parseFloat(elem.style.top)*z,angle,Math.round(dist)]);
}
fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/target_guidance_fixture.h','/* Actual HUD.update target-marker branch, CSS coordinates converted to physical pixels. */\nstatic const TargetCase target_cases[]={\n'+rows.map(r=>'{'+r.join(',')+'}').join(',\n')+'\n};\n');
const receipt={source:src,sha256:crypto.createHash('sha256').update(buf).digest('hex'),cases:rows.length,scope:'Actual HUD target visibility/placement/rotation/distance branch. NDC inputs supplied, not full world projection or GPU pixels; four viewports/zooms, strict boundary cases, behind and beyond-far semantics.'};fs.writeFileSync(out+'/target-guidance-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
