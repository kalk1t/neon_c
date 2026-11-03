// Headless source fixture capture. No browser, rendering or account code executes.
// node tools/capture-ui-time.mjs [hud.js] [output.json] [output.h]
import fs from 'node:fs';
import {fileURLToPath} from 'node:url';
const source=process.argv[2]??fileURLToPath(new URL('../../game/src/hud.js',import.meta.url));
const jsonOut=process.argv[3]??fileURLToPath(new URL('../reference/ui-parity-20261103/ui-time-reference.json',import.meta.url));
const headerOut=process.argv[4]??fileURLToPath(new URL('../tests/ui-time-reference.h',import.meta.url));
const text=fs.readFileSync(source,'utf8');
const body=text.split('export const fmt = (t) => {')[1]?.split('\n};')[0];
if(!body)throw new Error('Browser HUD fmt declaration changed; inspect before refreshing fixture.');
const fmt=new Function('t',body);
const values=[0,.001,.004,.005,.015,.125,1.125,1.005,1.015,1.999,59.994,59.995,59.999,60,60.005,60.015,61.125,119.995,3599.999,3600,5999.999,1e8];
let seed=6103;for(let i=0;i<120;i++){seed=(Math.imul(seed,1664525)+1013904223)>>>0;values.push(seed/4294967296*10000);}
const cases=values.map(t=>({t,expected:fmt(t)}));
fs.writeFileSync(jsonOut,JSON.stringify({source,scope:'Current browser HUD fmt, positive session-time domain',cases},null,2));
fs.writeFileSync(headerOut,'static const struct {double t;const char *expected;} time_cases[]={\n'+cases.map(c=>`{${c.t},${JSON.stringify(c.expected)}}`).join(',\n')+'\n};\n');
console.log(JSON.stringify({cases:cases.length,source,jsonOut,headerOut}));
