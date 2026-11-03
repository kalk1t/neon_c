// Actual browser picker/lock decisions in an in-memory DOM/storage adapter.
// No browser application startup, real localStorage, network or graphics.
import {readFile,writeFile} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath,pathToFileURL} from 'node:url';
import path from 'node:path';
import vm from 'node:vm';
const root=fileURLToPath(new URL('../',import.meta.url));
const browser=process.argv[2]?path.resolve(process.argv[2]):path.resolve(root,'../game');
const inputs=['package.json','src/main.js','src/roads.js','src/track.js','src/hud.js'];
const hashes=()=>Promise.all(inputs.map(async p=>({path:p,sha256:createHash('sha256').update(await readFile(path.join(browser,p))).digest('hex')})));
const before=await hashes(),main=await readFile(path.join(browser,'src/main.js'),'utf8');
const load=p=>import(pathToFileURL(path.join(browser,p)).href);
const [{ROADS},{Track},{HUD}]=await Promise.all([load('src/roads.js'),load('src/track.js'),load('src/hud.js')]);
const tracks=ROADS.map(r=>new Track(r));
function section(start,end){const a=main.indexOf(start),b=main.indexOf(end,a);if(a<0||b<0)throw Error('Picker source structure changed');return main.slice(a,b);}
const cases=[
 [null,0,1,0,1,1],['1',0,1,0,2,1],['3',0,1,0,3,1],['1',4,1,0,5,1],
 ['7',2,1,1,8,1],['2.5',0,1,0,3,1],[null,0,8,0,1,1],['0',0,1,1,1,1],
 ['junk',0,1,0,2,1],['99',0,1,0,9,1],['-3',0,1,0,1,1],['Infinity',0,1,1,9,1],
 ['-Infinity',0,1,0,2,1],['1',8,1,1,9,1],['1',0,1,0,1,2],['2',0,1,0,1,1],
 ['9',0,9,0,9,1],['1',2,3,1,3,1],['1',2,3,0,4,1],['',0,1,0,1,1],
];
const fixtures=[];
for(const [local,won,current,mode,pick,position] of cases){
  const storage=new Map();if(local!==null)storage.set('neon.roadsOpen',local);
  let focused=0,started=null,visible=false;
  const list={children:[],replaceChildren(...children){this.children=children;},querySelector(){return this.children.find(b=>b.className.includes('cur')&&!b.className.includes('locked'));},get firstChild(){return this.children[0];}};
  const elements={'roads-title':{},'road-list':list,roads:{classList:{remove(){visible=true;},add(){visible=false;}}}};
  const context={ROADS,tracks,account:{profile:{roadsWon:won}},road:ROADS[current-1],g:{mode:mode?'time':'race',finishPos:position},
    localStorage:{getItem:key=>storage.get(key)??null,setItem:(key,value)=>storage.set(key,value)},
    $:id=>elements[id],sound:{init(){}},fmt:String,bestKey:id=>id===1?'neon.best':`neon.best.r${id}`,trackEvent(){},
    document:{createElement(){const b={listeners:{},animate(){},focus(){focused=list.children.indexOf(b)+1;},addEventListener(name,fn){b.listeners[name]=fn;}};return b;}}};
  context.setRoad=id=>{context.road=ROADS[id-1];};context.startRace=m=>{started=m;};
  vm.createContext(context);
  vm.runInContext(`${section("const ROAD_OPEN = 'neon.roadsOpen';","$('roads-close').addEventListener")}\n${section('function unlockNext() {',"document.querySelectorAll('[data-act]')")}\nthis.openPicker=openRoads;this.openCount=roadsOpen;this.unlock=unlockNext;`,context);
  const open=context.openCount();context.openPicker(mode?'time':'race');const focus=focused;
  list.children[pick-1].listeners.click();
  const selected=context.road.id,accepted=started!==null,closed=!visible;
  const candidate=context.unlock()?.id??0;
  const number=Number(local);const f=x=>Number.isNaN(x)?'NAN':x===Infinity?'INFINITY':x===-Infinity?'-INFINITY':x.toPrecision(17);
  fixtures.push(`{${f(number)},${won},${current},${mode},${pick},${Math.floor(open)},${focus},${accepted?1:0},${selected},${closed?1:0},${position},${candidate}}`);
}
const mapFixtures=[];
for(const track of tracks){
  const hud=Object.create(HUD.prototype);hud.setTrack(track);
  for(const index of [0,Math.floor(track.n/4),Math.floor(track.n/2),track.n-1]){
    const pos=track.pts[index],point=hud.map(pos.x,pos.z);
    mapFixtures.push(`{${track.road.id},${index},${(point[0]/440).toPrecision(17)},${(point[1]/440).toPrecision(17)}}`);
  }
}
const after=await hashes();if(JSON.stringify(before)!==JSON.stringify(after))throw Error('Browser inputs changed; existing fixtures preserved.');
await writeFile(path.join(root,'tests/generated_road_picker_reference.h'),`/* Actual browser picker decisions and HUD.setTrack; fake DOM/storage only. */\nstatic const PickerFixture picker_fixtures[]={\n${fixtures.join(',\n')}\n};\nstatic const PickerMapFixture picker_map_fixtures[]={\n${mapFixtures.join(',\n')}\n};\n`);
await writeFile(path.join(root,'reference/road-picker.json'),JSON.stringify({captured_utc:new Date().toISOString(),sources:after,content_hashes_verified_twice:true,cases:cases.length,minimap_cases:mapFixtures.length,scope:'Actual browser open-count clamp, current/first focus, locked row refusal, selected mode/road, unlock eligibility and normalized HUD minimap projection. Storage is a new in-memory Map per case, not user/browser data. No graphics, auth or persistence integration.'},null,2)+'\n');
console.log(`${fixtures.length} actual browser picker/progression decision cases; fake DOM/storage only. No user-data writes or graphics.`);
