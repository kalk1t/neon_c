const fs=require('fs'),vm=require('vm'),crypto=require('crypto'),path=require('path');
const sourcePath='C:\\Users\\User\\Projects\\neon_c\\game\\src\\race.js';
const source=fs.readFileSync(sourcePath,'utf8');
const ordinal=source.match(/^export const ordinal = (.*);$/m)?.[1];
const bonus=source.match(/^  const bonus = (.*);$/m)?.[1];
const caption=source.match(/^  \$\('res-bank'\)\.textContent = (.*);$/m)?.[1];
if(!ordinal||!bonus||!caption)throw Error('Current source recipe extraction failed');
const cases=[];
for(let count=1;count<=11;count++)for(let rank=1;rank<=count;rank++){
 const text=vm.runInNewContext(`const ordinal=${ordinal};const multi=count>1,coinPos=rank;const bonus=${bonus};${caption}`,{count,rank},{timeout:1000});
 cases.push({count,rank,text});
}
const report={source:sourcePath,source_sha256:crypto.createHash('sha256').update(source).digest('hex'),ordinal_expression:ordinal,bonus_expression:bonus,caption_expression:caption,cases,scope:'Only extracted pure expressions evaluated in a timeout-limited VM. No game module import, WebGL/browser, account/progression operation, service/network, deployment or upstream writes.'};
fs.writeFileSync(path.join(__dirname,'results-caption-source.json'),JSON.stringify(report,null,2)+'\n');
console.log(JSON.stringify({cases:cases.length,first:cases[0],multiFirst:cases[1]}));
