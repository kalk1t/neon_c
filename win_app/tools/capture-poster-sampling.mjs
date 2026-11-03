import{readFile,writeFile}from'node:fs/promises';import{createHash}from'node:crypto';import{pathToFileURL}from'node:url';import path from'node:path';
const root=path.resolve(process.argv[2]),out=path.resolve(process.argv[3]),browser=path.resolve(root,'../game'),sha=b=>createHash('sha256').update(b).digest('hex');
const THREE=await import(pathToFileURL(path.join(browser,'node_modules/three/build/three.module.js')).href);
const context=new Proxy({createLinearGradient(){return{addColorStop(){}};}},{get(t,k){return k in t?t[k]:(()=>{});},set(t,k,v){t[k]=v;return true;}});globalThis.document={createElement(){return{width:0,height:0,getContext(){return context;}};}};
const{billboardTexture}=await import(pathToFileURL(path.join(browser,'src/materials.js')).href),texture=billboardTexture();
const values=[];for(const u of[-.01,0,.5/512,.25,.5-1e-8,.5,.5+1e-8,.75,1-.5/512,1,1.01])for(const v of[-.01,0,.5/1024,.25,.5-1e-8,.5,.5+1e-8,.75,1-.5/1024,1,1.01]){
 const transformed=texture.transformUv(new THREE.Vector2(u,v));const x=transformed.x*512-.5,y=transformed.y*1024-.5,ix=Math.floor(x),iy=Math.floor(y),clamp=(n,hi)=>Math.max(0,Math.min(hi,n));
 values.push({uv:[u,v],canvas_uv:transformed.toArray(),texels:[clamp(ix,511),clamp(ix+1,511),clamp(iy,1023),clamp(iy+1,1023)],weights:[x-ix,y-iy]});
}
const names=['src/materials.js','node_modules/three/src/textures/Texture.js','node_modules/three/src/textures/CanvasTexture.js'];const sources=await Promise.all(names.map(async p=>({path:p,sha256:sha(await readFile(path.join(browser,p)))})));
await writeFile(out,JSON.stringify({sources,scope:'Actual source CanvasTexture construction without raster plus Texture.transformUv; bilinear address goldens only. No stored poster pixels available, no decode or appearance claim.',width:512,height:1024,samples:values,canvas_panel_for_uv_quadrant:[2,3,0,1],pixels_verified:false},null,2)+'\n');console.log({samplingCases:values.length,pixelRasterAvailable:false});
