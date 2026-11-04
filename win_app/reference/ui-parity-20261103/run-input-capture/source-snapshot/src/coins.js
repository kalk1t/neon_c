import * as THREE from 'three';
import { COIN } from './ai.js';

const _m = new THREE.Matrix4(), _q = new THREE.Quaternion(), _s = new THREE.Vector3(), _e = new THREE.Euler();

/**
 * Gold coins strung along the racing line between gates. The first racer to touch one takes it;
 * it re-forms after COIN.RESPAWN seconds so later laps still have pickups.
 */
export class Coins {
  constructor(scene, track) {
    this.track = track;
    this.group = new THREE.Group(); // one road's coins: dispose() takes them away again
    scene.add(this.group);
    this.list = [];
    const cps = [...track.checkpoints.map((c) => c.sample), track.n].sort((a, b) => a - b);
    let prev = 0;
    for (const end of cps) {
      const span = (end - prev + track.n) % track.n;
      // a short arc of coins in the middle of each gap, weaving across lanes
      const lane = (this.list.length % 3 - 1) * 7;
      for (let k = 0; k < COIN.PER_GATE_GAP; k++) {
        const i = Math.round(prev + span * (0.3 + 0.4 * (k / (COIN.PER_GATE_GAP - 1)))) % track.n;
        const f = track.frame(i);
        const pos = f.p.clone().addScaledVector(f.right, lane + (k % 2 ? 7 : -7));
        pos.y += 1 + Math.cos(k * 0.7) * 1.5;
        this.list.push({ pos, sample: i, active: true, timer: 0, pop: 0 });
      }
      prev = end;
    }

    // the shop's gold wheel coin (Assets/04_Interface/shopUI/currency) on both faces, a glowing gold rim around it
    const geo = new THREE.CylinderGeometry(2.6, 2.6, 0.55, 40).rotateX(Math.PI / 2);
    const rim = new THREE.MeshStandardMaterial({ color: 0xffc23a, metalness: 1, roughness: 0.22, emissive: 0xffa012, emissiveIntensity: 2.4 });
    const art = new THREE.TextureLoader().load(`${import.meta.env.BASE_URL}ui/coin-256.png`);
    art.colorSpace = THREE.SRGBColorSpace;
    art.anisotropy = 4;
    art.repeat.setScalar(236 / 256); // the coin artwork fills pixels 10-245 of the image: crop to it
    art.offset.setScalar(10 / 256);
    const face = new THREE.MeshStandardMaterial({ map: art, emissiveMap: art, emissive: 0xffffff, emissiveIntensity: 0.55, metalness: 0.4, roughness: 0.35 }); // glows, but the wheel stays readable
    this.mesh = new THREE.InstancedMesh(geo, [rim, face, face], this.list.length); // cylinder groups: side, top, bottom
    this.mesh.frustumCulled = false;
    this.group.add(this.mesh);
    // soft glow halo per coin
    const halo = new THREE.SpriteMaterial({ color: new THREE.Color(1.6, 1.0, 0.25), blending: THREE.AdditiveBlending, depthWrite: false, transparent: true, opacity: 0.75, fog: false });
    const c = document.createElement('canvas');
    c.width = c.height = 64;
    const g = c.getContext('2d');
    const gr = g.createRadialGradient(32, 32, 0, 32, 32, 32);
    gr.addColorStop(0, 'rgba(255,255,255,1)');
    gr.addColorStop(0.35, 'rgba(255,255,255,0.35)');
    gr.addColorStop(1, 'rgba(255,255,255,0)');
    g.fillStyle = gr;
    g.fillRect(0, 0, 64, 64);
    halo.map = new THREE.CanvasTexture(c);
    this.halos = this.list.map((coin) => {
      const s = new THREE.Sprite(halo);
      s.position.copy(coin.pos);
      s.scale.setScalar(13);
      this.group.add(s);
      return s;
    });
    this.onCollect = null;
  }

  dispose() {
    this.group.removeFromParent();
    this.mesh.geometry.dispose();
    for (const m of new Set(this.mesh.material)) { m.map?.dispose(); m.dispose(); }
    this.halos[0]?.material.map.dispose();
    this.halos[0]?.material.dispose();
  }

  setVisible(v) {
    this.mesh.visible = v;
    this.hidden = !v;
  }

  reset() {
    for (const c of this.list) { c.active = true; c.timer = 0; c.pop = 0; }
  }

  /** Nearest active coin ahead of a racer (for AI), within reach and roughly in its lane. */
  ahead(r, maxDist = 90) {
    let best = null, bd = maxDist * maxDist;
    for (const c of this.list) {
      if (!c.active) continue;
      const dx = c.pos.x - r.car.pos.x, dy = c.pos.y - r.car.pos.y, dz = c.pos.z - r.car.pos.z;
      const along = dx * r.car.fwd.x + dy * r.car.fwd.y + dz * r.car.fwd.z;
      const d2 = dx * dx + dy * dy + dz * dz;
      if (along < 15 || d2 > bd || along * along < d2 * 0.85) continue;
      bd = d2;
      best = c.pos;
    }
    return best;
  }

  update(dt, t, racers, active) {
    const R2 = COIN.RADIUS * COIN.RADIUS;
    this.list.forEach((c, i) => {
      if (!c.active) {
        c.timer -= dt;
        if (c.timer <= 0) { c.active = true; c.pop = 0; }
      } else if (active) {
        for (const r of racers) {
          if (!r.alive || r.finished) continue;
          if (r.car.pos.distanceToSquared(c.pos) < R2) {
            c.active = false;
            c.timer = COIN.RESPAWN;
            r.coins += COIN.VALUE;
            this.onCollect?.(r, c);
            break;
          }
        }
      }
      c.pop = Math.min(1, c.pop + dt * 3);
      const sc = c.active ? 1 - (1 - c.pop) ** 3 : 0;
      _e.set(0, t * 2.6 + i * 0.7, 0);
      _q.setFromEuler(_e);
      _s.setScalar(Math.max(sc, 1e-4));
      _m.compose(c.pos, _q, _s);
      this.mesh.setMatrixAt(i, _m);
      this.halos[i].visible = c.active && !this.hidden;
      this.halos[i].scale.setScalar(sc * (12 + Math.sin(t * 4 + i) * 1.5));
    });
    this.mesh.instanceMatrix.needsUpdate = true;
  }
}
