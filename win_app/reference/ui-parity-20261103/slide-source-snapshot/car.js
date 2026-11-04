import * as THREE from 'three';
import { KillerAura } from './killer.js';
import { GLTFLoader } from 'three/addons/loaders/GLTFLoader.js';
import { Prefab, box, cbox, cyl, octo, torus, xform } from './geo.js';
import { CYAN, MAGENTA, hdr } from './materials.js';
import { Ribbon, axisBillboard } from './fx.js';
import { NitroFX, flameMaterial } from './nitro.js';

const PI = Math.PI;
export const CAR = { MAX: 95, BOOST_MAX: 138, ACCEL: 36, BOOST_ACCEL: 75, RADIUS: 2.1 };
/** Sideways slide (A / D with the mouse, v0.2.13): top side speed (m/s, ~90 km/h) and how fast it follows the keys (1/s). */
export const STRAFE = { SPEED: 25, RESPONSE: 6, LEAN: (42 * Math.PI) / 180 }; // LEAN: body roll at a full slide (rad)
/** Spin dodge (F): a barrel roll with a sideways hop; shots can't hurt you during the roll, then a speed surge. */
export const SPIN = { ROLL: 0.55, SURGE: 2, MUL: 1.5, JINK: 22, COOLDOWN: 6 };
const POD_POS = [[-1.85, -1.55], [1.85, -1.55], [-1.85, 1.6], [1.85, 1.6]];
/** Pulse cannon under the nose (body space, before the body's -0.6 drop). */
const CANNON_POS = [0, 0.2, -2.3];

// ---------------- model ----------------
// Built from the reference art in Assets/01_Vehicle: a faceted black supercar with brushed-steel armour, four turbine
// thruster pods, a pulse cannon under the nose and magenta/cyan light lines. Car space: nose towards -Z, y up.

/**
 * Faceted hull lofted through cross-sections. Each station is [z, rightSidePoints] with points listed bottom to top;
 * the left side is mirrored. Flat normals give the sharp panel look of the reference.
 */
function loft(stations) {
  const rings = stations.map(([z, pts]) => {
    const ring = [...pts, ...pts.slice().reverse().map(([x, y]) => [-x, y])];
    return ring.map(([x, y]) => new THREE.Vector3(x, y, z));
  });
  const pos = [];
  const tri = (a, b, c) => pos.push(a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z);
  const n = rings[0].length;
  for (let s = 0; s < rings.length - 1; s++) {
    const A = rings[s], B = rings[s + 1];
    for (let i = 0; i < n; i++) {
      const j = (i + 1) % n;
      tri(A[i], B[j], B[i]);
      tri(A[i], A[j], B[j]);
    }
  }
  const cap = (R, flip) => {
    const c = R.reduce((a, v) => a.add(v), new THREE.Vector3()).divideScalar(R.length);
    for (let i = 0; i < n; i++) {
      const j = (i + 1) % n;
      if (flip) tri(c, R[j], R[i]);
      else tri(c, R[i], R[j]);
    }
  };
  cap(rings[0], true);
  cap(rings[rings.length - 1], false);
  const g = new THREE.BufferGeometry();
  g.setAttribute('position', new THREE.Float32BufferAttribute(pos, 3));
  g.computeVertexNormals();
  return g;
}

const _beam = new THREE.Object3D();
/** A w×h bar running from a to b: light strips, struts and trim lines. */
function beam(P, a, b, w, h, mat) {
  const A = new THREE.Vector3(...a), B = new THREE.Vector3(...b);
  _beam.position.copy(A).add(B).multiplyScalar(0.5);
  _beam.up.set(0, 1, 0);
  _beam.lookAt(B);
  _beam.updateMatrix();
  P.add(box(w, h, A.distanceTo(B)), mat, _beam.matrix.clone());
}
/** Mirrored pair of beams (x of both ends negated for the left side). */
function beams(P, a, b, w, h, mat) {
  beam(P, a, b, w, h, mat);
  beam(P, [-a[0], a[1], a[2]], [-b[0], b[1], b[2]], w, h, mat);
}
const ring = (r0, r1, seg = 32) => new THREE.RingGeometry(r0, r1, seg);

const HULL = [
  [-2.92, [[0.48, 0.2], [0.6, 0.3], [0.54, 0.4], [0.3, 0.44]]],
  [-2.55, [[0.8, 0.14], [0.98, 0.32], [0.9, 0.52], [0.42, 0.6]]],
  [-1.9, [[0.92, 0.12], [1.08, 0.36], [1.0, 0.66], [0.46, 0.76]]],
  [-1.1, [[0.95, 0.12], [1.12, 0.38], [1.04, 0.76], [0.5, 0.86]]],
  [-0.55, [[0.95, 0.12], [1.13, 0.4], [1.05, 0.82], [0.52, 0.9]]],
  [0.7, [[0.95, 0.12], [1.12, 0.4], [1.05, 0.84], [0.58, 0.9]]],
  [1.7, [[0.95, 0.13], [1.13, 0.42], [1.06, 0.86], [0.62, 0.92]]],
  [2.4, [[0.88, 0.18], [1.04, 0.42], [0.98, 0.82], [0.6, 0.86]]],
  [2.7, [[0.8, 0.24], [0.94, 0.42], [0.9, 0.74], [0.56, 0.78]]],
];
const CABIN = [
  [-0.78, [[0.9, 0.86], [0.42, 0.91]]],
  [-0.12, [[0.92, 0.86], [0.6, 1.32]]],
  [0.55, [[0.93, 0.88], [0.6, 1.41]]],
  [1.3, [[0.93, 0.9], [0.57, 1.23]]],
  [2.1, [[0.86, 0.9], [0.5, 0.94]]],
];
const SILL = [
  [-2.62, [[0.8, 0.15], [0.99, 0.33]]],
  [-1.9, [[0.93, 0.14], [1.1, 0.37]]],
  [1.7, [[0.96, 0.15], [1.15, 0.43]]],
  [2.55, [[0.86, 0.2], [1.02, 0.43]]],
];
const ROOF = [
  [-0.1, [[0.62, 1.31], [0.5, 1.35]]],
  [0.55, [[0.62, 1.41], [0.5, 1.45]]],
  [1.28, [[0.59, 1.23], [0.47, 1.27]]],
];

