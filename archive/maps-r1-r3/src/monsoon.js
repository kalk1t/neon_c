import * as THREE from 'three';
import { mergeGeometries } from 'three/addons/utils/BufferGeometryUtils.js';
import { OBB, Cyl } from './physics.js';
import { makeCanvas, hdr, CYAN, rng } from './materials.js';

/**
 * MONSOON FALLS (map pack R3): a river canyon through jungle plateaus, overcast daylight, rain and mist.
 * The route (src/maps/monsoon.json) runs along the canyon: past the basalt cliff and the observation pod, through the
 * natural arch, low past the mesa, and high over the dam and its reservoir. Built here, all procedural:
 *  - the terrain: a heightfield (near grid 10 m, far grid 50 m) carved along the route, with karst peaks far out.
 *    It is solid: colliders.ground(x, z) (physics.js) pushes cars and shots up out of it;
 *  - the pack's landmarks (placements): basalt columns, the arch, the mesa, the dam with spillway, reservoir and
 *    waterfalls, the pod on its platform, trees, the boulder, vines;
 *  - waterfalls down the canyon walls (shader ribbons draped over the slope), instanced jungle;
 *  - screens for rent: steel pylons along the route and two screens on the dam face (ads.js draws the screens).
 */
export function buildMonsoon(scene, map, track, colliders) {
  const r = rng(3303);
  const W = map.waterY;
  const P = Object.fromEntries(map.placements.map((p) => [p.id, p]));
  const group = new THREE.Group();
  scene.add(group);
  const screens = [];
  const S = (a, b, t) => { t = Math.min(1, Math.max(0, (t - a) / (b - a))); return t * t * (3 - 2 * t); };

  // ---------------- terrain ----------------
  const route = track.pts.filter((_, i) => i % Math.max(1, Math.round(track.pts.length / 700)) === 0);
  /** Horizontal distance to the route and the route's height there. */
  const nearRoute = (x, z) => {
    let best = Infinity, y = 0;
    for (const q of route) {
      const d = (q.x - x) ** 2 + (q.z - z) ** 2;
      if (d < best) { best = d; y = q.y; }
    }
    return [Math.sqrt(best), y];
  };
  const DAM = P.F04_01 ? { x: P.F04_01.pos[0], z: P.F04_01.pos[2] } : { x: 506, z: -330 };
  const LAKE = { x: DAM.x + 114, z: DAM.z, rx: 105, rz: 62, y: 104 };
  function height(x, z) {
    const [d, ry] = nearRoute(x, z);
    const bed = W - 6 + 3 * vnoise(x / 25, z / 25);
    let plat = 92 + 52 * fbm(x / 380, z / 380, 5) + 12 * fbm(x / 55, z / 55, 3);
    plat += S(0.5, 0.78, fbm(x / 300 + 40, z / 300 - 20, 4)) * 320 * S(260, 620, d); // karst peaks far out
    let h = bed + (plat - bed) * S(62, 150 + 40 * vnoise(x / 90, z / 90), d);
    // the dam sits in a raised shelf; its reservoir behind it (east), its gorge below it (west)
    const shelf = ((x - DAM.x - 50) / 210) ** 2 + ((z - DAM.z) / 135) ** 2;
    h += (Math.max(h, 116 + 8 * vnoise(x / 40, z / 40)) - h) * (1 - S(0.75, 1, shelf));
    const e = ((x - LAKE.x) / LAKE.rx) ** 2 + ((z - LAKE.z) / LAKE.rz) ** 2;
    if (e < 1) h = 78 + (h - 78) * S(0.7, 1, e);
    const g = (1 - S(52, 72, Math.abs(z - DAM.z))) * (1 - S(DAM.x - 14, DAM.x - 4, x)) * S(250, 300, x);
    h += (bed - h) * g;
    return Math.min(h, ry - 22 + Math.max(0, d - 20) * 1.3); // always clear of the route
  }
  const grid = (x0, z0, step, nx, nz, sink) => {
    const hs = new Float32Array(nx * nz);
    for (let j = 0; j < nz; j++) for (let i = 0; i < nx; i++) hs[j * nx + i] = height(x0 + i * step, z0 + j * step) - sink;
    return { x0, z0, step, nx, nz, hs };
  };
  const near = grid(-420, -920, 10, 194, 185, 0);
  const far = grid(-2300, -2800, 50, 115, 113, 4); // sunk a little: hidden under the near grid where both are
  const sample = (G, x, z) => {
    const fx = (x - G.x0) / G.step, fz = (z - G.z0) / G.step;
    const i = Math.floor(fx), j = Math.floor(fz);
    if (i < 0 || j < 0 || i >= G.nx - 1 || j >= G.nz - 1) return null;
    const u = fx - i, v = fz - j, k = j * G.nx + i, h = G.hs;
    return (h[k] * (1 - u) + h[k + 1] * u) * (1 - v) + (h[k + G.nx] * (1 - u) + h[k + G.nx + 1] * u) * v;
  };
  const ground = (x, z) => sample(near, x, z) ?? sample(far, x, z) ?? -50;
  colliders.ground = ground;

  const grain = (() => {
    const [c, g] = makeCanvas(256, 256), img = g.createImageData(256, 256);
    for (let y = 0; y < 256; y++) for (let x = 0; x < 256; x++) {
      const v = 150 + 105 * (0.6 * vnoise(x / 9, y / 9) + 0.4 * vnoise(x / 2.5, y / 2.5)) - 50;
      const k = (y * 256 + x) * 4;
      img.data[k] = img.data[k + 1] = img.data[k + 2] = v;
      img.data[k + 3] = 255;
    }
    g.putImageData(img, 0, 0);
    const t = new THREE.CanvasTexture(c);
    t.wrapS = t.wrapT = THREE.RepeatWrapping;
    t.colorSpace = THREE.SRGBColorSpace;
    return t;
  })();
  const terrainMat = new THREE.MeshStandardMaterial({ vertexColors: true, map: grain, roughness: 0.92, metalness: 0 });
  const cBed = new THREE.Color(0x262b27), cRock = new THREE.Color(0x40453f), cRock2 = new THREE.Color(0x30352f),
    cMoss = new THREE.Color(0x3d5a2c), cJungle = new THREE.Color(0x2c5222), cJungle2 = new THREE.Color(0x46702c), tmp = new THREE.Color();
  for (const G of [near, far]) {
    const geo = new THREE.PlaneGeometry((G.nx - 1) * G.step, (G.nz - 1) * G.step, G.nx - 1, G.nz - 1);
    geo.rotateX(-Math.PI / 2);
    const pos = geo.attributes.position, uv = geo.attributes.uv;
    // PlaneGeometry rows run from -z to +z after the rotation; vertex k = row j (z), column i (x)
    for (let k = 0; k < pos.count; k++) {
      const i = k % G.nx, j = Math.floor(k / G.nx);
      const x = G.x0 + i * G.step, z = G.z0 + j * G.step;
      pos.setXYZ(k, x, G.hs[j * G.nx + i], z);
      uv.setXY(k, x / 36, z / 36);
    }
    geo.computeVertexNormals();
    const nor = geo.attributes.normal, col = new Float32Array(pos.count * 3);
    for (let k = 0; k < pos.count; k++) {
      const x = pos.getX(k), y = pos.getY(k), z = pos.getZ(k), ny = nor.getY(k);
      const n = vnoise(x / 30, z / 30);
      if (y < W + 4) tmp.copy(cBed);
      else if (ny < 0.72) tmp.copy(cRock).lerp(cRock2, n).lerp(cMoss, S(0.45, 0.72, ny) * 0.8 + 0.25 * vnoise(x / 12, y / 12));
      else tmp.copy(cJungle).lerp(cJungle2, n * 0.8).lerp(cMoss, 0.3 * vnoise(x / 7, z / 7));
      col.set([tmp.r, tmp.g, tmp.b], k * 3);
    }
    geo.setAttribute('color', new THREE.BufferAttribute(col, 3));
    const mesh = new THREE.Mesh(geo, terrainMat);
    mesh.matrixAutoUpdate = false;
    group.add(mesh);
  }

  // ---------------- landmark helpers ----------------
  const lists = { basalt: [], rock: [], moss: [], concrete: [], steel: [], glass: [], cyan: [], warm: [], vine: [] };
  const _m = new THREE.Matrix4();
  const at = (p, yaw = p.yaw) => { _m.compose(new THREE.Vector3(...p.pos), new THREE.Quaternion().setFromAxisAngle(THREE.Object3D.DEFAULT_UP, yaw), new THREE.Vector3(1, 1, 1)); };
  const push = (list, geo, x = 0, y = 0, z = 0, ry = 0) => {
    geo.applyMatrix4(new THREE.Matrix4().makeRotationY(ry).setPosition(x, y, z)).applyMatrix4(_m);
    lists[list].push(geo.index ? geo.toNonIndexed() : geo);
  };
  const box = (w, h, d) => new THREE.BoxGeometry(w, h, d);
  /** Local point of the current placement in world space. */
  const toWorld = (x, y, z) => new THREE.Vector3(x, y, z).applyMatrix4(_m);
  const solid = (p) => {
    const c = Math.cos(p.yaw), s = Math.sin(p.yaw);
    for (const part of p.parts) {
      const [cx, cy, cz] = part.c, [sx, sy, sz] = part.s;
      colliders.add(new OBB(p.pos[0] + cx * c + cz * s, p.pos[1] + cy, p.pos[2] - cx * s + cz * c, sx / 2, sy / 2, sz / 2, p.yaw));
    }
  };
  /** A rough rock: an icosahedron with its vertices pushed about. */
  const lump = (rad, seed, detail = 1) => {
    const g = new THREE.IcosahedronGeometry(rad, detail);
    const p = g.attributes.position;
    for (let k = 0; k < p.count; k++) {
      const x = p.getX(k), y = p.getY(k), z = p.getZ(k);
      const f = 0.75 + 0.45 * vnoise(x / rad * 2 + seed, z / rad * 2 + y / rad);
      p.setXYZ(k, x * f, y * f, z * f);
    }
    g.computeVertexNormals();
    return g;
  };
  const trees = []; // { x, y, z, s, palm }

  // ---- F01: basalt cliff of hexagonal columns
  for (const id of ['F01_01']) {
    const p = P[id];
    if (!p) continue;
    at(p);
    const [w, h, d] = p.size;
    for (let x = -w / 2 + 5; x <= w / 2 - 5; x += 8.6) for (let z = -d / 2 + 5; z <= d / 2 - 5; z += 7.5) {
      const zz = z + ((Math.round((x + w) / 8.6) % 2) * 3.75);
      if (zz > d / 2 - 4) continue;
      const edge = Math.min(w / 2 - Math.abs(x), d / 2 - Math.abs(zz)) / 18;
      const ch = h * (0.72 + 0.28 * Math.min(1, edge)) * (0.92 + r() * 0.1);
      push(r() < 0.5 ? 'basalt' : 'rock', new THREE.CylinderGeometry(4.9, 5.1, ch + 8, 6), x, ch / 2 - 4, zz, r() * 0.2);
      if (r() < 0.4) push('moss', new THREE.CylinderGeometry(5.0, 5.0, 1.2, 6), x, ch - 3.4, zz);
      if (r() < 0.12) trees.push({ ...toWorld(x, ch - 4, zz), s: 0.8 + r() * 0.4, palm: r() < 0.4 });
    }
    solid(p);
  }
  // ---- F12 vines hanging off the cliff top
  if (P.F12_01) {
    const p = P.F12_01;
    at(p);
    for (let k = 0; k < 16; k++) {
      const len = p.size[1] * (0.35 + r() * 0.65);
      push('vine', box(0.35, len, 0.35), (k - 8) * 1.3 + r() * 0.6, p.size[1] - len / 2, r() * 0.8);
    }
  }
  // ---- F02: the natural arch the route flies through (opening 44 m wide, 77 m high)
  if (P.F02_01) {
    const p = P.F02_01;
    at(p);
    const [lp, rp, cr] = p.parts;
    const x0 = lp.c[0] + lp.s[0] / 2, x1 = rp.c[0] - rp.s[0] / 2, top = cr.c[1] - cr.s[1] / 2, spring = top - (x1 - x0) / 2 * 0.8;
    const outer = new THREE.Shape();
    const ol = lp.c[0] - lp.s[0] / 2 - 6, or = rp.c[0] + rp.s[0] / 2 + 6;
    outer.moveTo(ol, -10);
    [[ol + 2, 40], [ol - 4, 70], [ol + 6, cr.c[1] + 4], [ol + 22, cr.c[1] + 12], [0, cr.c[1] + cr.s[1] / 2 + 8], [or - 20, cr.c[1] + 13], [or - 4, cr.c[1] + 5], [or + 4, 70], [or - 1, 40], [or, -10]]
      .forEach(([x, y]) => outer.lineTo(x, y));
    const hole = new THREE.Path();
    const mx = (x0 + x1) / 2, hw = (x1 - x0) / 2;
    hole.moveTo(x0, -6);
    hole.lineTo(x1, -6);
    hole.lineTo(x1, spring);
    hole.absellipse(mx, spring, hw, top - spring, 0, Math.PI, false);
    hole.lineTo(x0, -6);
    outer.holes.push(hole);
    const depth = cr.s[2];
    const g = new THREE.ExtrudeGeometry(outer, { depth, bevelEnabled: true, bevelThickness: 4, bevelSize: 3, bevelSegments: 2, curveSegments: 18 });
    g.translate(0, 0, -depth / 2);
    push('rock', g);
    // moss along the crown and ferny trees on top
    push('moss', box(or - ol - 26, 3, depth + 4), 0, cr.c[1] + cr.s[1] / 2 + 5, 0);
    for (let k = 0; k < 7; k++) trees.push({ ...toWorld((r() - 0.5) * (or - ol - 34), cr.c[1] + cr.s[1] / 2 + 5, (r() - 0.5) * depth * 0.7), s: 0.7 + r() * 0.4, palm: r() < 0.5 });
    for (let k = 0; k < 14; k++) {
      const len = 8 + r() * 22;
      push('vine', box(0.3, len, 0.3), mx + (r() - 0.5) * hw * 1.6, top - len / 2, (r() - 0.5) * depth);
    }
    solid(p);
    // the corners of the rounded opening (the pack's parts are boxes)
    for (const sx of [-1, 1]) colliders.add(new OBB(...toWorld(mx + sx * (hw - 4), (spring + top) / 2 + 4, 0).toArray(), 4, (top - spring) / 2 - 4, depth / 2, p.yaw));
  }
  // ---- F03: the mesa, trees on its flat top (F07, F08, F09)
  if (P.F03_01) {
    const p = P.F03_01;
    at(p);
    const [w, h, d] = p.size;
    const g = new THREE.CylinderGeometry(0.47, 0.53, 1, 14, 8);
    const q = g.attributes.position;
    for (let k = 0; k < q.count; k++) {
      const x = q.getX(k), y = q.getY(k), z = q.getZ(k);
      const f = 1 + (y < 0.49 ? 0.1 * (vnoise(Math.atan2(z, x) * 3, y * 6) - 0.5) : 0);
      q.setXYZ(k, x * f * w, (y + 0.5) * (h + 6) - 6, z * f * d);
    }
    g.computeVertexNormals();
    push('basalt', g);
    push('moss', new THREE.CylinderGeometry(0.47 * Math.min(w, d) * 0.98, 0.47 * Math.min(w, d), 2.4, 14), 0, h + 0.4, 0);
    for (let k = 0; k < 26; k++) {
      const a = r() * Math.PI * 2, rr = Math.sqrt(r()) * 0.4;
      trees.push({ ...toWorld(Math.cos(a) * rr * w, h + 1, Math.sin(a) * rr * d), s: 0.8 + r() * 0.5, palm: r() < 0.35 });
    }
    colliders.add(new Cyl(p.pos[0], p.pos[2], Math.min(w, d) * 0.5, p.pos[1] - 20, p.pos[1] + h));
  }
  for (const id of ['F07_01', 'F08_01']) {
    const p = P[id];
    if (p) trees.push({ x: p.pos[0], y: p.pos[1], z: p.pos[2], s: p.size[1] / 26, palm: id === 'F08_01' });
  }
  if (P.F09_01) { at(P.F09_01); push('moss', lump(4, 3, 0), 0, 1, 0); }
  // ---- F10: the mossy boulder in the river
  if (P.F10_01) {
    const p = P.F10_01;
    at(p);
    push('rock', lump(p.size[0] / 2, 7), 0, p.size[1] / 2 - 2, 0);
    push('moss', lump(p.size[0] / 2.4, 9), 0, p.size[1] / 2 + 3, 0);
    solid(p);
  }

  // ---- F04 dam, F05 spillway, F11 the falls; the face looks west, at the traffic coming over the plateau
  const falls = []; // ribbons: { pts: [Vector3], w, side: Vector3 }
  if (P.F04_01) {
    const p = P.F04_01;
    at(p, p.yaw + Math.PI); // local +z = downstream (west)
    const [w, h, d] = p.size, base = -10;
    push('concrete', box(w, h - base, d), 0, (h + base) / 2, 0);
    // the downstream face: pilasters, a cyan line along the crest, the walkway and its rail
    for (let k = -3; k <= 3; k++) push('concrete', box(5, h - base, 5), k * (w / 7), (h + base) / 2, d / 2 + 2.4);
    push('concrete', box(w + 8, 4, d + 10), 0, h + 2, 0);
    for (const sz of [-1, 1]) {
      push('cyan', box(w + 8, 0.6, 0.6), 0, h + 0.2, sz * (d / 2 + 5.3));
      push('steel', box(w + 8, 0.3, 0.3), 0, h + 5.2, sz * (d / 2 + 4.6));
    }
    for (let k = 0; k < 6; k++) push('cyan', box(0.6, h - 12, 0.6), (k - 2.5) * (w / 6), (h - 12) / 2 + 4, d / 2 + 5.1);
    // towers at both ends
    for (const sx of [-1, 1]) {
      push('concrete', box(16, 26, 18), sx * (w / 2 + 2), h + 13, 0);
      push('glass', box(16.4, 6, 18.4), sx * (w / 2 + 2), h + 18, 0);
      push('cyan', box(17, 0.5, 19), sx * (w / 2 + 2), h + 26.2, 0);
      colliders.add(new OBB(...toWorld(sx * (w / 2 + 2), h + 13, 0).toArray(), 8, 13, 9, p.yaw));
    }
    solid(p);
    // spillway gates at the crest
    if (P.F05_01) {
      const s = P.F05_01;
      const [sw, sh, sd] = s.size, y0 = s.pos[1] - p.pos[1];
      for (let k = -2; k <= 2; k++) push('concrete', box(3, sh + 6, sd), k * (sw / 4), y0 + sh / 2 + 3, 0);
      push('concrete', box(sw + 4, 4, sd), 0, y0 + sh + 6, 0);
      push('cyan', box(sw + 4, 0.5, 0.5), 0, y0 + sh + 3.8, sd / 2 + 0.3);
      solid(s);
    }
    // the main fall leaves the spillway lip and arcs out; two narrower falls run down the face
    const lip = (P.F05_01 ? P.F05_01.pos[1] - p.pos[1] : h - 12) + 2;
    const arc = [];
    for (let k = 0; k <= 22; k++) {
      const s = k / 22;
      arc.push(toWorld(0, lip - (lip - (W - p.pos[1])) * s, d / 2 + 2 + 20 * Math.sqrt(Math.min(1, s * 3))));
    }
    falls.push({ pts: arc, w: P.F11_01 ? P.F11_01.size[0] : 38, side: toWorld(1, 0, 0).sub(toWorld(0, 0, 0)) });
    for (const sx of [-1, 1]) {
      const pts = [];
      for (let k = 0; k <= 12; k++) pts.push(toWorld(sx * w * 0.36, h - 3 - (h - 3 - (W - p.pos[1])) * (k / 12), d / 2 + 6 + 3 * Math.sqrt(k / 12)));
      falls.push({ pts, w: 12, side: toWorld(1, 0, 0).sub(toWorld(0, 0, 0)) });
    }
    // screens for rent on the dam face, either side of the main fall
    const n = toWorld(0, 0, 1).sub(toWorld(0, 0, 0));
    for (const sx of [-1, 1]) {
      const c = toWorld(sx * 42, h * 0.58, d / 2 + 5.2);
      screens.push({ x: c.x, y: c.y, z: c.z, ang: Math.atan2(n.x, n.z), w: 26 });
    }
  }
  // the reservoir behind the dam
  const lake = new THREE.Mesh(new THREE.CircleGeometry(1, 64).rotateX(-Math.PI / 2),
    new THREE.MeshStandardMaterial({ color: 0x1b3934, roughness: 0.08, metalness: 0.35, envMapIntensity: 1.1 }));
  lake.scale.set(LAKE.rx, 1, LAKE.rz);
  lake.position.set(LAKE.x, LAKE.y, LAKE.z);
  group.add(lake);

  // ---- C15 service platform and F06 observation pod, on struts down to the rock
  if (P.C15_01) {
    const p = P.C15_01;
    at(p);
    const [w, h, d] = p.size;
    push('steel', box(w, h, d), 0, h / 2, 0);
    push('cyan', box(w + 0.6, 0.4, 0.4), 0, h - 0.2, d / 2 + 0.2);
    push('cyan', box(w + 0.6, 0.4, 0.4), 0, h - 0.2, -d / 2 - 0.2);
    for (const [sx, sz] of [[1, 1], [-1, 1], [1, -1], [-1, -1]]) {
      const wp = toWorld(sx * w * 0.38, 0, sz * d * 0.38);
      const len = p.pos[1] - ground(wp.x, wp.z) + 6;
      push('steel', box(1.6, len, 1.6), sx * w * 0.38, -len / 2, sz * d * 0.38);
    }
    solid(p);
  }
  if (P.F06_01) {
    const p = P.F06_01;
    at(p);
    const [w, h] = p.size, rad = w / 2;
    push('steel', new THREE.CylinderGeometry(rad, rad * 0.8, 2, 16), 0, 1, 0);
    push('warm', new THREE.CylinderGeometry(rad - 0.4, rad - 0.4, h - 4, 16, 1, true), 0, h / 2, 0);
    push('glass', new THREE.CylinderGeometry(rad, rad, h - 4, 16, 1, true), 0, h / 2, 0);
    for (let k = 0; k < 8; k++) push('steel', box(0.5, h - 4, 0.5), Math.cos(k * 0.785) * rad, h / 2, Math.sin(k * 0.785) * rad);
    push('steel', new THREE.CylinderGeometry(rad + 2, rad, 2, 16), 0, h - 1, 0);
    push('cyan', new THREE.CylinderGeometry(rad + 2.1, rad + 2.1, 0.5, 16, 1, true), 0, h - 0.6, 0);
    solid(p);
  }

  // ---------------- screens for rent: steel pylons along the route ----------------
  const N = track.pts.length;
  [0.1, 0.19, 0.31, 0.47, 0.64, 0.77, 0.88].forEach((u, k) => {
    const i = Math.floor(u * N);
    const q = track.pts[i], t = track.tans[i];
    const fl = new THREE.Vector3(t.x, 0, t.z).normalize();
    const right = new THREE.Vector3(-fl.z, 0, fl.x);
    const side = k % 2 ? 1 : -1;
    const c = q.clone().addScaledVector(right, side * 36);
    const sw = 22, sh = sw / 2, y = q.y + 7;
    const nrm = fl.clone().multiplyScalar(-0.8).addScaledVector(right, -side * 0.6).normalize(); // faces the oncoming cars
    const ang = Math.atan2(nrm.x, nrm.z);
    screens.push({ x: c.x, y, z: c.z, ang, w: sw });
    at({ pos: [c.x, 0, c.z], yaw: ang }, ang);
    const g0 = ground(c.x, c.z) - 4, legH = y - sh / 2 - 1 - g0;
    for (const sx of [-1, 1]) {
      push('steel', box(1.4, legH, 1.4), sx * sw * 0.32, g0 + legH / 2, -1.8);
      colliders.add(new OBB(...toWorld(sx * sw * 0.32, g0 + legH / 2, -1.8).toArray(), 0.7, legH / 2, 0.7, ang));
    }
    for (let yy = g0 + 10; yy < y - sh / 2 - 4; yy += 12) {
      const b = box(sw * 0.64 * 1.45, 0.45, 0.45);
      b.rotateZ(Math.atan2(12, sw * 0.64) * ((Math.round(yy) % 2) ? 1 : -1));
      push('steel', b, 0, yy + 6, -1.8);
    }
    push('steel', box(sw + 3, 1, 3), 0, y - sh / 2 - 1.6, -0.6);
    push('cyan', box(0.5, 3, 0.5), sw * 0.32, y + sh / 2 + 2.6, -1.8);
    colliders.add(new OBB(c.x, y, c.z, sw / 2 + 1.5, sh / 2 + 1.5, 1.4, ang));
  });

  // ---------------- canyon waterfalls: ribbons draped over the walls ----------------
  const up = new THREE.Vector3(0, 1, 0);
  for (let k = 0; k < 11; k++) {
    const u = (k + 0.3 + r() * 0.4) / 11;
    const i = Math.floor(u * N);
    const q = track.pts[i], t = track.tans[i];
    const fl = new THREE.Vector3(t.x, 0, t.z).normalize();
    const out = new THREE.Vector3().crossVectors(fl, up).multiplyScalar(r() < 0.5 ? 1 : -1);
    const ys = [];
    for (let s = 50; s <= 270; s += 5) ys.push([s, ground(q.x + out.x * s, q.z + out.z * s)]);
    const hTop = Math.max(...ys.map((v) => v[1]));
    if (hTop < W + 45) continue;
    const top = ys.find((v) => v[1] >= hTop * 0.9)[0] + 6;
    const bot = ys.filter((v) => v[0] < top && v[1] < W + 1).pop()?.[0] ?? top - 90;
    const pts = [];
    for (let k2 = 0; k2 <= 26; k2++) {
      const s = top - (top - bot + 8) * (k2 / 26);
      const x = q.x + out.x * s, z = q.z + out.z * s;
      pts.push(new THREE.Vector3(x, Math.max(W, ground(x, z)) + 1.6, z));
    }
    falls.push({ pts, w: 16 + r() * 22, side: fl.clone() });
  }
  const fallGeo = [];
  for (const f of falls) {
    const pos = [], fa = [], uvs = [], idx = [];
    let along = 0;
    f.pts.forEach((p, k) => {
      if (k) along += p.distanceTo(f.pts[k - 1]);
      for (const s of [0, 1]) {
        const o = (s - 0.5) * f.w * (1 + 0.25 * (k / f.pts.length));
        pos.push(p.x + f.side.x * o, p.y + f.side.y * o, p.z + f.side.z * o);
        fa.push(s, k / (f.pts.length - 1));
        uvs.push((s - 0.5) * f.w / 3, along / 14);
      }
      if (k) { const a = (k - 1) * 2; idx.push(a, a + 1, a + 2, a + 1, a + 3, a + 2); }
    });
    const g = new THREE.BufferGeometry();
    g.setAttribute('position', new THREE.Float32BufferAttribute(pos, 3));
    g.setAttribute('aF', new THREE.Float32BufferAttribute(fa, 2));
    g.setAttribute('uv', new THREE.Float32BufferAttribute(uvs, 2));
    g.setIndex(idx);
    fallGeo.push(g.toNonIndexed());
    // white water where it lands
    const foot = f.pts[f.pts.length - 1];
    const fg = new THREE.CircleGeometry(f.w * 0.75, 20).rotateX(-Math.PI / 2);
    const n = fg.attributes.position.count;
    fg.setAttribute('aF', new THREE.Float32BufferAttribute(new Array(n).fill(0).flatMap(() => [0.5, 0.97]), 2));
    const fu = fg.attributes.uv;
    for (let v = 0; v < n; v++) fu.setXY(v, fu.getX(v) * f.w / 3, fu.getY(v) * 4);
    fg.translate(foot.x, W + 0.4, foot.z);
    fallGeo.push(fg.toNonIndexed());
  }
  const fallMat = new THREE.ShaderMaterial({
    uniforms: THREE.UniformsUtils.merge([THREE.UniformsLib.fog, { uTime: { value: 0 } }]),
    vertexShader: /* glsl */ `
      attribute vec2 aF;
      varying vec2 vF;
      varying vec2 vUv;
      #include <fog_pars_vertex>
      void main() {
        vF = aF; vUv = uv;
        vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
        gl_Position = projectionMatrix * mvPosition;
        #include <fog_vertex>
      }`,
    fragmentShader: /* glsl */ `
      uniform float uTime;
      varying vec2 vF;
      varying vec2 vUv;
      #include <fog_pars_fragment>
      float h(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
      float n(vec2 p) { vec2 i = floor(p), f = fract(p); vec2 u = f * f * (3. - 2. * f);
        return mix(mix(h(i), h(i + vec2(1, 0)), u.x), mix(h(i + vec2(0, 1)), h(i + vec2(1, 1)), u.x), u.y); }
      void main() {
        vec2 p = vec2(vUv.x * 1.3, vUv.y - uTime * 1.7);
        float s = n(p * vec2(1.0, 0.35)) * 0.55 + n(p * vec2(2.6, 0.9) + 7.) * 0.3 + n(p * vec2(6., 2.) + 3.) * 0.15;
        float foam = smoothstep(0.82, 1.0, vF.y);
        vec3 col = mix(vec3(0.58, 0.68, 0.7), vec3(0.96, 0.98, 1.0), smoothstep(0.35, 0.7, s) + foam * 0.5);
        float a = (0.35 + 0.65 * smoothstep(0.25, 0.6, s)) * (0.8 + foam * 0.2);
        a *= smoothstep(0.0, 0.2, vF.x) * smoothstep(1.0, 0.8, vF.x) * smoothstep(0.0, 0.05, vF.y);
        gl_FragColor = vec4(col, a * 0.92);
        #include <tonemapping_fragment>
        #include <colorspace_fragment>
        #include <fog_fragment>
      }`,
    transparent: true, depthWrite: false, side: THREE.DoubleSide, fog: true,
  });
  if (fallGeo.length) {
    const fm = new THREE.Mesh(mergeGeometries(fallGeo.map((g) => { g.deleteAttribute('normal'); return g; }), false), fallMat);
    fm.renderOrder = 2;
    group.add(fm);
  }

  // ---------------- jungle: instanced trees and palms ----------------
  for (let k = 0; k < 9000 && trees.length < 2600; k++) {
    const x = near.x0 + r() * (near.nx - 1) * near.step, z = near.z0 + r() * (near.nz - 1) * near.step;
    const y = ground(x, z);
    if (y < W + 10) continue;
    const sl = Math.hypot(ground(x + 3, z) - y, ground(x, z + 3) - y) / 3;
    if (sl > (r() < 0.25 ? 1.4 : 0.5)) continue; // a few cling to the walls
    // never into the route: keep the crowns well below it nearby
    const [d, ry] = nearRoute(x, z);
    if (d < 30 && y + 30 > ry - 12) continue;
    trees.push({ x, y: y - 1, z, s: 0.7 + r() * 0.6, palm: r() < 0.3 });
  }
  const treeMat = new THREE.MeshStandardMaterial({ vertexColors: true, roughness: 0.85, flatShading: true });
  const paint = (g, c) => {
    const n = g.attributes.position.count, col = new Float32Array(n * 3);
    const cc = new THREE.Color(c);
    for (let v = 0; v < n; v++) col.set([cc.r, cc.g, cc.b], v * 3);
    g.setAttribute('color', new THREE.BufferAttribute(col, 3));
    return g.index ? g.toNonIndexed() : g;
  };
  const broad = mergeGeometries([
    paint(new THREE.CylinderGeometry(0.7, 1.2, 14, 5).translate(0, 7, 0), 0x4a3b2c),
    paint(new THREE.IcosahedronGeometry(6, 0).scale(1, 0.7, 1).translate(0, 16, 0), 0x2f5e26),
    paint(new THREE.IcosahedronGeometry(4.6, 0).scale(1, 0.75, 1).translate(3.6, 19, 1.5), 0x3d7330),
    paint(new THREE.IcosahedronGeometry(4.2, 0).scale(1, 0.75, 1).translate(-3, 18.5, -2.4), 0x28521f),
  ]);
  const palmParts = [paint(new THREE.CylinderGeometry(0.4, 0.65, 18, 5).translate(0, 9, 0), 0x6b5a45)];
  for (let k = 0; k < 7; k++) {
    const leaf = new THREE.ConeGeometry(1.3, 9, 3).scale(1, 1, 0.25).translate(0, 4.5, 0).rotateX(Math.PI / 2 + 0.55).rotateY((k / 7) * Math.PI * 2).translate(0, 18, 0);
    palmParts.push(paint(leaf, k % 2 ? 0x3f7a2e : 0x356a28));
  }
  const palm = mergeGeometries(palmParts);
  for (const [geo, isPalm] of [[broad, false], [palm, true]]) {
    const list = trees.filter((t) => t.palm === isPalm);
    const im = new THREE.InstancedMesh(geo, treeMat, list.length);
    const m4 = new THREE.Matrix4(), q4 = new THREE.Quaternion(), c4 = new THREE.Color();
    list.forEach((t, k) => {
      q4.setFromEuler(new THREE.Euler((r() - 0.5) * 0.12, r() * 6.28, (r() - 0.5) * 0.12));
      m4.compose(new THREE.Vector3(t.x, t.y, t.z), q4, new THREE.Vector3(t.s, t.s * (0.85 + r() * 0.3), t.s));
      im.setMatrixAt(k, m4);
      im.setColorAt(k, c4.setHSL(0, 0, 0.75 + r() * 0.4));
    });
    im.computeBoundingSphere();
    group.add(im);
  }
  // boulders in the river
  for (let k = 0, n = 0; k < 6000 && n < 140; k++) {
    const x = near.x0 + r() * (near.nx - 1) * near.step, z = near.z0 + r() * (near.nz - 1) * near.step;
    if (ground(x, z) > W - 1) continue;
    const s = 2 + r() * 6;
    at({ pos: [x, W - s * 0.3, z], yaw: r() * 6 });
    push(r() < 0.6 ? 'rock' : 'moss', lump(s, n++, 0));
  }

  // ---------------- materials and meshes ----------------
  const mats = {
    basalt: new THREE.MeshStandardMaterial({ color: 0x3a3e3a, roughness: 0.86, flatShading: true, map: grain }),
    rock: new THREE.MeshStandardMaterial({ color: 0x4a4e47, roughness: 0.9, flatShading: true, map: grain }),
    moss: new THREE.MeshStandardMaterial({ color: 0x3f6230, roughness: 0.95, flatShading: true }),
    concrete: new THREE.MeshStandardMaterial({ color: 0x8d918c, roughness: 0.78, map: grain }),
    steel: new THREE.MeshStandardMaterial({ color: 0x8c96a0, roughness: 0.35, metalness: 0.85 }),
    glass: new THREE.MeshStandardMaterial({ color: 0x9fc0cc, roughness: 0.05, metalness: 0.9, transparent: true, opacity: 0.45 }),
    cyan: new THREE.MeshBasicMaterial({ color: hdr(CYAN, 3) }),
    warm: new THREE.MeshBasicMaterial({ color: hdr(new THREE.Color(0xffc27a), 1.6) }),
    vine: new THREE.MeshStandardMaterial({ color: 0x35602a, roughness: 0.9 }),
  };
  for (const [k, list] of Object.entries(lists)) {
    if (!list.length) continue;
    const g = mergeGeometries(list.map((x) => { x.deleteAttribute('uv'); return x; }), false);
    if (mats[k].map) { // world-space grain for the stone
      const p = g.attributes.position, nr = g.attributes.normal, uv = new Float32Array(p.count * 2);
      for (let v = 0; v < p.count; v++) {
        const ax = Math.abs(nr.getX(v)) > Math.abs(nr.getZ(v));
        uv[v * 2] = (ax ? p.getZ(v) : p.getX(v)) / 30;
        uv[v * 2 + 1] = (Math.abs(nr.getY(v)) > 0.7 ? p.getZ(v) : p.getY(v)) / 30;
      }
      g.setAttribute('uv', new THREE.BufferAttribute(uv, 2));
    }
    g.computeBoundingSphere();
    const mesh = new THREE.Mesh(g, mats[k]);
    mesh.matrixAutoUpdate = false;
    group.add(mesh);
  }

  return {
    towers: [],
    screens,
    adBase: W,
    ground,
    update(t) {
      fallMat.uniforms.uTime.value = t;
    },
  };
}

// ---------------- value noise ----------------
function hash(x, y) {
  let h = Math.imul(x, 374761393) + Math.imul(y, 668265263);
  h = Math.imul(h ^ (h >>> 13), 1274126177);
  return ((h ^ (h >>> 16)) >>> 0) / 4294967296;
}
function vnoise(x, y) {
  const xi = Math.floor(x), yi = Math.floor(y), xf = x - xi, yf = y - yi;
  const u = xf * xf * (3 - 2 * xf), v = yf * yf * (3 - 2 * yf);
  const a = hash(xi, yi), b = hash(xi + 1, yi), c = hash(xi, yi + 1), d = hash(xi + 1, yi + 1);
  return a + (b - a) * u + (c - a) * v + (a - b - c + d) * u * v;
}
function fbm(x, y, oct = 5) {
  let s = 0, a = 0.5;
  for (let i = 0; i < oct; i++) {
    s += a * vnoise(x, y);
    x = x * 2.03 + 17.1;
    y = y * 2.03 - 9.7;
    a *= 0.5;
  }
  return s;
}
