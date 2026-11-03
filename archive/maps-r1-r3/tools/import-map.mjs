// Converts a map pack's route + placements (Assets/map_*/Layout) into the game's map data (src/maps/<id>.json).
// The route is turned 180° and moved so its start gate (CP00) sits where the game's start deck is: (0, START_Y, 0),
// heading -Z. Everything else (gate test, grid, spawns, FREE ROAM hub) then works as on the Skyline Loop.
//   node tools/import-map.mjs ../Assets/map_R1_neon_downpour downpour 128
//   node tools/import-map.mjs ../Assets/map_R3_monsoon_falls monsoon 133
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const [src, id] = process.argv.slice(2);
if (!src || !id) throw new Error('usage: node tools/import-map.mjs <pack folder> <map id>');
// The start gate's height: 10 m above the Skyline Loop's 118, because these routes climb through the gate and dip
// ~10 m within 100 m behind it, where the start deck (top at 111 m) is.
const START_Y = Number(process.argv[4] ?? 128);
const route = JSON.parse(readFileSync(join(src, 'Layout/route.json'), 'utf8'));
const place = JSON.parse(readFileSync(join(src, 'Layout/placements.json'), 'utf8'));
const o = route.control_points[0].position_game;
const dy = START_Y - o[1];
const r2 = (v) => Math.round(v * 100) / 100;
const P = ([x, y, z]) => [r2(-(x - o[0])), r2(y + dy), r2(-(z - o[2]))]; // 180° about Y, start at the origin
const D = ([x, y, z]) => [r2(-x * 1e4) / 1e4, r2(y * 1e4) / 1e4, r2(-z * 1e4) / 1e4];
const yaw = (deg) => r2(((deg + 180) * Math.PI) / 180 * 1e3) / 1e3;

const out = {
  id,
  packId: route.map_id,
  name: route.map_name.toUpperCase(),
  source: src.replace(/\\/g, '/').split('/').slice(-1)[0],
  lengthM: Math.round(route.computed.length_m),
  startY: START_Y,
  routeWidth: route.route_width_m,
  waterY: r2((route.environment.water_level_y_m ?? 0) + dy),
  waterExtent: route.environment.water_extent_xz_m,
  palette: route.environment.palette,
  ctrl: route.control_points.map((c) => P(c.position_game)),
  checkpoints: route.checkpoints.filter((c) => c.sequence > 0).map((c) => ({ pos: P(c.position_game), fwd: D(c.forward_game) })),
  sectors: route.sectors.map((s) => ({ name: s.name, from: Math.round(s.start_distance_m), to: Math.round(s.end_distance_m) })),
  placements: place.instances.map((i) => ({
    id: i.id,
    asset: i.asset_id,
    kind: i.placeholder_kind,
    pos: P(i.position_game),
    yaw: yaw(i.yaw_degrees),
    size: i.size_xyz_m,
    parts: (i.collision_parts ?? []).map((p) => ({ id: p.id, c: p.center_local, s: p.size_xyz_m })),
  })),
};
const file = join(dirname(fileURLToPath(import.meta.url)), '../src/maps', `${id}.json`);
mkdirSync(dirname(file), { recursive: true });
writeFileSync(file, JSON.stringify(out, null, 1));
console.log(`${file}: ${out.name}, ${out.lengthM} m, ${out.checkpoints.length} rings, ${out.placements.length} placements, water y ${out.waterY}`);