function bodyPrefab(M) {
  const P = new Prefab();
  P.add(loft(HULL), M.carPaint);
  P.add(loft(CABIN), M.carGlass);
  P.add(loft(ROOF), M.carPaint);
  // brushed-steel lower body band, like the silver sills and bumper corners of the reference
  P.add(loft(SILL), M.steel);

  // cockpit (seen through the tinted glass)
  for (const s of [-1, 1]) {
    P.add(cbox(0.42, 0.5, 0.46, 0.08), M.vent, [s * 0.32, 1.02, 0.7], [-0.25, 0, 0]);
    beam(P, [s * 0.32 - 0.17, 0.96, 0.92], [s * 0.32 - 0.17, 1.24, 0.82], 0.02, 0.02, M.neonMSoft);
    beam(P, [s * 0.32 + 0.17, 0.96, 0.92], [s * 0.32 + 0.17, 1.24, 0.82], 0.02, 0.02, M.neonMSoft);
  }
  P.add(cbox(1.5, 0.12, 0.3, 0.04), M.vent, [0, 0.96, -0.3]);
  beam(P, [-0.7, 1.03, -0.42], [0.7, 1.03, -0.42], 0.02, 0.02, M.neonMSoft);
  P.add(torus(0.13, 0.02, 6, 20), M.carGun, [-0.32, 1.08, -0.05], [0.4, 0, 0]);

  // roof: magenta light bar and a centre ridge
  P.add(cbox(0.56, 0.06, 0.14, 0.02), M.steel, [0, 1.46, 0.3]);
  P.add(box(0.46, 0.04, 0.08), M.neonM, [0, 1.49, 0.3]);
  beam(P, [0, 1.44, 0.5], [0, 1.28, 1.25], 0.12, 0.04, M.steel);

  // ---- front ----
  // splitter with a magenta lip
  P.add(cbox(1.95, 0.05, 0.36, 0.02), M.steel, [0, 0.1, -2.68]);
  P.add(box(1.5, 0.03, 0.03), M.neonM, [0, 0.1, -2.87]);
  // chevron grille outline running from the headlights down to the cannon
  beams(P, [0.88, 0.5, -2.45], [0.3, 0.17, -2.86], 0.035, 0.035, M.neonM);
  beams(P, [0.3, 0.17, -2.86], [0.05, 0.12, -2.88], 0.035, 0.035, M.neonM);
  for (const s of [-1, 1]) {
    // intakes either side of the cannon
    P.add(box(0.36, 0.17, 0.06), M.vent, [s * 0.52, 0.27, -2.7], [0, s * 0.35, 0]);
    beam(P, [s * 0.36, 0.36, -2.76], [s * 0.72, 0.36, -2.58], 0.025, 0.025, M.steel);
    beam(P, [s * 0.36, 0.18, -2.76], [s * 0.72, 0.18, -2.58], 0.025, 0.025, M.steel);
    // angular cyan headlights
    beam(P, [s * 0.98, 0.52, -2.18], [s * 0.5, 0.6, -2.6], 0.11, 0.035, M.neonC);
    beam(P, [s * 0.92, 0.42, -2.34], [s * 0.66, 0.47, -2.52], 0.05, 0.03, M.neonC);
    P.add(box(0.05, 0.26, 0.05), M.neonC, [s * 0.99, 0.3, -2.43]);
    // cyan edge running up the front fender
    beam(P, [s * 1.02, 0.62, -1.95], [s * 0.98, 0.72, -1.1], 0.03, 0.03, M.neonC);
    // steel brow over the headlight
    beam(P, [s * 1.02, 0.6, -2.1], [s * 0.45, 0.66, -2.56], 0.16, 0.04, M.steel);
  }

  // ---- hood ----
  P.add(box(0.52, 0.03, 0.36), M.vent, [0, 0.74, -2.05], [-0.16, 0, 0]);
  beam(P, [-0.27, 0.76, -1.88], [0.27, 0.76, -1.88], 0.03, 0.03, M.neonM);
  beam(P, [-0.27, 0.71, -2.22], [0.27, 0.71, -2.22], 0.03, 0.03, M.neonM);
  beams(P, [0.27, 0.71, -2.22], [0.27, 0.76, -1.88], 0.03, 0.03, M.neonM);
  beams(P, [0.36, 0.88, -0.8], [0.3, 0.62, -2.42], 0.14, 0.03, M.steel);
  P.add(cbox(0.7, 0.1, 0.4, 0.04), M.carPaint, [0, 0.92, -0.95]);
  P.add(box(0.5, 0.03, 0.05), M.vent, [0, 0.96, -1.13]);

  // ---- sides ----
  for (const s of [-1, 1]) {
    P.add(box(0.04, 0.04, 3.4), M.neonM, [s * 1.15, 0.12, 0.05]);
    beam(P, [s * 1.1, 0.6, -1.5], [s * 1.12, 0.6, 1.55], 0.03, 0.035, M.neonM);
    // side intake behind the door
    P.add(cbox(0.08, 0.3, 0.62, 0.03), M.steel, [s * 1.13, 0.48, 1.35]);
    P.add(box(0.06, 0.2, 0.48), M.vent, [s * 1.17, 0.48, 1.35]);
    P.add(box(0.03, 0.18, 0.03), M.neonC, [s * 1.18, 0.48, 1.08]);
    // cyan flash on the rear haunch
    beam(P, [s * 1.07, 0.8, 1.85], [s * 1.0, 0.62, 2.5], 0.035, 0.035, M.neonC);
    // mirror
    beam(P, [s * 0.9, 0.95, -0.6], [s * 1.08, 1.02, -0.62], 0.04, 0.04, M.steel);
    P.add(cbox(0.26, 0.13, 0.18, 0.04), M.carPaint, [s * 1.16, 1.04, -0.62]);
    P.add(box(0.2, 0.02, 0.02), M.neonM, [s * 1.16, 1.11, -0.7]);
  }

  // ---- rear ----
  P.add(box(1.6, 0.05, 0.04), M.neonM, [0, 0.7, 2.72]);
  for (const s of [-1, 1]) {
    beam(P, [s * 0.45, 0.6, 2.73], [s * 0.86, 0.66, 2.62], 0.05, 0.12, M.neonM);
    // twin exhaust thrusters
    P.add(cyl(0.13, 0.15, 0.2, 16), M.steel, [s * 0.36, 0.3, 2.72], [PI / 2, 0, 0]);
    P.add(cyl(0.09, 0.09, 0.02, 16), M.neonC, [s * 0.36, 0.3, 2.83], [PI / 2, 0, 0]);
  }
  P.add(box(0.5, 0.14, 0.06), M.vent, [0, 0.3, 2.74]);
  for (const x of [-0.18, 0, 0.18]) P.add(box(0.03, 0.16, 0.24), M.steel, [x, 0.22, 2.68]);

  // rear wing on two struts
  beams(P, [0.5, 0.84, 2.2], [0.6, 1.48, 2.5], 0.07, 0.16, M.steel);
  P.add(cbox(2.3, 0.07, 0.56, 0.025), M.carPaint, [0, 1.52, 2.5], [-0.12, 0, 0]);
  P.add(box(2.16, 0.03, 0.04), M.neonM, [0, 1.52, 2.79]);
  for (const s of [-1, 1]) P.add(cbox(0.06, 0.34, 0.64, 0.02), M.steel, [s * 1.17, 1.48, 2.5]);
  P.add(cbox(0.06, 0.22, 0.5, 0.02), M.steel, [0, 1.62, 2.42]);
  P.add(box(0.02, 0.03, 0.4), M.neonM, [0, 1.74, 2.42]);

  // pod arms
  for (const z of [-1.55, 1.6]) {
    for (const s of [-1, 1]) {
      P.add(cbox(0.75, 0.26, 0.56, 0.06), M.carGun, [s * 1.38, 0.45, z]);
      P.add(box(0.62, 0.03, 0.03), M.neonC, [s * 1.38, 0.59, z - 0.2]);
    }
  }
  return P;
}

