import * as THREE from 'three';
import { mergeGeometries } from 'three/addons/utils/BufferGeometryUtils.js';
import { OBB } from './physics.js';
import { buildCity, facadeBox } from './city.js';
import { windowTexture, hdr, MAGENTA, rng } from './materials.js';

/**
 * NEON DOWNPOUR (map pack R1): a rain-soaked night city standing in flood water. The pack's landmarks (placements in
 * src/maps/downpour.json) are built here: the twin-tower skybridge the route flies through, the cyan tower, the stepped
 * tower with its antenna, the midrise, the canal walls and bridge, the blank billboard (an ad screen) and a distant
 * cluster; the procedural city (city.js) fills the rest around them. Every landmark is solid along its collision parts.
 */
export function buildDownpour(scene, map, track, colliders, M) {
  const r = rng(808);
  const lists = { glass: [], neonC: [], neonM: [], concrete: [], tip: [] };
  const towers = []; // for wall screens (ads.js) and the FREE ROAM radar (hud.js)
  const screens = []; // fixed ad screens: { x, y, z, ang, w }
  const P = Object.fromEntries(map.placements.map((p) => [p.id, p]));
  const _m = new THREE.Matrix4();
  const at = (p) => _m.compose(new THREE.Vector3(...p.pos), new THREE.Quaternion().setFromAxisAngle(THREE.Object3D.DEFAULT_UP, p.yaw), new THREE.Vector3(1, 1, 1));
  const push = (list, geo, x, y, z, ry = 0) => {
    geo.applyMatrix4(new THREE.Matrix4().makeRotationY(ry).setPosition(x, y, z)).applyMatrix4(_m);
    lists[list].push(geo.index ? geo.toNonIndexed() : geo);
  };
  const box = (w, h, d) => new THREE.BoxGeometry(w, h, d);
  /** Solid collision boxes of a placement (its pack-authored parts). */
  const solid = (p) => {
    for (const part of p.parts) {
      const [cx, cy, cz] = part.c, [sx, sy, sz] = part.s;
      const c = Math.cos(p.yaw), s = Math.sin(p.yaw);
      colliders.add(new OBB(p.pos[0] + cx * c + cz * s, p.pos[1] + cy, p.pos[2] - cx * s + cz * c, sx / 2, sy / 2, sz / 2, p.yaw));
    }
  };
  /** A glass tower block with cyan corner ribs and a lit crown. */
  const block = (w, h, d, x, y0, z, accent = 'neonC') => {
    push('glass', facadeBox(w, h, d, r(), r()), x, y0 + h / 2, z);
    for (const [sx, sz] of [[1, 1], [-1, 1], [1, -1], [-1, -1]]) push(accent, box(0.7, h * 0.92, 0.7), x + sx * (w / 2 + 0.2), y0 + h * 0.5, z + sz * (d / 2 + 0.2));
    for (const yy of [y0 + h - 1.5]) {
      push(accent, box(w + 1, 0.6, 0.6), x, yy, z + d / 2 + 0.3);
      push(accent, box(w + 1, 0.6, 0.6), x, yy, z - d / 2 - 0.3);
      push(accent, box(0.6, 0.6, d + 1), x + w / 2 + 0.3, yy, z);
      push(accent, box(0.6, 0.6, d + 1), x - w / 2 - 0.3, yy, z);
    }
  };
  const addTower = (p, top) => {
    const c = Math.cos(p.yaw), s = Math.sin(p.yaw);
    towers.push({ x: p.pos[0], z: p.pos[2], w: p.size[0], d: p.size[2], top, rot: p.yaw, hx: p.size[0] / 2, hz: p.size[2] / 2, near: true, poster: false, c, s });
  };
  const water = map.waterY;
  // the buildings stand in the flood: their bases go a few metres under the water
  const sink = 6;

  // ---- N03: twin towers joined by the high skybridge; the route flies through the 75 m opening between them
  if (P.N03_01) {
    const p = P.N03_01;
    at(p);
    const [lt, , br] = p.parts;
    for (const sx of [-1, 1]) {
      const x = sx * Math.abs(lt.c[0]), w = lt.s[0], d = lt.s[2], h = lt.s[1];
      block(w, h + sink, d, x, -sink, 0);
      // inner faces: tall cyan light ribs facing the opening, magenta beacons on top
      for (const k of [-0.3, 0, 0.3]) push('neonC', box(0.9, h * 0.8, 0.9), x - sx * (w / 2 + 0.3), h * 0.45, k * d);
      push('glass', facadeBox(w * 0.7, 26, d * 0.7, r(), r()), x, h + 13, 0);
      push('tip', box(1.4, 2, 1.4), x, h + 27, 0);
    }
    // the bridge: a deck with cyan underside lines and windows along its length
    const [bw, bh, bd] = br.s;
    push('glass', facadeBox(bw, bh, bd, r(), r()), 0, br.c[1], 0);
    for (const sz of [-1, 1]) {
      push('neonC', box(bw - 4, 0.6, 0.6), 0, br.c[1] - bh / 2 - 0.3, sz * (bd / 2 - 2));
      push('neonM', box(bw - 10, 0.45, 0.45), 0, br.c[1] + bh / 2 + 0.25, sz * (bd / 2 + 0.2));
    }
    solid(p);
    addTower({ ...p, pos: [p.pos[0] + lt.c[0] * Math.cos(p.yaw), 0, p.pos[2] - lt.c[0] * Math.sin(p.yaw)], size: lt.s }, p.pos[1] + lt.s[1]);
  }
  // ---- N01: tall cyan-ribbed tower
  for (const id of ['N01_01', 'N04_01']) {
    const p = P[id];
    if (!p) continue;
    at(p);
    const [w, h, d] = p.size;
    block(w, h + sink, d, 0, -sink, 0, id === 'N04_01' ? 'neonM' : 'neonC');
    if (id === 'N04_01') for (let k = 0; k < 5; k++) push('concrete', box(5, 4, 6), (r() - 0.5) * w * 0.7, h + 2, (r() - 0.5) * d * 0.7);
    else push('tip', box(1.2, 2, 1.2), 0, h + 1, 0);
    solid(p);
    addTower(p, p.pos[1] + h);
  }
  // ---- N02: stepped tower in three tiers, N09 antenna mast on its roof
  if (P.N02_01) {
    const p = P.N02_01;
    at(p);
    const [w, h, d] = p.size;
    const tiers = [[1, 0.55], [0.8, 0.8], [0.6, 1]];
    let y0 = -sink;
    for (const [k, top] of tiers) {
      const y1 = h * top;
      block(w * k, y1 - y0, d * k, 0, y0, 0, k < 0.7 ? 'neonM' : 'neonC');
      y0 = y1;
    }
    solid(p);
    addTower(p, p.pos[1] + h);
  }
  if (P.N09_01) {
    const p = P.N09_01;
    at(p);
    push('concrete', new THREE.CylinderGeometry(0.5, 1.6, p.size[1], 6), 0, p.size[1] / 2, 0);
    push('tip', box(1.6, 2.2, 1.6), 0, p.size[1] + 1, 0);
  }
  // ---- N06 canal walls and N07 bridge below the route
  for (const id of ['N06_01', 'N06_02']) {
    const p = P[id];
    if (!p) continue;
    at(p);
    const [w, h, d] = p.size;
    push('concrete', box(w, h + sink, d), 0, (h - sink) / 2, 0);
    push('neonC', box(0.6, 0.6, d), w / 2 + 0.3, h - 1, 0);
    push('neonC', box(0.6, 0.6, d), -w / 2 - 0.3, h - 1, 0);
    solid(p);
  }
  if (P.N07_01) {
    const p = P.N07_01;
    at(p);
    const [w, h, d] = p.size;
    push('concrete', box(w, h, d), 0, h / 2, 0);
    for (const sx of [-0.35, 0.35]) push('concrete', box(6, p.pos[1] - water + sink, 8), sx * w, -(p.pos[1] - water + sink) / 2, 0);
    for (const sz of [-1, 1]) {
      push('neonM', box(w, 0.5, 0.5), 0, h + 1.2, sz * (d / 2 - 0.5));
      push('neonC', box(w, 0.4, 0.4), 0, -0.2, sz * (d / 2 + 0.2));
    }
    solid(p);
  }
  // ---- N10: the pack's blank billboard becomes an ad screen on a frame (screens for rent)
  if (P.N10_01) {
    const p = P.N10_01;
    at(p);
    const [w, h, d] = p.size;
    push('concrete', box(w + 2, h + 2, d), 0, h / 2, 0);
    for (const sx of [-0.3, 0.3]) push('concrete', box(2.4, p.pos[1] - water + sink, 2.4), sx * w, -(p.pos[1] - water + sink) / 2, 0);
    // the screen faces the route: the side of the frame nearer the track
    const n = track.nearestGlobal(new THREE.Vector3(...p.pos));
    const q = track.pts[n];
    const front = Math.sin(p.yaw) * (q.x - p.pos[0]) + Math.cos(p.yaw) * (q.z - p.pos[2]) > 0 ? 0 : Math.PI;
    const ang = p.yaw + front;
    const off = d / 2 + 0.6;
    screens.push({ x: p.pos[0] + Math.sin(ang) * off, y: p.pos[1] + h / 2, z: p.pos[2] + Math.cos(ang) * off, ang, w: w - 2 });
    solid(p);
  }
  // ---- N12: a distant cluster (scenery only, nothing to hit)
  if (P.N12_01) {
    const p = P.N12_01;
    at(p);
    const [w, h, d] = p.size;
    for (let k = 0; k < 9; k++) {
      const bw = 26 + r() * 30, bh = h * (0.5 + r() * 0.6);
      block(bw, bh + sink, bw, (r() - 0.5) * w, -sink, (r() - 0.5) * d, r() < 0.3 ? 'neonM' : 'neonC');
    }
  }

  // ---- the rest of the city, kept off the landmarks
  const avoid = map.placements.filter((p) => p.parts.length || p.asset === 'N12')
    .map((p) => ({ x: p.pos[0], z: p.pos[2], r: Math.hypot(p.size[0], p.size[2]) / 2 + 30 }));
  const city = buildCity(scene, track, colliders, M, {
    base: water - sink, cx: 450, cz: 0, extent: 2100, accent: 0.8, facade: 0x0b0f1a, facadeRough: 0.18, windows: 0.5, seed: 5150, avoid,
  });

  // ---- materials and meshes
  const winTex = windowTexture();
  const mats = {
    glass: new THREE.MeshStandardMaterial({ color: 0x0b0f1a, metalness: 0.85, roughness: 0.16, emissive: 0xffffff, emissiveMap: winTex, emissiveIntensity: 0.5, envMapIntensity: 0.45 }),
    concrete: new THREE.MeshStandardMaterial({ color: 0x151d26, metalness: 0.4, roughness: 0.55 }),
    neonC: M.neonC,
    neonM: M.neonM,
    tip: new THREE.MeshBasicMaterial({ color: hdr(MAGENTA, 6) }),
  };
  const group = new THREE.Group();
  for (const [k, list] of Object.entries(lists)) {
    if (!list.length) continue;
    const g = mergeGeometries(list, false);
    g.computeBoundingSphere();
    const mesh = new THREE.Mesh(g, mats[k]);
    mesh.matrixAutoUpdate = false;
    group.add(mesh);
  }
  scene.add(group);

  return {
    towers: [...towers, ...city.towers],
    screens,
    adBase: water,
    update(t) {
      mats.tip.color.copy(MAGENTA).multiplyScalar(Math.sin(t * 3) > 0.2 ? 7 : 0.6);
      city.update(t);
    },
  };
}
