import * as THREE from 'three';

const loader = new THREE.TextureLoader();
const texCache = {};
export function tex(name) {
  if (!texCache[name]) {
    const t = loader.load(`tex/${name}.png`);
    t.colorSpace = THREE.SRGBColorSpace;
    texCache[name] = t;
  }
  return texCache[name];
}

export function additive(map, color = new THREE.Color(1, 1, 1), opacity = 1) {
  return new THREE.MeshBasicMaterial({
    map, color, transparent: true, opacity, blending: THREE.AdditiveBlending, depthWrite: false, side: THREE.DoubleSide, fog: false,
  });
}

const _x = new THREE.Vector3(), _y = new THREE.Vector3(), _z = new THREE.Vector3();
/** Orients a unit XY quad so +X runs along `dir` (head at +X) while facing the camera. */
export function axisBillboard(mesh, center, dir, length, width, camPos) {
  _x.copy(dir).normalize();
  _z.subVectors(camPos, center);
  _z.addScaledVector(_x, -_x.dot(_z)).normalize();
  _y.crossVectors(_z, _x);
  mesh.matrix.makeBasis(_x.multiplyScalar(length), _y.multiplyScalar(width), _z);
  mesh.matrix.setPosition(center);
  mesh.matrixWorldNeedsUpdate = true;
}

/** Camera-facing ribbon trail with fading alpha. */
export class Ribbon {
  /** wave: sideways ripple amplitude (m) travelling down the trail. */
  constructor(scene, color, points = 34, width = 0.9, wave = 0) {
    this.n = points;
    this.width = width;
    this.wave = wave;
    this.boost = 0;
    this.hist = [];
    const pos = new Float32Array(points * 2 * 3);
    const a = new Float32Array(points * 2);
    const side = new Float32Array(points * 2);
    for (let i = 0; i < points; i++) {
      a[i * 2] = a[i * 2 + 1] = 1 - i / (points - 1);
      side[i * 2 + 1] = 1;
    }
    const idx = [];
    for (let i = 0; i < points - 1; i++) {
      const k = i * 2;
      idx.push(k, k + 1, k + 2, k + 1, k + 3, k + 2);
    }
    const g = new THREE.BufferGeometry();
    g.setAttribute('position', new THREE.BufferAttribute(pos, 3));
    g.setAttribute('aFade', new THREE.BufferAttribute(a, 1));
    g.setAttribute('aSide', new THREE.BufferAttribute(side, 1));
    g.setIndex(idx);
    this.geo = g;
    this.mat = new THREE.ShaderMaterial({
      transparent: true, depthWrite: false, blending: THREE.AdditiveBlending, side: THREE.DoubleSide,
      uniforms: { uColor: { value: color }, uAlpha: { value: 1 }, uTime: { value: 0 }, uBoost: { value: 0 }, uPhase: { value: Math.random() * 6.3 } },
      vertexShader: `attribute float aFade; attribute float aSide; varying float vF; varying float vS;
        void main(){ vF = aFade; vS = aSide; gl_Position = projectionMatrix*modelViewMatrix*vec4(position,1.); }`,
      // energy flowing away from the car: fine scrolling bands + bigger pulses, a white-hot core with soft edges and a
      // light flicker; everything runs faster and brighter while boosting
      fragmentShader: `uniform vec3 uColor; uniform float uAlpha, uTime, uBoost, uPhase; varying float vF; varying float vS;
        void main(){
          float along = 1. - vF;
          float core = pow(clamp(1. - abs(vS * 2. - 1.), 0., 1.), 1.4);
          float speed = 1. + uBoost * 1.4;
          float bands = 0.45 + 0.55 * sin(along * 46. - uTime * 20. * speed + uPhase);
          float pulse = 1.6 * smoothstep(0.6, 1., sin(along * 10. - uTime * 9. * speed + uPhase));
          float flick = 0.88 + 0.12 * sin(uTime * 57. + uPhase * 7.);
          vec3 hot = mix(uColor, vec3(max(uColor.r, max(uColor.g, uColor.b))), pow(core, 5.) * (0.45 + 0.3 * uBoost));
          float a = vF * vF * uAlpha * flick * core * (bands + pulse * (0.9 + uBoost));
          gl_FragColor = vec4(hot * a, 1.);
        }`,
    });
    this.mesh = new THREE.Mesh(g, this.mat);
    this.mesh.frustumCulled = false;
    scene.add(this.mesh);
  }
  reset(p) {
    this.hist = Array.from({ length: this.n }, () => p.clone());
  }
  update(p, camPos, widthScale = 1) {
    if (!this.hist.length) this.reset(p);
    const last = this.hist.pop();
    this.hist.unshift(last.copy(p));
    const t = performance.now() / 1000;
    this.mat.uniforms.uTime.value = t;
    this.mat.uniforms.uBoost.value = this.boost;
    const arr = this.geo.attributes.position.array;
    const wave = this.wave * (1 + this.boost * 1.5);
    for (let i = 0; i < this.n; i++) {
      const a = this.hist[i], b = this.hist[Math.min(i + 1, this.n - 1)];
      _x.subVectors(i === this.n - 1 ? a : a, b);
      if (_x.lengthSq() < 1e-6) _x.set(0, 0, 1);
      _z.subVectors(camPos, a);
      _y.crossVectors(_x, _z).normalize();
      // ripple: zero at the car, growing down the trail
      const off = wave * (i / this.n) * Math.sin(i * 0.75 - t * 16 + this.mat.uniforms.uPhase.value);
      const cx = a.x + _y.x * off, cy = a.y + _y.y * off, cz = a.z + _y.z * off;
      _y.multiplyScalar(this.width * widthScale * (1 - i / this.n * 0.6));
      arr.set([cx + _y.x, cy + _y.y, cz + _y.z, cx - _y.x, cy - _y.y, cz - _y.z], i * 6);
    }
    this.geo.attributes.position.needsUpdate = true;
  }
}