/** Pulse cannon module (reference 03), slung under the nose with the muzzle forward (-Z). */
function cannonPrefab(M) {
  const P = new Prefab();
  P.add(octo(0.19, 0.19, 1.3), M.carPaint, [0, 0, 0.05], [PI / 2, 0, 0]);
  for (const z of [-0.2, -0.38]) P.add(cyl(0.22, 0.22, 0.07, 20), M.steel, [0, 0, z], [PI / 2, 0, 0]);
  P.add(cyl(0.23, 0.25, 0.22, 24), M.steel, [0, 0, -0.62], [PI / 2, 0, 0]);
  P.add(cyl(0.18, 0.18, 0.04, 24), M.vent, [0, 0, -0.74], [PI / 2, 0, 0]);
  P.add(torus(0.2, 0.035, 8, 32), M.neonM, [0, 0, -0.74]);
  P.add(torus(0.12, 0.02, 6, 24), M.neonM, [0, 0, -0.75]);
  P.add(cyl(0.05, 0.05, 0.02, 16), M.neonMSoft, [0, 0, -0.76], [PI / 2, 0, 0]);
  for (let i = 0; i < 6; i++) {
    const a = (i / 6) * PI * 2;
    P.add(box(0.025, 0.06, 0.03), M.neonM, [Math.cos(a) * 0.24, Math.sin(a) * 0.24, -0.62], [0, 0, a]);
  }
  // breech block with a cyan cell and hex vents
  P.add(cbox(0.46, 0.34, 0.42, 0.06), M.carGun, [0, 0.02, 0.62]);
  for (const s of [-1, 1]) {
    P.add(box(0.03, 0.2, 0.05), M.neonC, [s * 0.24, 0.02, 0.56]);
    P.add(box(0.02, 0.14, 0.18), M.vent, [s * 0.235, 0.02, 0.74]);
    P.add(box(0.025, 0.025, 0.7), M.neonM, [s * 0.18, 0.06, -0.05]);
  }
  // top fin
  P.add(cbox(0.1, 0.12, 0.55, 0.03), M.steel, [0, 0.21, 0.05]);
  P.add(box(0.03, 0.03, 0.42), M.neonM, [0, 0.28, 0.05]);
  return P;
}

const FAN_X = 0.42;
/** The spinning turbine fan inside a pod (separate so the player's car can spin it). */
function fanPrefab(M, side) {
  const P = new Prefab();
  const blade = box(0.025, 0.34, 0.09);
  for (let i = 0; i < 12; i++) {
    // radial blade at the fan face, twisted around its own axis, then turned to its slot
    const m = new THREE.Matrix4().makeRotationX((i / 12) * PI * 2)
      .multiply(new THREE.Matrix4().makeTranslation(side * FAN_X, 0.32, 0))
      .multiply(new THREE.Matrix4().makeRotationY(0.45 * side));
    P.add(blade, M.fan, m);
  }
  P.add(cyl(0.5, 0.5, 0.01, 32), M.vent, [side * (FAN_X - 0.04), 0, 0], [0, 0, PI / 2]);
  return P;
}

