// v0.3.9 tyre shop: one concept sheet of ten wheel designs per car (Meshy text-to-image, gpt-image-2: 9 credits each).
// Usage (in meshy/tires/): node concepts.mjs roadster|viper|titan ...  -> concept_<car>.png, concept_<car>.json
import { writeFileSync } from 'node:fs';
const KEY = process.env.MESHY_API_KEY, API = 'https://api.meshy.ai/openapi/v1/text-to-image';
const H = { Authorization: `Bearer ${KEY}`, 'Content-Type': 'application/json' };
const BASE = 'Product design sheet for a futuristic neon racing game: ten clearly different wheel designs (rim + tyre), '
  + 'each shown from its outer face, slightly three-quarter, arranged in a neat 5 by 2 grid on a plain dark grey '
  + 'background, even studio light, no car, no text, no labels. Every wheel has a different rim style, spoke count, '
  + 'finish and tyre. ';
const CARS = {
  roadster: 'For a sleek black hover sports car with magenta neon: iron and machined-steel wheels, no rubber needed: '
    + 'turned steel bands, chrome, gunmetal, gold, carbon; split spokes, mesh lattice, turbine blades, disc covers, '
    + 'thin glowing accent rings in magenta, cyan, gold, red, green.',
  viper: 'For a low silver supercar: low-profile performance tyres and forged aero rims: thin multi-spoke, twisted '
    + 'blades, aero disc covers, Y-spokes, centre-lock hubs, carbon fibre, polished alloy, anodised colours, glowing '
    + 'accent rings in pink, ice blue, gold, toxic green, violet.',
  titan: 'For a heavy armoured off-road truck-car: big chunky off-road tyres with deep lugs and beadlock rims: armour '
    + 'plates, bolted rings, deep dish, six and eight thick spokes, hex hubs, gunmetal, matte olive, desert tan, chrome, '
    + 'hazard orange accents and glowing cyan or amber rings.',
};
for (const car of process.argv.slice(2)) {
  const r = await fetch(API, { method: 'POST', headers: H, body: JSON.stringify({ ai_model: 'gpt-image-2', prompt: BASE + CARS[car], aspect_ratio: '16:9' }) });
  const j = await r.json(); console.log(car, r.status, JSON.stringify(j)); if (!j.result) continue;
  for (;;) {
    await new Promise((s) => setTimeout(s, 8000));
    const t = await (await fetch(`${API}/${j.result}`, { headers: H })).json();
    if (t.status === 'SUCCEEDED') {
      writeFileSync(`concept_${car}.json`, JSON.stringify(t, null, 1));
      writeFileSync(`concept_${car}.png`, Buffer.from(await (await fetch(t.image_urls[0])).arrayBuffer()));
      console.log(car, 'credits', t.consumed_credits); break;
    }
    if (['FAILED', 'CANCELED'].includes(t.status)) { console.log(car, JSON.stringify(t.task_error)); break; }
  }
}
