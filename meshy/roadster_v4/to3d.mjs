// v0.3.9 ROADSTER pilot, step 2: multi-image-to-3D from the concept views (meshy-7 latest, textured, PBR: 30 credits).
// Usage (in roadster_v4/): node to3d.mjs view_0.png view_1.png view_2.png  -> task.json, result.json, model.glb
import { readFileSync, writeFileSync } from 'node:fs';
const KEY = process.env.MESHY_API_KEY, API = 'https://api.meshy.ai/openapi/v1/multi-image-to-3d';
const H = { Authorization: `Bearer ${KEY}`, 'Content-Type': 'application/json' };
const image_urls = process.argv.slice(2).map((f) => 'data:image/png;base64,' + readFileSync(f).toString('base64'));
const body = { image_urls, ai_model: 'latest', should_texture: true, enable_pbr: true, should_remesh: false, symmetry_mode: 'on' };
const r = await fetch(API, { method: 'POST', headers: H, body: JSON.stringify(body) });
const j = await r.json(); console.log(r.status, JSON.stringify(j)); if (!j.result) process.exit(1);
writeFileSync('task.json', JSON.stringify(j));
for (;;) {
  await new Promise((s) => setTimeout(s, 10000));
  const t = await (await fetch(`${API}/${j.result}`, { headers: H })).json();
  console.log(t.status, t.progress ?? '');
  if (t.status === 'SUCCEEDED') {
    writeFileSync('result.json', JSON.stringify(t, null, 1));
    writeFileSync('model.glb', Buffer.from(await (await fetch(t.model_urls.glb)).arrayBuffer()));
    console.log('credits', t.consumed_credits); break;
  }
  if (['FAILED', 'CANCELED'].includes(t.status)) { console.log(JSON.stringify(t.task_error)); process.exit(1); }
}
