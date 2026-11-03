// Serve/test only the generated C artifact. Does not launch or alter game/.
import http from 'node:http';
import {readFile,writeFile,mkdir} from 'node:fs/promises';
import path from 'node:path';
import {fileURLToPath,pathToFileURL} from 'node:url';
import assert from 'node:assert/strict';
const root=fileURLToPath(new URL('../',import.meta.url)),binary=path.join(root,'build/web/bin');
const {chromium}=await import(pathToFileURL(path.resolve(root,'../game/node_modules/playwright-core/index.mjs')).href);
const mime={'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.data':'application/octet-stream'};
const server=http.createServer(async(req,res)=>{
  try{
    const relative=decodeURIComponent(new URL(req.url,'http://localhost').pathname).replace(/^\/+/,''),file=path.resolve(binary,relative);
    if(!file.startsWith(binary+path.sep)){res.writeHead(403).end();return;}
    const body=await readFile(file);res.writeHead(200,{'content-type':mime[path.extname(file)]??'application/octet-stream'});res.end(body);
  }catch{res.writeHead(404).end();}
});
await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
let browser;
try{
  browser=await chromium.launch({executablePath:process.env.RR_CHROME??'C:/Program Files/Google/Chrome/Application/chrome.exe',headless:true});
  const page=await browser.newPage({viewport:{width:1440,height:900}}),errors=[],logs=[];
  let rejectRuntime;
  const runtimeFailure=new Promise((_,reject)=>{rejectRuntime=reject;});
  runtimeFailure.catch(()=>{});
  page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>{if(m.type()==='error')errors.push(m.text());else logs.push(m.text());});
  page.on('response',response=>{if(response.status()>=400 && /\.(js|wasm|data)(\?|$)/.test(response.url()))rejectRuntime(Error(`Artifact request failed: ${response.status()} ${response.url()}`));});
  page.on('pageerror',e=>rejectRuntime(e));
  await page.goto(`http://127.0.0.1:${server.address().port}/RoadstersRace.html?smoke`,{waitUntil:'load',timeout:120000});
  await Promise.race([page.waitForFunction(()=>globalThis.Module?.rrSmoke?.frame===151,null,{timeout:240000}),runtimeFailure]);
  const state=await page.evaluate(()=>Module.rrSmoke);
  assert.equal(state.carLoaded,true,'Converted original Roadster must load in WebAssembly');
  assert.equal(state.mode,1,'Smoke run must reach flight mode');assert.ok(state.speed>60 && state.speed<63);
  assert.deepEqual(errors,[],'No browser runtime or shader errors');
  await mkdir(path.join(root,'build/web/captures'),{recursive:true});
  await page.screenshot({path:path.join(root,'build/web/captures/flight.png')});
  await writeFile(path.join(root,'build/web/captures/smoke-result.json'),JSON.stringify({state,errors,logs},null,2)+'\n');
  console.log('C/WebAssembly graphics smoke passed:',JSON.stringify(state));
}finally{
  if(browser)await browser.close();await new Promise(resolve=>server.close(resolve));
}
