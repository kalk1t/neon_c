import fs from 'node:fs';import vm from 'node:vm';import crypto from 'node:crypto';
const [source,out]=process.argv.slice(2),buf=fs.readFileSync(source),s=buf.toString();let start=s.indexOf('export class Input');if(start<0)throw Error('Input class');
const listeners=new Map(),docListeners=new Map(),clock={now:0},document={pointerLockElement:null,addEventListener(n,fn){docListeners.set(n,fn);}};
const context={document,performance:{now:()=>clock.now},addEventListener:(n,fn)=>listeners.set(n,fn),Math,navigator:{},window:{}};vm.createContext(context);vm.runInContext(s.replace('export class Input','class Input')+';this.instance=new Input();',context);
const input=context.instance,rows=[];
for(const changed of [0,1000,10000000])for(const locked of [false,true]){
 clock.now=changed;document.pointerLockElement=locked?{}:null;docListeners.get('pointerlockchange')();
 for(const age of [-.001,0,1,149.999999,150,150.000001,166.667,1000]){
  clock.now=changed+age;input.takeMouse();listeners.get('mousemove')({movementX:1,movementY:0});const allowed=input.takeMouse().dx===1;
  rows.push('{'+[+(!rows.length||age===-.001),+locked,changed,clock.now,+allowed].join(',')+'}');
 }
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');if(sha(fs.readFileSync(source))!==sha(buf))throw Error('Source changed');fs.mkdirSync(out,{recursive:true});fs.writeFileSync(out+'/mouse_capture_fixture.h','/* Actual Input constructor pointerlockchange/mousemove listeners at capture boundaries; small eligible events avoid the separate spike filter. */\nstatic const CaptureCase capture_cases[]={\n'+rows.join(',\n')+'\n};\n');const receipt={source,sha256:sha(buf),states:rows.length,scope:'Actual browser Input pointerlockchange and mousemove listeners, unlocked/locked, negative/zero/strict150ms boundary/long times. Small events isolate eligibility from event-spike filtering. Native applies eligibility to frame totals, not event timestamps; no real cursor capture or graphics.'};fs.writeFileSync(out+'/mouse-capture-oracle.json',JSON.stringify(receipt,null,2));console.log(receipt);