/** Turbine thruster pod (reference 02): the turbine faces outward like a wheel, armour wraps the top and inside. */
function podPrefab(M, side) {
  const P = new Prefab();
  const X = [0, 0, PI / 2];
  const faceOut = [0, side * PI / 2, 0];
  // drum, rim and recessed turbine well
  P.add(cyl(0.58, 0.58, 0.7, 24), M.carGun, [0, 0, 0], X);
  P.add(cyl(0.7, 0.7, 0.16, 32, true), M.carChrome, [side * 0.38, 0, 0], X);
  P.add(ring(0.55, 0.7), M.carGun, [side * 0.46, 0, 0], faceOut);
  P.add(cyl(0.56, 0.56, 0.04, 24), M.vent, [side * 0.3, 0, 0], X);
  // hub, cyan core and glowing rings
  P.add(cyl(0.15, 0.17, 0.12, 16), M.steel, [side * 0.4, 0, 0], X);
  P.add(cyl(0.1, 0.1, 0.02, 16), M.neonCSoft, [side * 0.47, 0, 0], X);
  P.add(cyl(0.04, 0.04, 0.02, 12), M.neonC, [side * 0.48, 0, 0], X);
  P.add(torus(0.5, 0.045, 8, 40), M.neonM, [side * 0.45, 0, 0], faceOut);
  P.add(torus(0.63, 0.018, 6, 40), M.neonM, [side * 0.47, 0, 0], faceOut);
  P.add(torus(0.2, 0.022, 6, 24), M.neonC, [side * 0.46, 0, 0], faceOut);

  // octagonal armour shell over the inner half, plated on top
  P.add(octo(0.8, 0.8, 0.46), M.steel, [-side * 0.12, 0, 0], X);
  P.add(cbox(0.62, 0.22, 1.36, 0.06), M.carPaint, [-side * 0.04, 0.74, 0.04]);
  P.add(box(0.04, 0.04, 1.04), M.neonC, [side * 0.26, 0.86, 0.02]);
  P.add(box(0.03, 0.06, 0.9), M.neonM, [side * 0.28, 0.72, 0.06]);
  P.add(cbox(0.5, 0.12, 0.5, 0.04), M.steel, [-side * 0.05, 0.9, -0.2]);
  for (const z of [-0.42, 0.5]) P.add(cyl(0.035, 0.035, 0.03, 8), M.carChrome, [side * 0.18, 0.86, z]);
  P.add(box(0.04, 0.48, 0.05), M.neonC, [side * 0.22, 0.2, -0.68]);
  // panelling on the housing's front and back faces
  for (const z of [-1, 1]) {
    P.add(box(0.34, 0.26, 0.03), M.carGun, [-side * 0.12, 0.08, z * 0.75]);
    P.add(box(0.26, 0.16, 0.02), M.vent, [-side * 0.12, 0.08, z * 0.765]);
    P.add(box(0.34, 0.03, 0.03), M.neonM, [-side * 0.12, 0.31, z * 0.75]);
    for (const y of [-0.12, 0.28]) P.add(cyl(0.03, 0.03, 0.02, 8), M.carChrome, [-side * 0.3, y, z * 0.765], [PI / 2, 0, 0]);
  }

  // front-lower vectoring block with a magenta cell (the glowing square in the reference)
  P.add(cbox(0.42, 0.42, 0.44, 0.06), M.carGun, [side * 0.04, -0.44, -0.56]);
  P.add(box(0.26, 0.2, 0.03), M.neonM, [side * 0.04, -0.44, -0.79]);
  P.add(box(0.03, 0.2, 0.26), M.vent, [side * 0.26, -0.44, -0.56]);
  P.add(box(0.03, 0.03, 0.3), M.neonC, [side * 0.26, -0.3, -0.56]);

  // rear nozzle where the thrust flame comes out
  P.add(cyl(0.34, 0.3, 0.3, 24), M.steel, [0, 0, 0.6], [PI / 2, 0, 0]);
  P.add(cyl(0.26, 0.26, 0.02, 20), M.vent, [0, 0, 0.75], [PI / 2, 0, 0]);
  P.add(torus(0.3, 0.03, 6, 32), M.neonM, [0, 0, 0.76]);
  P.add(torus(0.16, 0.03, 6, 24), M.neonC, [0, 0, 0.76]);

  // underglow
  P.add(cyl(0.45, 0.45, 0.05, 20), M.neonCSoft, [0, -0.62, 0]);
  return P;
}

// ---------------- sculpted model ----------------
// public/models/car.glb: the car generated from the reference art (Meshy, front/side/rear views) and prepared in Blender
// by meshy/build_game_car.py: nose towards -Z, 5.6 m long, centred on the origin, two levels of detail sharing one
// texture set ("hi_*" for the player and its close-up chase/pause views, "lo_*" for rivals), each split into a body and
// four thruster pods ("*_pod_FL|FR|RL|RR") pivoting at their centres. Until it has loaded, or if it fails to load, the
// code-built car above is shown instead.
// One entry per car in the shop (see shop.js). Attachment points were measured on each finished model in Blender
// (meshy/car02 for the VIPER), in the car's own space after the `y` drop.
export const CAR_MODELS = {
  roadster: {
    url: 'models/car.glb',
    y: -0.35, // drop to match the hover height of the code-built car
    exhaust: [1.55, -0.54, 2.7], // flame nozzles (x mirrored): the rear thruster pods
    turbine: [0, -0.5, 2.62, 0.34], // main rear turbine nozzle: centre + radius (glowing ring added in code)
    tail: [0.8, 0.1, 2.8], // tail-light ribbon sources (x mirrored)
    podTilt: 0.45, // share of the code-built pods' tilt (the sculpted pods are joined to the body)
  },
  viper: {
    url: 'models/car02.glb',
    y: -0.75, // a lower body (1.17 m tall): this puts its underside level with the ROADSTER's
    exhaust: [0.59, -0.89, 2.72], // the two side nozzles in the tail (its pods' fans face sideways)
    turbine: [0, -0.79, 2.74, 0.28],
    tail: [1.01, -0.2, 2.66], // rear wing tips
    podTilt: 0.8, // its pods are free-standing, so they can tilt further
  },
  titan: {
    url: 'models/car03.glb',
    y: -0.63, // 1.39 m tall: underside level with the ROADSTER's
    exhaust: [0.52, -0.9, 2.62], // the two smaller tail nozzles
    turbine: [0, -0.93, 2.72, 0.24],
    tail: [0.86, 0, 2.42], // rear wing tips
    podTilt: 0.8, // free-standing pods on arms
  },
};
const SCULPT = CAR_MODELS.roadster;

// Neon is lit from the texture itself: strongly saturated texels (the magenta/cyan light lines) become emissive so they
// bloom; the black and steel panels stay purely reflective. Rivals recolour the same texture: neon -> livery accent,
// dark body -> livery paint (steel trim kept on dark liveries).
const NEON_MASK = `
  float tMax = max(tC.r, max(tC.g, tC.b));
  float tSat = tMax > 0.0 ? (tMax - min(tC.r, min(tC.g, tC.b))) / tMax : 0.0;
  float neon = smoothstep(0.45, 0.8, tSat) * smoothstep(0.12, 0.4, tMax);`;

export function sculptShader(m, livery) {
  const uniforms = livery && {
    uPaint: { value: new THREE.Color(livery.paint) },
    uAccent: { value: new THREE.Color(livery.accent) },
  };
  m.customProgramCacheKey = () => (livery ? 'sculpt-livery' : 'sculpt');
  m.onBeforeCompile = (sh) => {
    let fs = sh.fragmentShader;
    if (livery) {
      Object.assign(sh.uniforms, uniforms);
      fs = `uniform vec3 uPaint;\nuniform vec3 uAccent;\n${fs}`;
      fs = fs.replace('#include <map_fragment>', `#include <map_fragment>
        { vec3 tC = diffuseColor.rgb; ${NEON_MASK}
          vec3 body = max(uPaint * (0.6 + 1.5 * tMax), tC * 0.8);
          diffuseColor.rgb = mix(body, uAccent * tMax, neon); }`);
    }
    fs = fs.replace('#include <emissivemap_fragment>', `
      { vec3 tC = texture2D(emissiveMap, vEmissiveMapUv).rgb; ${NEON_MASK}
        totalEmissiveRadiance *= ${livery ? 'uAccent * tMax' : 'tC'} * neon; }`);
    sh.fragmentShader = fs;
  };
}