const _seg = new THREE.Vector3(), _rel = new THREE.Vector3(), _dir = new THREE.Vector3();

/** Projectiles (optionally homing), hit-tests against racers + scenery, impacts, explosions, damage smoke. */
export class Weapons {
  constructor(scene, colliders) {
    this.scene = scene;
    this.colliders = colliders;
    this.shots = [];
    this.fx = [];
    this.racers = [];
    this.onHit = null;
    this.quad = new THREE.PlaneGeometry(1, 1);
    this.playerMat = additive(tex('projectile'), new THREE.Color(2.5, 2.5, 2.5));
    this.boltMat = additive(tex('projectile'), new THREE.Color(3.6, 1.4, 3.3));
    this.missileMat = additive(tex('projectile'), new THREE.Color(1.8, 3.1, 3.8));
    this.waves = [];
    this.sphere = new THREE.SphereGeometry(1, 40, 20);
    this.ring = new THREE.RingGeometry(0.92, 1, 96);
    this.light = new THREE.PointLight(0xff2bd6, 0, 70, 2);
    this.boom = new THREE.PointLight(0xff7ae6, 0, 160, 2);
    scene.add(this.light, this.boom);
    this.boomT = 0;
  }

  /**
   * owner: Racer; target: Racer to home on (or null).
   * o: { kind, mat, speed, accel, maxSpeed, turn, life, damage, len, wid, radius, trail, always }
   */
  fire(owner, pos, dir, inheritSpeed, target, o = {}) {
    const m = new THREE.Mesh(this.quad, o.mat || owner.shotMat || this.playerMat);
    m.matrixAutoUpdate = false;
    m.frustumCulled = false;
    this.scene.add(m);
    const speed = inheritSpeed + (o.speed ?? 240);
    const shot = {
      m, owner, target, kind: o.kind || 'shot', pos: pos.clone(), vel: dir.clone().normalize().multiplyScalar(speed), speed,
      accel: o.accel || 0, maxSpeed: o.maxSpeed || speed, life: o.life ?? 2.4, turn: o.turn ?? (owner.isPlayer ? 5 : 2.2),
      damage: o.damage, flown: 0, len: o.len || 18, wid: o.wid || 6, radius: o.radius || 3.2, trail: !!o.trail, trailT: 0, always: !!o.always,
    };
    this.shots.push(shot);
    return shot;
  }

  /** Short muzzle flash where a shot leaves the car. */
  flash(pos, color = new THREE.Color(3, 1.2, 2.8)) {
    this._sprite(tex('impact'), color, THREE.AdditiveBlending, pos, 3.2, 0.07);
  }

  /** Expanding EMP bubble + ground ring. */
  shockwave(pos, radius) {
    const mk = (geo, op) => {
      const m = new THREE.Mesh(geo, new THREE.MeshBasicMaterial({
        color: new THREE.Color(2.4, 0.5, 2.2), transparent: true, opacity: op, blending: THREE.AdditiveBlending, depthWrite: false, side: THREE.DoubleSide, fog: false,
      }));
      m.position.copy(pos);
      this.scene.add(m);
      return m;
    };
    const bubble = mk(this.sphere, 0.2);
    const ring = mk(this.ring, 0.9);
    ring.lookAt(pos.clone().add(new THREE.Vector3(0, 1, 0)));
    this.waves.push({ meshes: [[bubble, 0.2], [ring, 0.9]], t: 0, life: 0.55, radius });
    this._sprite(tex('impact'), new THREE.Color(2.6, 0.8, 2.6), THREE.AdditiveBlending, pos, 40, 0.45);
    this.boom.position.copy(pos);
    this.boomT = 0.4;
  }

