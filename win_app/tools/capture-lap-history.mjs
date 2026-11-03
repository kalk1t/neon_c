import fs from 'node:fs';import crypto from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';
const [source,assets,out]=process.argv.slice(2),bytes=fs.readFileSync(source),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const {HUD,fmt}=await import(pathToFileURL(path.resolve(source)));
const cases=[];
for(const count of [0,1,2,3,4,10,64,256]){
 const laps=Array.from({length:count},(_,i)=>i===1||i===3?60:70+i*.125),d=Object.create(HUD.prototype);d.el={list:{innerHTML:''},best:{textContent:''}};
 d.laps(laps,count?Math.min(...laps):null);
 const matches=[...d.el.list.innerHTML.matchAll(/<li class="([^"]*)">([^<]*)<\/li>/g)];
 if(matches.length!==count||matches.some((m,i)=>m[2]!==fmt(laps[i])))throw Error('Browser order/count differs');
 if(count>1&&matches.filter(m=>m[1]==='best').length!==1)throw Error('First tied best contract changed');
 cases.push({count,rows:matches.map(m=>({best:m[1]==='best',text:m[2]})),best:d.el.best.textContent});
}
const assetBytes=fs.readFileSync(assets),sample=JSON.parse(assetBytes).find(a=>a.id==='hud/lap-list'&&a.scale===1);
const ol=sample.observed.children.find(c=>c.tag==='OL');if(sample.observed.rect.height!==103||ol.rect.y!==35||ol.rect.height!==36)throw Error('Prepared two-lap browser geometry changed');
if(sha(fs.readFileSync(source))!==sha(bytes))throw Error('HUD changed during capture');
const report={source,sourceSHA256:sha(bytes),assetManifest:assets,assetManifestSHA256:sha(assetBytes),cases,twoLapGeometry:{height:sample.observed.rect.height,children:sample.observed.children.map(c=>({tag:c.tag,rect:c.rect}))},scope:'Actual HUD.laps order/format/first-best with inert DOM text fields; existing prepared browser geometry, not fresh layout rendering. No graphics/network/accounts.'};
fs.writeFileSync(out,JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify({cases:cases.length,maxRows:256,twoLapPanelHeight:103}));
