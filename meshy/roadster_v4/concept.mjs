// v0.3.9 ROADSTER pilot, step 1: concept views from renders of the current car (Meshy image-to-image, nano-banana-2,
// multi-view: 6 credits). Usage: node concept.mjs <ref.png> [<ref.png> ...]  -> concept_task.json, concept_<i>.png
import { readFileSync, writeFileSync } from 'node:fs';
const KEY = process.env.MESHY_API_KEY, API = 'https://api.meshy.ai/openapi/v1/image-to-image';
const H = { Authorization: `Bearer ${KEY}`, 'Content-Type': 'application/json' };
const refs = process.argv.slice(2).map((f) => 'data:image/png;base64,' + readFileSync(f).toString('base64'));
const prompt = 'Redesign this car as a clean, production-quality futuristic sports car for a racing game, keeping its '
  + 'silhouette, low wedge nose, cockpit and rear wing. Smooth, crisp, symmetric body panels with sharp creases, no '
  + 'dents or lumps. Gloss black paint with thin magenta and cyan neon light lines along the sides and nose. Four large '
  + 'wheels in open, deep wheel arches with clear gaps between wheel and body: solid machined dark iron wheels (metal '
  + 'tyres, no rubber) with a 5-spoke rim. The car should look equally right driving on a road and hovering. Full car '
  + 'visible, plain white background, soft studio light, no shadow, no text.';
const r = await fetch(API, { method: 'POST', headers: H, body: JSON.stringify({ ai_model: 'nano-banana-2', prompt, reference_image_urls: refs, generate_multi_view: true }) });
const j = await r.json(); console.log(r.status, JSON.stringify(j)); if (!j.result) process.exit(1);
writeFileSync('concept_task.json', JSON.stringify(j));
for (;;) {
  await new Promise((s) => setTimeout(s, 8000));
  const t = await (await fetch(`${API}/${j.result}`, { headers: H })).json();
  console.log(t.status, t.progress ?? '');
  if (t.status === 'SUCCEEDED') {
    writeFileSync('concept_result.json', JSON.stringify(t, null, 1));
    let i = 0; for (const u of t.image_urls) writeFileSync(`concept_${i++}.png`, Buffer.from(await (await fetch(u)).arrayBuffer()));
    console.log('credits', t.consumed_credits, 'images', t.image_urls.length); break;
  }
  if (['FAILED', 'CANCELED'].includes(t.status)) { console.log(JSON.stringify(t.task_error)); process.exit(1); }
}
