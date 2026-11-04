import * as THREE from 'three';

// A road of SKYLINE CITY (roads.js): a closed aerial loop through the start deck. The start gate is at (0, 118, 0),
// heading -Z. Road 1 (SKYLINE LOOP) climbs to a high sweeper, dives low over the clouds, climbs again, hairpins, then
// returns over the deck; roads 2-9 are longer, twistier loops through the same city.
export const SAMPLE_STEP = 4;
// Start grid on the deck (metres relative to the start line, +Z is behind it). Two columns, six rows.
export const GRID = Array.from({ length: 12 }, (_, i) => ({ x: i % 2 ? 8 : -8, z: 20 + Math.floor(i / 2) * 13 + (i % 2) * 5 }));

export const yawOf = (t) => Math.atan2(-t.x, -t.z);
export const forwardOf = (yaw, out = new THREE.Vector3()) => out.set(-Math.sin(yaw), 0, -Math.cos(yaw));

/**
 * The racing line of one road: road.ctrl is a centripetal Catmull-Rom loop. Its gates are either at fractions of the
 * control polygon (road.checkpoints, road 1) or road.rings gates spread evenly along the lap.
 */
export class Track {
  constructor(road) {
    this.road = road;
    const ctrl = road.ctrl;
    this.curve = new THREE.CatmullRomCurve3(ctrl.map((p) => new THREE.Vector3(...p)), true, 'centripetal', 0.5);
    this.length = this.curve.getLength();
    const n = Math.ceil(this.length / SAMPLE_STEP);
    this.n = n;
    this.pts = [];
    this.tans = [];
    for (let i = 0; i < n; i++) {
      const u = i / n;
      this.pts.push(this.curve.getPointAt(u));
      this.tans.push(this.curve.getTangentAt(u));
    }
    const rings = road.checkpoints
      ? road.checkpoints.map((ci) => ({ pos: this.curve.getPoint(ci / ctrl.length), tan: this.curve.getTangent(ci / ctrl.length) }))
      : Array.from({ length: road.rings }, (_, k) => {
        // first and last gate ~7% of a lap from the start line; each slides to the straightest spot nearby so it never
        // sits in the middle of a corner
        const u = 0.07 + (0.86 * k) / (road.rings - 1);
        const at = Math.round(u * n), reach = Math.round((0.86 / (road.rings - 1)) * n * 0.3);
        let best = at, bc = Infinity;
        for (let i = at - reach; i <= at + reach; i++) {
          const j = (i + n) % n, t = this.tans[j];
          // straight here and on the 100 m run-in before it, not steep
          let bend = 0;
          for (let b = -24; b <= 8; b += 4) bend = Math.max(bend, Math.abs(this.turn((j + b + n) % n, 8)));
          const cost = bend + Math.abs(t.y) * 0.8 + (Math.abs(i - at) / reach) * 0.05;
          if (cost < bc) { bc = cost; best = j; }
        }
        return { pos: this.pts[best].clone(), tan: this.tans[best].clone() };
      });
    this.checkpoints = rings.map(({ pos, tan }, k) => {
      const yaw = yawOf(tan);
      return { index: k, pos, yaw, forward: forwardOf(yaw), sample: this.nearestGlobal(pos) };
    });
    this.start = { pos: new THREE.Vector3(0, 118, 0), yaw: 0, forward: new THREE.Vector3(0, 0, -1), sample: 0 };
    // coarse xz lookup for scenery placement
    this.coarse = this.pts.filter((_, i) => i % 5 === 0);
    this.bounds = new THREE.Box3().setFromPoints(this.pts);
  }

  nearestGlobal(p) {
    let best = 0, bd = Infinity;
    for (let i = 0; i < this.n; i++) {
      const d = this.pts[i].distanceToSquared(p);
      if (d < bd) { bd = d; best = i; }
    }
    return best;
  }

  nearestLocal(p, hint, win = 30) {
    let best = hint, bd = Infinity;
    for (let k = -win; k <= win; k++) {
      const i = (hint + k + this.n) % this.n;
      const d = this.pts[i].distanceToSquared(p);
      if (d < bd) { bd = d; best = i; }
    }
    return best;
  }

  /** Distance in XZ to the nearest coarse point, and the track altitude there. */
  nearestXZ(x, z) {
    return nearestXZ(this.coarse, x, z);
  }

  /** Is a vertical span reaching up to y1 at (x,z), radius r, clear of the flight corridor (margin metres)? */
  corridorClear(x, z, r, y1, margin = 68) {
    for (const p of this.coarse) {
      const dxz = Math.hypot(p.x - x, p.z - z) - r;
      if (dxz < margin && y1 > p.y - margin) return false;
    }
    return true;
  }

  frame(i) {
    const t = this.tans[i];
    const right = new THREE.Vector3().crossVectors(t, THREE.Object3D.DEFAULT_UP).normalize();
    const up = new THREE.Vector3().crossVectors(right, t).normalize();
    return { p: this.pts[i], t, right, up };
  }

  /** Signed heading change (rad) between samples i-k and i+k (positive = turning left). */
  turn(i, k = 10) {
    const a = this.tans[(i - k + this.n) % this.n], b = this.tans[(i + k) % this.n];
    return Math.atan2(a.x * b.z - a.z * b.x, a.x * b.x + a.z * b.z) * -1;
  }
}

function nearestXZ(coarse, x, z) {
  let bd = Infinity, y = 0;
  for (const p of coarse) {
    const d = (p.x - x) ** 2 + (p.z - z) ** 2;
    if (d < bd) { bd = d; y = p.y; }
  }
  return { d: Math.sqrt(bd), y };
}

const CELL = 160;
/**
 * Every road at once, for building the city and its scenery: the same queries as a Track (coarse points, nearestXZ,
 * corridorClear, checkpoints, bounds, pts), answered for all roads. So the city is identical whichever road is raced,
 * and nothing solid stands in any road's way.
 */
export class Corridor {
  constructor(tracks) {
    this.tracks = tracks;
    this.coarse = tracks.flatMap((t) => t.coarse);
    this.pts = tracks.flatMap((t) => t.pts);
    this.n = this.pts.length;
    this.checkpoints = tracks.flatMap((t) => t.checkpoints);
    this.start = tracks[0].start;
    this.bounds = tracks.reduce((b, t) => b.union(t.bounds), new THREE.Box3());
    // xz grid of the coarse points for corridorClear (margin + radius stays under a cell)
    this.grid = new Map();
    for (const p of this.coarse) {
      const k = `${Math.floor(p.x / CELL)},${Math.floor(p.z / CELL)}`;
      if (!this.grid.has(k)) this.grid.set(k, []);
      this.grid.get(k).push(p);
    }
  }

  nearestXZ(x, z) {
    return nearestXZ(this.coarse, x, z);
  }

  corridorClear(x, z, r, y1, margin = 68) {
    const reach = r + margin, cx = Math.floor(x / CELL), cz = Math.floor(z / CELL), c = Math.ceil(reach / CELL);
    for (let i = cx - c; i <= cx + c; i++) {
      for (let j = cz - c; j <= cz + c; j++) {
        for (const p of this.grid.get(`${i},${j}`) ?? []) {
          if (Math.hypot(p.x - x, p.z - z) - r < margin && y1 > p.y - margin) return false;
        }
      }
    }
    return true;
  }
}
