import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';import path from 'node:path';import {pathToFileURL} from 'node:url';
const [mainPath,cameraPath,threePath,out]=process.argv.slice(2),mainBytes=fs.readFileSync(mainPath),cameraBytes=fs.readFileSync(cameraPath),main=mainBytes.toString(),camera=cameraBytes.toString(),THREE=await import(pathToFileURL(path.resolve(threePath)));
const start=main.indexOf("  if (input.hit('KeyC')) {"),end=main.indexOf("  if (input.hit('Escape', 'KeyP'))",start);if(start<0||end<0)throw Error('view dispatch boundary');
const controls=main.slice(start,end),cams=camera.match(/export const CAMS = [^;]+;/)[0].replace('export ',''),poseStart=camera.indexOf('  const sf = Math.min(1.4'),poseEnd=camera.indexOf('  if (cockpit) {',poseStart);if(poseStart<0||poseEnd<0)throw Error('visibility boundary');
let writes=0,dashVisible=false;const ctx={THREE,Math,CAR:{MAX:125},g:{},cam:{},car:{},player:{alive:true},input:{hit(k){return k==='KeyC'?ctx.c:ctx.i;}},setInterior(v){ctx.g.interior=v;writes++;},dash:{show(v){dashVisible=v;}},dt:.016};vm.createContext(ctx);vm.runInContext(cams+';this.controls=function(){'+controls+'};this.visible=function(){'+camera.slice(poseStart,poseEnd)+';return [cockpit,dashValue];};'.replace('dashValue','g.interior && inside'),ctx);
const phases=[['menu',0],['race',1],['paused',2],['countdown',3],['finished',4],['finished',5]],rows=[];
for(const [state,phase]of phases)for(const mode of [0,1,2])for(const interior of [false,true])for(const c of [false,true])for(const i of [false,true])for(const enabled of [false,true]){
 ctx.g={state,camMode:mode,interior};ctx.c=c;ctx.i=i&&enabled;writes=0;ctx.controls();
 rows.push('{'+[phase,mode,+interior,+c,+i,+enabled,ctx.g.camMode,+ctx.g.interior,writes].join(',')+'}');
}
const visibility=[];
for(const [state,phase]of phases)for(const mode of [0,1,2])for(const interior of [false,true])for(const look of [0,.0124999,.0125,.0125001,.2]){
 ctx.g={state,camMode:mode,interior};ctx.cam={yaw:.7,pitch:.2,lookYaw:look,lookPitch:0,looking:true};ctx.car={speed:80,yaw:1.1,pitch:.1,model:{visible:true}};ctx.player.alive=true;dashVisible=false;
 const [raw,cockpit]=ctx.visible(),flight=state==='race'||state==='countdown'||state==='finished';
 // Menus/paused call showcaseCamera instead; readiness is a native safety gate.
 visibility.push('{'+[phase,mode,+interior,look,+(flight&&mode===2&&raw&&!interior),+(flight&&cockpit)].join(',')+'}');
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');for(const [p,b]of [[mainPath,mainBytes],[cameraPath,cameraBytes]])if(sha(fs.readFileSync(p))!==sha(b))throw Error('Source changed');
fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/view_control_fixture.h','/* Actual main C/I dispatch and chaseCamera visibility prefix. */\nstatic const ViewCase view_cases[]={\n'+rows.join(',\n')+'\n};\nstatic const VisibleCase visible_cases[]={\n'+visibility.join(',\n')+'\n};\n');
const receipt={sources:[{path:mainPath,sha256:sha(mainBytes)},{path:cameraPath,sha256:sha(cameraBytes)}],dispatch_cases:rows.length,visibility_cases:visibility.length,scope:'Actual main C then I branch and chaseCamera visibility prefix, real Three vector math, synthetic state/input. Native results maps browser finished. No DOM, localStorage, graphics, physical input or native cockpit activation; setInterior is a counted in-memory stub.'};fs.writeFileSync(out+'/view-controls-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