/**
 * Loads a sculpted car: resolves to { hi, lo } groups (see liveryModel for rivals). `spec` is a CAR_MODELS entry (its
 * attachment points travel with the model, in userData, through clones and liveries).
 */
export function loadCarModel(url, envMap, onProgress, spec = SCULPT) {
  return new GLTFLoader().loadAsync(url, onProgress).then((gltf) => {
    const model = gltf.scene;
    model.traverse((o) => {
      if (!o.isMesh) return;
      // glossy lacquer: the Meshy paint gets a clear coat that catches the city's reflections (reads less flat-black)
      const g = o.material;
      const m = new THREE.MeshPhysicalMaterial({
        map: g.map, normalMap: g.normalMap, normalScale: g.normalScale, metalnessMap: g.metalnessMap, roughnessMap: g.roughnessMap,
        metalness: g.metalness, roughness: g.roughness, clearcoat: 1, clearcoatRoughness: 0.12, side: g.side,
      });
      if (g.normalMap) m.clearcoatNormalScale = new THREE.Vector2(0.15, 0.15);
      g.dispose();
      o.material = m;
      m.envMap = envMap;
      m.envMapIntensity = 1.3;
      m.emissiveMap = m.map;
      m.emissive = new THREE.Color(1, 1, 1);
      m.emissiveIntensity = 3.2;
      sculptShader(m);
      o.castShadow = o.receiveShadow = false;
    });
    const lods = { hi: new THREE.Group(), lo: new THREE.Group() };
    for (const o of [...model.children]) (lods[o.name.split('_')[0]] ?? lods.hi).add(o);
    if (!lods.lo.children.length) lods.lo = lods.hi; // single-detail file
    lods.hi.userData.spec = lods.lo.userData.spec = spec;
    return lods;
  });
}

/** A rival copy of the sculpted car in its livery colours (geometry and textures stay shared). */
export function liveryModel(model, livery) {
  const copy = model.clone();
  const mats = new Map();
  copy.traverse((o) => {
    if (!o.isMesh) return;
    if (!mats.has(o.material)) {
      const m = o.material.clone();
      m.metalness = livery.metal ?? m.metalness;
      m.emissiveIntensity = 2.6;
      sculptShader(m, livery);
      mats.set(o.material, m);
    }
    o.material = mats.get(o.material);
  });
  return copy;
}

const cache = new WeakMap();
/** Car prefabs are built once per material set and shared by every car. */
function prefabs(M) {
  if (!cache.has(M)) {
    const body = bodyPrefab(M).merge(cannonPrefab(M), xform(CANNON_POS));
    const pods = [podPrefab(M, -1), podPrefab(M, 1)];
    const fans = [fanPrefab(M, -1), fanPrefab(M, 1)];
    // rivals: body + pods + fans merged into one static mesh set (fewer draw calls, no pod animation)
    const whole = new Prefab().merge(body, xform([0, -0.6, 0]));
    for (const [x, z] of POD_POS) {
      whole.merge(pods[x > 0 ? 1 : 0], xform([x, -0.15, z]));
      whole.merge(fans[x > 0 ? 1 : 0], xform([x, -0.15, z]));
    }
    cache.set(M, { body, pods, fans, whole });
  }
  return cache.get(M);
}

/** Livery = body paint + accent neon, swapped in by material remapping. */
function liveryRemap(M, livery) {
  const paint = new THREE.MeshPhysicalMaterial({
    color: livery.paint, metalness: livery.metal ?? 0.65, roughness: 0.24, clearcoat: 1, clearcoatRoughness: 0.08, envMap: M.carPaint.envMap,
  });
  const accent = new THREE.MeshBasicMaterial({ color: new THREE.Color(livery.accent).multiplyScalar(3.2) });
  const accentSoft = new THREE.MeshBasicMaterial({ color: new THREE.Color(livery.accent).multiplyScalar(1.8) });
  return new Map([[M.carPaint, paint], [M.neonM, accent], [M.neonMSoft, accentSoft]]);
}

export class Car {
  /** Passing opts.livery makes a rival: merged model, no dynamic lights, no ribbon trails. */
  constructor(scene, M, opts = {}) {
    this.rival = !!opts.livery;
    this.scene = scene;
    this.modelId = 'roadster'; // CAR_MODELS id of the model shown (the code-built car stands in for the ROADSTER)
    this.root = new THREE.Group();
    this.model = new THREE.Group();
    this.root.add(this.model);
    const P = prefabs(M);
    this.pods = [];
    this.fans = [];
    if (this.rival) {
      this.model.add(P.whole.toGroup(liveryRemap(M, opts.livery)));
    } else {
      const body = P.body.toGroup();
      body.position.y = -0.6;
      this.model.add(body);
      for (const [x, z] of POD_POS) {
        const p = P.pods[x > 0 ? 1 : 0].toGroup();
        p.position.set(x, -0.15, z);
        const fan = P.fans[x > 0 ? 1 : 0].toGroup();
        p.add(fan);
        this.fans.push(fan);
        this.model.add(p);
        this.pods.push(p);
      }
      this.glow = new THREE.PointLight(0xff2bd6, 14, 14, 2);
      this.glow.position.set(0, -1.2, 0);
      this.model.add(this.glow);
      this.head = new THREE.PointLight(0xb8c8ff, 22, 14, 2);
      this.head.position.set(0, 3.2, 3.5);
      this.model.add(this.head);
      // soft key light above and behind: the chase camera looks at the rear, which would otherwise sit in shadow
      this.rearFill = new THREE.PointLight(0xd8c8ff, 9, 9, 2);
      this.rearFill.position.set(0, 2.6, 5.2);
      this.model.add(this.rearFill);
    }
    scene.add(this.root);

    // thruster flames: one per nozzle (the two rear pods, plus the main turbine once the sculpted car is in)
    const flameCol = this.rival
      ? new THREE.Color(opts.livery.accent).lerp(new THREE.Color(0x8ad8ff), 0.5).multiplyScalar(0.9)
      : new THREE.Color(0.35, 0.75, 1.5);
    const tipCol = new THREE.Color(this.rival ? opts.livery.accent : MAGENTA).multiplyScalar(1.6);
    this.flames = [0, 1, 2].map((i) => {
      const m = new THREE.Mesh(new THREE.PlaneGeometry(1, 1), flameMaterial(flameCol, tipCol));
      m.matrixAutoUpdate = false;
      m.frustumCulled = false;
      m.visible = i < 2;
      scene.add(m);
      return m;
    });
    this.nozzles = [];
    this.nitro = 0; // smoothed 0..1
    this.kick = 0; // ignition burst, decays
    this.sputter = 0; // flame stutter after nitro runs out
    this.thrust = 0; // smoothed throttle
    this.ignited = this.ended = false; // one-frame events for main.js
    this.nfx = this.rival ? null : new NitroFX(scene, MAGENTA);
    this.podTilt = 1;
    this.exhaust = [1.85, -0.15, 2.1];
    this.tail = [0.9, 0.15, 2.65];
    this.ribbons = this.rival ? [] : [
      new Ribbon(scene, hdr(CYAN, 0.85), 16, 0.36, 0.14), new Ribbon(scene, hdr(CYAN, 0.85), 16, 0.36, 0.14),
      new Ribbon(scene, hdr(MAGENTA, 0.75), 12, 0.15, 0.06), new Ribbon(scene, hdr(MAGENTA, 0.75), 12, 0.15, 0.06),
    ];

    this.pos = new THREE.Vector3();
    this.vel = new THREE.Vector3();
    this.fwd = new THREE.Vector3(0, 0, -1);
    this.yaw = 0;
    this.pitch = 0;
    this.roll = 0;
    this.speed = 0;
    this.side = 0; // sideways slide speed, m/s (+ right)
    this.boost = 1;
    this.boosting = false;
    this.maxMul = 1;
    this.stun = 0;
    this.rollT = 0; // spin dodge: seconds of barrel roll left
    this.rollDir = 1; // +1 rolls to the right, -1 to the left
    this.surge = 0; // spin dodge: seconds of 1.5× speed left
    this.t = Math.random() * 10;
  }