  /** Destroys other owners' shots within radius (EMP). Returns count. */
  purge(pos, radius, owner) {
    let n = 0;
    for (let i = this.shots.length - 1; i >= 0; i--) {
      const p = this.shots[i];
      if (p.owner === owner || p.pos.distanceTo(pos) > radius) continue;
      this.impact(p.pos, true, 10);
      this.scene.remove(p.m);
      this.shots.splice(i, 1);
      n++;
    }
    return n;
  }

  _sprite(map, color, blending, pos, size, life, rise = 0) {
    const s = new THREE.Sprite(new THREE.SpriteMaterial({ map, color, blending, depthWrite: false, transparent: true, fog: blending !== THREE.AdditiveBlending }));
    s.position.copy(pos);
    s.material.rotation = Math.random() * 6.28;
    this.scene.add(s);
    this.fx.push({ s, t: 0, life, size, rise, spin: (Math.random() - 0.5) * 2 });
  }

  impact(pos, spark = false, size = 14) {
    this._sprite(tex('impact'), spark ? new THREE.Color(0.6, 1.8, 2.4) : new THREE.Color(2.2, 2.2, 2.2), THREE.AdditiveBlending, pos, size, spark ? 0.35 : 0.5);
  }

  smoke(pos, k = 1) {
    if (this.fx.length > 260) return;
    const c = new THREE.Color(0.16, 0.12, 0.24).lerp(new THREE.Color(0.5, 0.15, 0.4), Math.random() * 0.4);
    this._sprite(tex('smoke'), c, THREE.NormalBlending, pos, 4 + 5 * k, 1.1 + Math.random() * 0.6, 3);
  }

  /** Damage smoke out of an engine nozzle (v0.2.18): starts with the car's speed and drags behind it, so it streams
   *  back from the engine; k 0..1 = how badly damaged (bigger, longer puffs). */
  exhaustSmoke(pos, vel, k = 1) {
    if (this.fx.length > 260) return;
    const c = new THREE.Color(0.16, 0.12, 0.24).lerp(new THREE.Color(0.5, 0.15, 0.4), Math.random() * 0.4);
    this._sprite(tex('smoke'), c, THREE.NormalBlending, pos, 1.3 + 2.2 * k, 0.5 + 0.5 * k + Math.random() * 0.3, 1.2);
    const f = this.fx[this.fx.length - 1];
    f.vel = vel.clone().multiplyScalar(0.93);
  }

  explode(pos) {
    this._sprite(tex('impact'), new THREE.Color(3, 1.6, 2.8), THREE.AdditiveBlending, pos, 46, 0.7);
    this._sprite(tex('impact'), new THREE.Color(0.8, 2.2, 3), THREE.AdditiveBlending, pos, 30, 0.5);
    this._sprite(tex('impact'), new THREE.Color(3, 3, 3), THREE.AdditiveBlending, pos, 16, 0.3);
    for (let i = 0; i < 9; i++) {
      const p = pos.clone().add(new THREE.Vector3((Math.random() - 0.5) * 8, (Math.random() - 0.5) * 6, (Math.random() - 0.5) * 8));
      this._sprite(tex('smoke'), new THREE.Color(0.2, 0.12, 0.28), THREE.NormalBlending, p, 10 + Math.random() * 10, 1.6 + Math.random(), 4);
    }
    this.boom.position.copy(pos);
    this.boomT = 0.5;
  }

