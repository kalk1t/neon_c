const fs=require('fs'),vm=require('vm'),crypto=require('crypto');
const root='C:/Users/User/Projects/neon_c',out=root+'/win_app/reference/ui-parity-20261103/overlay-keys-capture';
const html=fs.readFileSync(root+'/game/index.html','utf8'),main=fs.readFileSync(root+'/game/src/main.js','utf8'),input=fs.readFileSync(root+'/game/src/input.js','utf8');
function actions(id){const part=html.split(`<div id="${id}"`)[1].split('</div></div>')[0];return [...part.matchAll(/<button\b([^>]*)>/g)].filter(m=>! /\bhidden\b/.test(m[1])).map(m=>m[1].match(/data-act="([^"]+)"/)[1]);}
const pause=actions('pause'),results=actions('results');
if(JSON.stringify(pause)!=='["resume","restart","quit"]'||JSON.stringify(results)!=='["restart","quit"]')throw Error('Source button subset changed');
const begin=main.indexOf("document.querySelectorAll('[data-act]')"),end=main.indexOf('\n);',begin)+3,body=main.slice(begin,end);if(begin<0||end<begin)throw Error('dispatcher missing');
const cases=[];
for(const mode of ['race','time'])for(const act of [...pause,...results]){
 let click,trace=[];const button={dataset:{act},addEventListener(event,cb){if(event!=='click')throw Error(event);click=cb;}};
 const context={document:{querySelectorAll(selector){if(selector!=='[data-act]')throw Error(selector);return [button];}},g:{mode},resume(){trace.push('resume');},startRace(x){trace.push('start:'+x);},toMenu(){trace.push('menu');},road:{id:1},roadsOpen(){return 1;},setRoad(){throw Error('Unexpected progression');}};
 vm.runInNewContext(body,context,{timeout:1000});click();
 const expected=act==='resume'?'resume':act==='restart'?'start:'+mode:'menu';if(trace.length!==1||trace[0]!==expected)throw Error('dispatch mismatch');cases.push({act,mode,trace});
}
if(!input.includes("['Space', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.code)) e.preventDefault()"))throw Error('Space default rule changed');
const hashes={};for(const p of ['package.json','index.html','src/main.js','src/input.js'])hashes[p]=crypto.createHash('sha256').update(fs.readFileSync(root+'/game/'+p)).digest('hex');
fs.writeFileSync(out+'/source-reference.json',JSON.stringify({version:JSON.parse(fs.readFileSync(root+'/game/package.json','utf8')).version,hashes,pause,results,dispatcher:body,cases,scope:'Actual source click dispatcher in inert Node VM; HTML enabled offline button subset. Not a UA keyboard, browser graphics or native input test.'},null,2));
const codes={resume:0,restart:1,quit:2};fs.writeFileSync(root+'/win_app/tests/overlay_keys_reference.h','/* Derived from read-only browser markup and actual dispatcher; see source-reference.json. */\nstatic const int source_pause_actions[]={'+pause.map(x=>codes[x]).join(',')+'};\nstatic const int source_results_actions[]={'+results.map(x=>codes[x]).join(',')+'};\n');console.log('Source dispatcher:10 mode/action checks; enabled button order captured; browser files untouched.');