  /** Swaps the code-built body and pods for the sculpted model. */
  useModel(model, accent = MAGENTA) {
    const spec = model.userData.spec ?? SCULPT;
    this.model.clear();
    model.position.y = spec.y;
    this.model.add(model);
    // sculpted pods tilt like the code-built ones (front pair first, left before right); their fans are painted on
    this.pods = model.children.filter((o) => o.name.includes('_pod_')).sort((a, b) => (a.name < b.name ? -1 : 1));
    this.podTilt = spec.podTilt;
    this.fans = [];
    if (this.glow) this.model.add(this.glow);
    if (this.head) this.model.add(this.head);
    if (this.rearFill) this.model.add(this.rearFill);
    // glowing ring + core in the main rear turbine (the chase camera's view), brightening with throttle
    const [tx, ty, tz, tr] = spec.turbine;
    this.nozzleCol = new THREE.Color(accent);
    this.nozzleMat = new THREE.MeshBasicMaterial({ color: hdr(this.nozzleCol, 2.5) });
    const ring = new THREE.Mesh(new THREE.TorusGeometry(tr, tr * 0.14, 8, 40), this.nozzleMat);
    const core = new THREE.Mesh(new THREE.CircleGeometry(tr * 0.42, 24), this.nozzleMat);
    ring.position.set(tx, ty, tz);
    core.position.set(tx, ty, tz - 0.06);
    // turbine blades spinning inside the ring
    this.fan = new THREE.Group();
    for (let i = 0; i < 6; i++) {
      const b = new THREE.Mesh(new THREE.BoxGeometry(tr * 0.16, tr * 0.78, 0.04), this.nozzleMat);
      b.position.y = tr * 0.48;
      b.rotation.y = 0.5;
      const arm = new THREE.Group();
      arm.rotation.z = (i / 6) * Math.PI * 2;
      arm.add(b);
      this.fan.add(arm);
    }
    this.fan.position.set(tx, ty, tz - 0.1);
    this.model.add(ring, core, this.fan);
    this.turbine = [tx, ty, tz + 0.05, 1.05];
    this.flames[2].visible = this.flames[0].visible;
    this.exhaust = spec.exhaust;
    this.tail = spec.tail;
    this.modelId = Object.keys(CAR_MODELS).find((k) => CAR_MODELS[k] === spec) ?? 'roadster';
    this.aura?.attach(); // a Killer keeps their aura on the new car
  }

  /**
   * KILLER STATUS (killer.js): the aura on or off. tier: 'high' | 'low' | 'menu' | 'remote' (how many sparks, whether
   * the trail shows); style: 'crimson' | 'void' (killer.js auraStyleFor: by Killer kill count). A new tier or style rebuilds it. Calling it
   * again with the same values does nothing.
   */
  setKiller(on, tier = this.rival ? 'remote' : 'high', style = 'crimson') {
    if (this.aura && (!on || this.aura.tier !== tier || this.aura.style !== style)) {
      this.aura.dispose();
      this.aura = null;
    }
    if (on && !this.aura) this.aura = new KillerAura(this, this.scene, tier, style);
  }
  get isKiller() {
    return !!this.aura;
  }

  setVisible(v) {
    this.root.visible = v;
    this.flames.forEach((f, i) => { f.visible = v && (i < 2 || !!this.turbine); });
    for (const r of this.ribbons) r.mesh.visible = v;
    this.nfx?.setVisible(v);
  }

  /** Thrust flames and light trails (hidden on the pause showcase, where they'd face the camera). */
  showFX(v) {
    this.flames.forEach((f, i) => { f.visible = v && this.root.visible && (i < 2 || !!this.turbine); });
    for (const r of this.ribbons) r.mesh.visible = v && this.root.visible;
    this.nfx?.setVisible(v && this.root.visible);
  }

  place(pos, yaw, speed = 0) {
    this.pos.copy(pos);
    this.yaw = yaw;
    this.pitch = 0;
    this.roll = 0;
    this.speed = speed;
    this.side = 0;
    this.stun = 0;
    this.rollT = this.surge = 0;
    this.updateForward();
    this.vel.copy(this.fwd).multiplyScalar(speed);
    this.sync();
    for (const r of this.ribbons) r.hist = [];
  }

  /** Spin dodge: barrel roll + sideways hop toward `side` (+1 right, -1 left) + speed × SPIN.MUL for SPIN.SURGE s. */
  dodge(side) {
    this.barrelRoll(side);
    this.surge = SPIN.SURGE;
    this.speed = Math.max(this.speed, CAR.MAX * 0.5) * SPIN.MUL;
    this.vel.x += Math.cos(this.yaw) * side * SPIN.JINK;
    this.vel.z -= Math.sin(this.yaw) * side * SPIN.JINK;
  }
  /** The roll alone (another player's spin, drawn from the network). */
  barrelRoll(side) {
    this.rollT = SPIN.ROLL;
    this.rollDir = side < 0 ? -1 : 1;
  }
  /** Rolling: shots can't hurt this car. */
  get dodging() {
    return this.rollT > 0;
  }