  update(dt, camPos) {
    const prev = new THREE.Vector3();
    let nearest = null, nd = Infinity;
    for (let i = this.shots.length - 1; i >= 0; i--) {
      const p = this.shots[i];
      // homing toward a live target that is still in front of the shot
      const tg = p.target;
      if (p.accel) p.speed = Math.min(p.maxSpeed, p.speed + p.accel * dt);
      if (tg && tg.alive) {
        _rel.subVectors(tg.car.pos, p.pos);
        _dir.copy(p.vel).normalize();
        if (p.always || _rel.dot(_dir) > 0) {
          _dir.lerp(_rel.normalize(), 1 - Math.exp(-p.turn * dt)).normalize();
          p.vel.copy(_dir).multiplyScalar(p.speed);
        }
      } else if (p.accel) p.vel.setLength(p.speed);
      prev.copy(p.pos);
      p.pos.addScaledVector(p.vel, dt);
      p.life -= dt;
      // racer hits: closest point on the travelled segment vs. a 3.2m sphere
      let hitRacer = null;
      _seg.subVectors(p.pos, prev);
      const segLen2 = _seg.lengthSq() || 1;
      for (const r of this.racers) {
        if (r === p.owner || !r.alive || r.finished) continue;
        _rel.subVectors(r.car.pos, prev);
        const t = THREE.MathUtils.clamp(_rel.dot(_seg) / segLen2, 0, 1);
        _rel.copy(prev).addScaledVector(_seg, t);
        if (_rel.distanceToSquared(r.car.pos) < p.radius * p.radius) { hitRacer = r; break; }
      }
      const hitWorld = hitRacer ? null : this.colliders.segment(prev, p.pos) || (p.pos.y < (this.floor ?? 24) ? p.pos.clone() : null);
      // grow a little with distance so far-away fire stays readable (visual only; hit radius unchanged)
      const far = 1 + Math.min(2, p.pos.distanceTo(camPos) / 120);
      // the streak trails behind the shot's head, no longer than the way flown so far: drawn centred on the shot, half of
      // it used to stick out behind the muzzle, over the shooter's own car (v0.2.14)
      p.flown += p.pos.distanceTo(prev);
      const len = Math.min(p.len * far, p.flown + 1);
      _seg.copy(p.vel).setLength(len / 2);
      axisBillboard(p.m, prev.copy(p.pos).sub(_seg), p.vel, len, p.wid * far, camPos);
      if (p.trail && (p.trailT -= dt) <= 0) {
        p.trailT = 0.022;
        if (this.fx.length < 300) {
          this._sprite(tex('smoke'), new THREE.Color(0.35, 0.3, 0.5), THREE.NormalBlending, p.pos, 2.2, 0.7, 0.6);
          this._sprite(tex('impact'), new THREE.Color(0.5, 1.4, 2), THREE.AdditiveBlending, p.pos, 3, 0.18);
        }
      }
      if (hitRacer || hitWorld || p.life <= 0) {
        if (hitRacer) {
          this.impact(hitRacer.car.pos, p.kind === 'bolt', p.kind === 'bolt' ? 9 : p.kind === 'missile' ? 30 : 22);
          this.onHit?.(p, hitRacer);
        } else if (hitWorld) this.impact(hitWorld, p.kind === 'bolt', p.kind === 'bolt' ? 8 : 26);
        this.scene.remove(p.m);
        this.shots.splice(i, 1);
        continue;
      }
      const d = p.pos.distanceToSquared(camPos);
      if (d < nd) { nd = d; nearest = p; }
    }
    this.light.intensity = nearest ? 900 : 0;
    if (nearest) this.light.position.copy(nearest.pos);
    this.boomT = Math.max(0, this.boomT - dt);
    this.boom.intensity = this.boomT * 16000;

    for (let i = this.waves.length - 1; i >= 0; i--) {
      const w = this.waves[i];
      w.t += dt;
      const k = Math.min(1, w.t / w.life), e = 1 - (1 - k) ** 3;
      for (const [m, op] of w.meshes) {
        m.scale.setScalar(Math.max(0.01, w.radius * e));
        m.material.opacity = op * (1 - k);
      }
      if (k >= 1) {
        for (const [m] of w.meshes) { this.scene.remove(m); m.material.dispose(); }
        this.waves.splice(i, 1);
      }
    }

    for (let i = this.fx.length - 1; i >= 0; i--) {
      const f = this.fx[i];
      f.t += dt;
      const k = f.t / f.life;
      f.s.scale.setScalar(f.size * (0.4 + k * 0.9));
      f.s.material.opacity = (1 - k * k) * (f.rise ? 0.7 : 1);
      f.s.material.rotation += f.spin * dt;
      f.s.position.y += f.rise * dt;
      if (f.vel) {
        f.s.position.addScaledVector(f.vel, dt);
        f.vel.multiplyScalar(Math.exp(-1.6 * dt)); // air drag: the puff falls behind the car
      }
      if (k >= 1) {
        this.scene.remove(f.s);
        f.s.material.dispose();
        this.fx.splice(i, 1);
      }
    }
  }

  clear() {
    for (const p of this.shots) this.scene.remove(p.m);
    this.shots.length = 0;
  }
}
