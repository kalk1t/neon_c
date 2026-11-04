import * as THREE from 'three';

const CELL = 120;
const _v = new THREE.Vector3();
const _l = new THREE.Vector3();
const _q = new THREE.Vector3();

/** Oriented box rotated only around Y. */
export class OBB {
  constructor(cx, cy, cz, hx, hy, hz, rotY = 0) {
    this.c = new THREE.Vector3(cx, cy, cz);
    this.h = new THREE.Vector3(hx, hy, hz);
    this.cos = Math.cos(rotY);
    this.sin = Math.sin(rotY);
    const r = Math.hypot(hx, hz);
    this.bounds = [cx - r, cz - r, cx + r, cz + r];
  }
  resolve(p, radius, out) {
    const dx = p.x - this.c.x, dz = p.z - this.c.z;
    // world -> local (inverse Y rotation)
    const lx = this.cos * dx - this.sin * dz;
    const lz = this.sin * dx + this.cos * dz;
    const ly = p.y - this.c.y;
    const h = this.h;
    const cx = Math.max(-h.x, Math.min(h.x, lx));
    const cy = Math.max(-h.y, Math.min(h.y, ly));
    const cz = Math.max(-h.z, Math.min(h.z, lz));
    let nx = lx - cx, ny = ly - cy, nz = lz - cz;
    let d = Math.hypot(nx, ny, nz);
    let depth;
    if (d > 1e-6) {
      if (d >= radius) return false;
      nx /= d; ny /= d; nz /= d;
      depth = radius - d;
    } else {
      // inside: push out through the nearest face
      const px = h.x - Math.abs(lx), py = h.y - Math.abs(ly), pz = h.z - Math.abs(lz);
      nx = ny = nz = 0;
      if (px < py && px < pz) { nx = Math.sign(lx) || 1; depth = px + radius; }
      else if (py < pz) { ny = Math.sign(ly) || 1; depth = py + radius; }
      else { nz = Math.sign(lz) || 1; depth = pz + radius; }
    }
    // local -> world
    out.n.set(this.cos * nx + this.sin * nz, ny, -this.sin * nx + this.cos * nz);
    out.depth = depth;
    return true;
  }
}

/** Vertical cylinder. */
export class Cyl {
  constructor(x, z, r, y0, y1) {
    Object.assign(this, { x, z, r, y0, y1 });
    this.bounds = [x - r, z - r, x + r, z + r];
  }
  resolve(p, radius, out) {
    const dx = p.x - this.x, dz = p.z - this.z;
    const dr = Math.hypot(dx, dz);
    const cy = Math.max(this.y0, Math.min(this.y1, p.y));
    if (dr > this.r) {
      const ex = dx / dr * (dr - this.r), ez = dz / dr * (dr - this.r), ey = p.y - cy;
      const d = Math.hypot(ex, ey, ez);
      if (d >= radius) return false;
      out.n.set(ex / d, ey / d, ez / d);
      out.depth = radius - d;
      return true;
    }
    if (p.y < this.y0 - radius || p.y > this.y1 + radius) return false;
    const side = this.r - dr, top = this.y1 - p.y, bot = p.y - this.y0;
    if (top < side && top < bot) { out.n.set(0, 1, 0); out.depth = top + radius; }
    else if (bot < side) { out.n.set(0, -1, 0); out.depth = bot + radius; }
    else { out.n.set(dx / (dr || 1), 0, dz / (dr || 1)); out.depth = side + radius; }
    return true;
  }
}

/** Torus in a vertical plane (checkpoint hoop), axis = horizontal forward. */
export class Torus {
  constructor(center, yaw, R, r) {
    this.c = center.clone();
    this.R = R;
    this.r = r;
    this.cos = Math.cos(yaw);
    this.sin = Math.sin(yaw);
    const e = R + r;
    this.bounds = [center.x - e, center.z - e, center.x + e, center.z + e];
  }
  resolve(p, radius, out) {
    const dx = p.x - this.c.x, dz = p.z - this.c.z;
    // local frame: x = right, y = up, z = along axis
    const lx = this.cos * dx - this.sin * dz;
    const lz = this.sin * dx + this.cos * dz;
    const ly = p.y - this.c.y;
    const rr = Math.hypot(lx, ly) || 1e-6;
    const qx = lx - (lx / rr) * this.R, qy = ly - (ly / rr) * this.R;
    const d = Math.hypot(qx, qy, lz);
    if (d >= this.r + radius) return false;
    const nx = qx / d, ny = qy / d, nz = lz / d;
    out.n.set(this.cos * nx + this.sin * nz, ny, -this.sin * nx + this.cos * nz);
    out.depth = this.r + radius - d;
    return true;
  }
}

export class Colliders {
  constructor() {
    this.grid = new Map();
    this.all = [];
    this.stamp = 0;
  }
  add(c) {
    c._stamp = -1;
    this.all.push(c);
    const [x0, z0, x1, z1] = c.bounds;
    for (let i = Math.floor(x0 / CELL); i <= Math.floor(x1 / CELL); i++) {
      for (let j = Math.floor(z0 / CELL); j <= Math.floor(z1 / CELL); j++) {
        const k = i * 100003 + j;
        if (!this.grid.has(k)) this.grid.set(k, []);
        this.grid.get(k).push(c);
      }
    }
    return c;
  }
  /** Takes colliders out again (a road's race furniture when another road is chosen). */
  remove(list) {
    const gone = new Set(list);
    this.all = this.all.filter((c) => !gone.has(c));
    for (const [k, cell] of this.grid) {
      const keep = cell.filter((c) => !gone.has(c));
      if (keep.length) this.grid.set(k, keep); else this.grid.delete(k);
    }
  }
  /** Pushes the sphere out of everything it touches. Returns strongest contact normal or null. */
  collide(p, radius) {
    const list = this.grid.get(Math.floor(p.x / CELL) * 100003 + Math.floor(p.z / CELL));
    if (!list) return null;
    this.stamp++;
    const out = { n: new THREE.Vector3(), depth: 0 };
    let hit = null;
    for (const c of list) {
      if (c._stamp === this.stamp || c.off) continue; // off = switched off (race-only furniture in FREE ROAM)
      c._stamp = this.stamp;
      if (c.resolve(p, radius, out)) {
        p.addScaledVector(out.n, out.depth);
        if (!hit || out.depth > hit.depth) hit = { n: out.n.clone(), depth: out.depth };
      }
    }
    return hit;
  }
  /** Ray-march a projectile segment; returns hit point or null. */
  segment(a, b, radius = 0.6) {
    const steps = Math.max(1, Math.ceil(a.distanceTo(b) / 3));
    for (let s = 1; s <= steps; s++) {
      _v.lerpVectors(a, b, s / steps);
      _q.copy(_v);
      if (this.collide(_q, radius)) return _v.clone();
    }
    return null;
  }
}

export { _l };