  updateForward() {
    const cp = Math.cos(this.pitch);
    this.fwd.set(-Math.sin(this.yaw) * cp, Math.sin(this.pitch), -Math.cos(this.yaw) * cp);
  }

  sync() {
    this.root.position.copy(this.pos);
    // stun = brief wobble after taking a hit
    // spin dodge: one full turn about the nose, fast in the middle (smoothstep)
    const k = 1 - this.rollT / SPIN.ROLL;
    const spin = this.rollT > 0 ? -this.rollDir * Math.PI * 2 * k * k * (3 - 2 * k) : 0;
    this.root.rotation.set(this.pitch, this.yaw, this.roll + spin + Math.sin(this.t * 26) * this.stun * 0.5, 'YXZ');
    this.root.updateMatrixWorld(true);
  }

  update(dt, inp, colliders) {
    this.t += dt;
    this.stun = Math.max(0, this.stun - dt);
    this.rollT = Math.max(0, this.rollT - dt);
    this.surge = Math.max(0, this.surge - dt);
    const sf = Math.min(1, Math.abs(this.speed) / CAR.MAX);
    // boost
    const was = this.boosting;
    this.boosting = inp.boost && this.boost > 0.02 && inp.throttle > 0 && this.stun <= 0;
    this.ignited = this.boosting && !was;
    this.ended = was && !this.boosting;
    if (this.ignited) this.kick = 1;
    if (this.ended) this.sputter = 0.45;
    this.nitro += ((this.boosting ? 1 : 0) - this.nitro) * (1 - Math.exp(-(this.boosting ? 10 : 3.5) * dt));
    this.kick = Math.max(0, this.kick - dt * 2.4);
    this.sputter = Math.max(0, this.sputter - dt);
    this.thrust += (Math.max(0, inp.throttle) - this.thrust) * (1 - Math.exp(-8 * dt));
    this.fxDt = dt;
    this.boost = THREE.MathUtils.clamp(this.boost + (this.boosting ? -0.32 : 0.06) * dt, 0, 1);
    const max = (this.boosting ? CAR.BOOST_MAX : CAR.MAX) * this.maxMul * (this.surge > 0 ? SPIN.MUL : 1);
    const acc = this.boosting ? CAR.BOOST_ACCEL : CAR.ACCEL;
    if (inp.throttle > 0) this.speed += acc * inp.throttle * dt * Math.max(0, 1 - (this.speed / max) ** 2);
    else {
      // coasting slows the car to a stop and stops there: overshooting zero flipped the speed's sign every frame, and with
      // it the steering (reversing steers the other way), so a stopped car shook instead of turning (fixed in v0.2.13)
      const drag = 9 * dt;
      this.speed = Math.abs(this.speed) <= drag ? 0 : this.speed - drag * Math.sign(this.speed);
    }
    if (inp.brake > 0) this.speed -= (this.speed > 0 ? 70 : 14) * inp.brake * dt;
    if (this.speed > max) this.speed -= 28 * dt;
    this.speed = Math.max(-18, this.speed);

    // steering & pitch
    const turnRate = THREE.MathUtils.lerp(1.9, 1.15, sf) * (inp.brake > 0 ? 1.25 : 1);
    this.yaw -= inp.steer * turnRate * dt * (this.speed < 0 && !inp.aimSteer ? -1 : 1) * Math.min(1, Math.abs(this.speed) / 8 + 0.35);
    const pt = inp.climb * 0.62;
    this.pitch += (pt - this.pitch) * (1 - Math.exp(-3.2 * dt));
    this.updateForward();

    // velocity with a little drift
    const grip = inp.brake > 0 && Math.abs(inp.steer) > 0 ? 1.6 : 3.4;
    // sideways slide: its own speed toward the keys' (not while stunned), added across the nose
    const sideT = (this.stun > 0 ? 0 : inp.strafe || 0) * STRAFE.SPEED;
    this.side += (sideT - this.side) * (1 - Math.exp(-STRAFE.RESPONSE * dt));
    const desired = this.fwd.clone().multiplyScalar(this.speed);
    desired.x += Math.cos(this.yaw) * this.side;
    desired.z -= Math.sin(this.yaw) * this.side;
    this.vel.lerp(desired, 1 - Math.exp(-grip * dt));
    this.pos.addScaledVector(this.vel, dt);

    let impact = 0;
    const hit = colliders.collide(this.pos, CAR.RADIUS);
    if (hit) {
      const vn = this.vel.dot(hit.n);
      if (vn < 0) {
        this.vel.addScaledVector(hit.n, -vn * 1.4);
        impact = -vn;
        if (impact > 12) this.speed *= 0.7;
        this.speed = Math.min(this.speed, Math.max(0, this.vel.dot(this.fwd)) + 6);
      }
    }

    // visual attitude
    const slip = this.vel.clone().sub(desired).dot(new THREE.Vector3(Math.cos(this.yaw), 0, -Math.sin(this.yaw)));
    const rollT = -inp.steer * 0.62 * Math.min(1, Math.abs(this.speed) / 35) - slip * 0.01 - (this.side / STRAFE.SPEED) * STRAFE.LEAN; // lean into a slide
    this.roll += (rollT - this.roll) * (1 - Math.exp(-5 * dt));
    this.sync();
    this.model.position.y = Math.sin(this.t * 2.3) * 0.12 * (1 - sf * 0.7);
    // nitro: nose lifts a touch, with an extra jolt on ignition
    this.model.rotation.x = this.nitro * 0.03 + this.kick * this.kick * 0.07;
    if (this.fan) this.fan.rotation.z -= dt * (4 + 26 * this.thrust + 40 * this.nitro);
    const tilt = -0.35 * inp.throttle * (this.boosting ? 1.6 : 1) + 0.3 * inp.brake;
    this.pods.forEach((p, i) => {
      const k = this.podTilt;
      p.rotation.x += ((tilt + (i < 2 ? -0.1 : 0) * inp.climb) * k - p.rotation.x) * (1 - Math.exp(-6 * dt));
      // steering turns the pods like wheels; a slide tilts them all toward it
      p.rotation.z = ((i % 2 ? 1 : -1) * inp.steer * 0.15 - (this.side / STRAFE.SPEED) * 0.3) * k;
    });
    if (this.glow) this.glow.intensity = 12 + 4 * Math.sin(this.t * 9) + (this.boosting ? 10 : 0);
    if (this.nozzleMat) {
      this.nozzleMat.color.copy(this.nozzleCol).multiplyScalar(1.6 + 2.2 * this.thrust + 2.5 * this.nitro + 4 * this.kick);
      this.nozzleMat.color.lerp(new THREE.Color(3, 3, 3.4), this.kick * 0.6); // white-hot flash on ignition
    }
    this.spin(dt, 6 + sf * 30 + (this.boosting ? 20 : 0));
    if (this.head) this.head.intensity = 22;
    return { impact, n: hit && hit.n };
  }

