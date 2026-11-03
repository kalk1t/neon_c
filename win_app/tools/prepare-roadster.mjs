// Build-time only: lossless decode of the original embedded WebP into PNG.
// Uses installed Chrome's image decoder; no changes to browser game sources.
import {readFile,writeFile,mkdir} from 'node:fs/promises';
import {createHash} from 'node:crypto';
import {fileURLToPath,pathToFileURL} from 'node:url';
import path from 'node:path';
const root=fileURLToPath(new URL('../',import.meta.url));
const {chromium}=await import(pathToFileURL(path.resolve(root,'../game/node_modules/playwright-core/index.mjs')).href);
const input=await readFile(path.join(root,'reference/source-assets/roadster.glb'));
if(input.readUInt32LE(0)!==0x46546c67 || input.readUInt32LE(4)!==2)throw Error('Expected GLB 2');
const jsonLength=input.readUInt32LE(12),binStart=20+jsonLength;
const gltf=JSON.parse(input.subarray(20,binStart).toString('utf8'));
const originalBin=input.subarray(binStart+8,binStart+8+input.readUInt32LE(binStart));
const pieces=[originalBin]; let length=originalBin.length;
const browser=await chromium.launch({executablePath:process.env.RR_CHROME??'C:/Program Files/Google/Chrome/Application/chrome.exe',headless:true});
const page=await browser.newPage();
const converted=[];
try{
  for(const [index,image] of gltf.images.entries()){
    if(image.mimeType!=='image/webp')continue;
    const view=gltf.bufferViews[image.bufferView];
    const webp=originalBin.subarray(view.byteOffset??0,(view.byteOffset??0)+view.byteLength);
    const encoded=await page.evaluate(async base64=>{
      const image=new Image();image.src='data:image/webp;base64,'+base64;await image.decode();
      const canvas=document.createElement('canvas');canvas.width=image.naturalWidth;canvas.height=image.naturalHeight;
      canvas.getContext('2d').drawImage(image,0,0);return canvas.toDataURL('image/png').split(',')[1];
    },webp.toString('base64'));
    const png=Buffer.from(encoded,'base64'),pad=(4-length%4)%4;
    if(pad){pieces.push(Buffer.alloc(pad));length+=pad;}
    image.bufferView=gltf.bufferViews.length;image.mimeType='image/png';
    gltf.bufferViews.push({buffer:0,byteOffset:length,byteLength:png.length});
    pieces.push(png);length+=png.length;converted.push({index,source_bytes:webp.length,png_bytes:png.length});
  }
}finally{await browser.close();}
for(const texture of gltf.textures){
  const extension=texture.extensions?.EXT_texture_webp;
  if(extension){texture.source=extension.source;delete texture.extensions.EXT_texture_webp;if(!Object.keys(texture.extensions).length)delete texture.extensions;}
}
for(const key of ['extensionsUsed','extensionsRequired']){
  if(gltf[key]){gltf[key]=gltf[key].filter(x=>x!=='EXT_texture_webp');if(!gltf[key].length)delete gltf[key];}
}
gltf.buffers[0].byteLength=length;
const json=Buffer.from(JSON.stringify(gltf));const jsonPad=Buffer.alloc((4-json.length%4)%4,0x20);
const binPad=Buffer.alloc((4-length%4)%4);const binary=Buffer.concat([...pieces,binPad]);
const header=Buffer.alloc(20);header.writeUInt32LE(0x46546c67,0);header.writeUInt32LE(2,4);header.writeUInt32LE(20+json.length+jsonPad.length+8+binary.length,8);header.writeUInt32LE(json.length+jsonPad.length,12);header.writeUInt32LE(0x4e4f534a,16);
const binHeader=Buffer.alloc(8);binHeader.writeUInt32LE(binary.length,0);binHeader.writeUInt32LE(0x004e4942,4);
const output=Buffer.concat([header,json,jsonPad,binHeader,binary]);
await mkdir(path.join(root,'assets/models'),{recursive:true});
await writeFile(path.join(root,'assets/models/roadster.glb'),output);
const meshMask=[];
for(const node of gltf.nodes){if(node.mesh===undefined)continue;for(const primitive of gltf.meshes[node.mesh].primitives){if((primitive.mode??4)===4)meshMask.push(node.name.startsWith('hi_'));}}
await writeFile(path.join(root,'src/presentation/roadster_meshes.h'),`/* Generated from original GLB node order; high/low detail are separate. */\nstatic const bool rr_roadster_hi_meshes[]={${meshMask.map(x=>x?'true':'false').join(',')}};\n`);
await writeFile(path.join(root,'reference/roadster-conversion.json'),JSON.stringify({source_sha256:createHash('sha256').update(input).digest('hex'),output_sha256:createHash('sha256').update(output).digest('hex'),conversion:'embedded WebP decoded to PNG by Chrome canvas; geometry/node transforms unchanged; unused original image bytes retained',converted,hi_mesh_mask:meshMask,output_bytes:output.length},null,2)+'\n');
console.log(`Prepared ${converted.length} PNG textures in Roadster GLB (${output.length} bytes), ${meshMask.filter(Boolean).length}/${meshMask.length} high-detail meshes.`);