  /** Turbine fans spin with speed; also called while paused so the car idles on the showcase. */
  spin(dt, rate = 4) {
    for (const f of this.fans) f.rotation.x += rate * dt;
  }

  /** Another player's car (FREE ROAM): pose from the network, d = [x, y, z, yaw, pitch, roll, speed, thrust, nitro]. */
  follow(d, dt) {
    this.t += dt;
    this.rollT = Math.max(0, this.rollT - dt);
    const [x, y, z, yaw, pitch, roll, speed, thrust, nitro] = d;
    if (dt > 0) this.vel.set(x - this.pos.x, y - this.pos.y, z - this.pos.z).divideScalar(dt);
    if (this.vel.lengthSq() > 600 * 600) this.vel.set(0, 0, 0); // a teleport (respawn) is not a speed
    this.pos.set(x, y, z);
    Object.assign(this, { yaw, pitch, roll, speed, thrust });
    const was = this.boosting;
    this.boosting = nitro > 0.5;
    this.ignited = this.boosting && !was;
    this.ended = was && !this.boosting;
    if (this.ignited) this.kick = 1;
    if (this.ended) this.sputter = 0.45;
    this.nitro = nitro;
    this.kick = Math.max(0, this.kick - dt * 2.4);
    this.sputter = Math.max(0, this.sputter - dt);
    this.fxDt = dt;
    this.updateForward();
    this.sync();
    const sf = Math.min(1, Math.abs(speed) / CAR.MAX);
    this.model.position.y = Math.sin(this.t * 2.3) * 0.12 * (1 - sf * 0.7);
    this.model.rotation.x = nitro * 0.03 + this.kick * this.kick * 0.07;
    if (this.fan) this.fan.rotation.z -= dt * (4 + 26 * thrust + 40 * nitro);
    if (this.nozzleMat) this.nozzleMat.color.copy(this.nozzleCol).multiplyScalar(1.6 + 2.2 * thrust + 2.5 * nitro + 4 * this.kick);
    this.spin(dt, 6 + sf * 30 + (this.boosting ? 20 : 0));
  }

  /** Gentle hover and fan spin with no input (pause showcase). */
  idle(dt) {
    this.t += dt;
    this.model.position.y = Math.sin(this.t * 2.3) * 0.12;
    this.model.rotation.x = 0;
    if (this.fan) this.fan.rotation.z -= dt * 3;
    this.spin(dt);
    if (this.glow) this.glow.intensity = 12 + 4 * Math.sin(this.t * 9);
    if (this.head) this.head.intensity = 4; // the chase-camera fill light would glare in close-ups
  }

  updateFX(camPos) {
    this.aura?.update(camPos); // KILLER STATUS flames (they hide themselves with the car)
    if (!this.root.visible) return;
    const sf = Math.min(1, Math.max(0, this.speed) / CAR.MAX);
    const back = this.fwd.clone().negate();
    const e = this.root.matrixWorld;
    const t = performance.now() / 1000;
    // nozzle positions in world space (scale = flame size)
    const [ex, ey, ez] = this.exhaust;
    const noz = [[-ex, ey, ez, 1], [ex, ey, ez, 1]];
    if (this.turbine) noz.push(this.turbine);
    this.nozzles = noz.map(([x, y, z, s]) => ({ p: new THREE.Vector3(x, y, z).applyMatrix4(e), s }));
    // flames: a short pilot flame when coasting, full thrust on W, long afterburner on nitro, sputter as it runs out
    const thrust = 0.25 + 0.75 * this.thrust;
    const stutter = this.sputter > 0 ? 0.35 + 0.65 * (Math.sin(t * 90) > 0.2 ? 1 : 0.25) : 1;
    this.nozzles.forEach((n, i) => {
      const f = this.flames[i];
      const len = (1.6 + 3.2 * thrust + sf * 1.6 + this.nitro * 6 + this.kick * 3) * n.s;
      const wid = (0.75 + 0.3 * thrust + this.nitro * 0.35 + this.kick * 0.3) * n.s;
      const center = n.p.clone().addScaledVector(back, len * 0.5 - 0.25 * n.s);
      axisBillboard(f, center, this.fwd, len, wid, camPos);
      const u = f.material.uniforms;
      u.uTime.value = t;
      u.uNitro.value = Math.min(1, this.nitro + this.kick * 0.5);
      u.uAlpha.value = (0.4 + 0.3 * thrust + 0.15 * this.nitro) * stutter * (i === 2 ? 0.75 : 1);
    });
    if (this.nfx) {
      if (this.ignited) this.nfx.ignite(this.nozzles, this);
      if (this.ended) this.nfx.end(this.nozzles, this);
      this.nfx.update(this.fxDt || 1 / 60, this.nozzles, this, this.nitro);
    }
    this.ignited = this.ended = false;
    if (this.rival) return;
    [-1, 1].forEach((side, k) => {
      const src = this.nozzles[k].p;
      this.ribbons[k].boost = this.nitro;
      this.ribbons[k + 2].boost = this.nitro;
      this.ribbons[k].update(src, camPos, 0.5 + sf * 0.6 + this.nitro * 0.15);
      this.ribbons[k].mat.uniforms.uAlpha.value = (0.25 + sf * 0.6) * (1 - this.nitro * 0.3);
      const [tx, ty, tz] = this.tail;
      const tl = new THREE.Vector3(side * tx, ty, tz).applyMatrix4(e);
      this.ribbons[k + 2].update(tl, camPos, 1);
      this.ribbons[k + 2].mat.uniforms.uAlpha.value = sf;
    });
  }

  muzzle() {
    return new THREE.Vector3(0, CANNON_POS[1] - 0.6, CANNON_POS[2] - 0.85).applyMatrix4(this.root.matrixWorld);
  }
}
