import * as THREE from 'three';

export const LIVERIES = [
  { name: 'VOLT', paint: 0x0d0f18, accent: 0xffa020, skill: 1.0 },
  { name: 'RAZOR', paint: 0xc9ced8, accent: 0xff3344, skill: 0.99, metal: 0.9 },
  { name: 'NOVA', paint: 0x101a44, accent: 0x9a5cff, skill: 0.98 },
  { name: 'HEX', paint: 0x0a0a0c, accent: 0x7dff3a, skill: 0.97 },
  { name: 'ZERO', paint: 0xe8ecf4, accent: 0x3a8cff, skill: 0.96, metal: 0.3 },
  { name: 'ONYX', paint: 0x2a0710, accent: 0xffd040, skill: 0.955 },
  { name: 'KAIRO', paint: 0x07202a, accent: 0x19ffc0, skill: 0.95 },
  { name: 'VEX', paint: 0x3a3e48, accent: 0xff6a1a, skill: 0.94, metal: 0.95 },
  { name: 'LUX', paint: 0x1c0830, accent: 0xff6ab0, skill: 0.93 },
  { name: 'AXIOM', paint: 0x0b1220, accent: 0xffffff, skill: 0.92 },
];

// The PULSE blaster, the seekers and the EMP. Since v0.2.15 rivals fire the same bolts (race.js rivalCombat; they
// used to fire one homing shot every 4-8 s: 30 damage on the player, 51 on a rival). Every car has 100 hp.
export const WEAPON = {
  BOLT_DMG: 21, BOLT_RATE: 0.21, BOLT_SPEED: 750, HEAT_PER_BOLT: 0.08, COOL_RATE: 0.45, // pulse blaster (hold LMB); v0.2.14: 5 bolts wreck a car
  BOLT_LIFE: 1.5, // s: with BOLT_SPEED 750, ~1,125 m standing still, more at speed (the car's speed is added); v0.2.14, was 430 m/s × 1.3 s
  BOLT_LEN: 45, BOLT_WID: 7, // m: the bolt's streak (looks only; the hit radius is 3.6 m); v0.2.14, was 24 / 5
  LOCK_RANGE: 2000, LOCK_KEEP: 2100, // m: lock-on acquired within / kept until (v0.2.14, was 400 / 460)
  MISSILE_DMG: 48, MISSILES: 3, MISSILE_RECHARGE: 4.5, LOCK_TIME: 0.5, // seekers (Space / MMB; F is the spin dodge)
  EMP_DMG: 25, EMP_RADIUS: 95, EMP_COOLDOWN: 11, // shockwave (X / V)
};
export const COIN = { PER_GATE_GAP: 6, VALUE: 1, RESPAWN: 6, RADIUS: 6.5 }; // coins between each pair of gates

/** Race state for any car, player or AI. */
export class Racer {
  constructor(car, name, color, isPlayer = false, skill = 1) {
    Object.assign(this, { car, name, color, isPlayer, skill });
    this.seed = Math.random() * 100;
    this.lane = (Math.random() - 0.5) * 12;
    this.laneY = (Math.random() - 0.5) * 5;
    this.dodge = 0;
    this.prevPos = new THREE.Vector3();
    this.passed = new Set();
    this.takedowns = 0;
  }
  reset(pos, yaw, track) {
    this.car.place(pos, yaw, 0);
    this.car.setVisible(true);
    this.car.boost = 1;
    this.prevPos.copy(pos);
    this.lap = 1;
    this.nextCP = 0;
    this.passed.clear();
    this.lapTime = 0;
    this.lapTimes = [];
    this.finished = false;
    this.finishTime = null;
    this.hp = 100;
    this.wreck = 0;
    this.lostTimer = 0;
    this.stuck = 0;
    this.wrongWay = 0;
    this.smokeT = 0;
    this.takedowns = 0;
    this.coins = 0;
    this.fireCD = 0; // > 0: holding fire (an EMP)
    this.heat = 0;
    this.overheated = false;
    this.boltCD = Math.random() * 0.5;
    // the rest of the player's arsenal (race.js rivalCombat, v0.2.16)
    this.missiles = WEAPON.MISSILES;
    this.seekerCD = 0;
    this.spinCD = 0;
    this.empCD = 0;
    this.lock = null;
    this.lockT = 0;
    this.react = 0.1 + Math.random() * 0.45;
    this.lastSafe = { pos: pos.clone(), yaw };
    this.sample = track.nearestGlobal(pos);
  }
  get alive() {
    return this.wreck <= 0;
  }
  /** Distance-along-race in track samples (for positions). */
  progress(track) {
    let s = this.sample;
    if (this.nextCP === 0 && s > track.n / 2) s -= track.n;
    return (this.lap - 1) * track.n + s;
  }
}

const _t = new THREE.Vector3(), _rel = new THREE.Vector3();

/** Corner speeds for fast rivals (v0.2.15): a car turns at most ~1.15 rad/s at speed, so a bend of curvature k
 *  (rad per metre, sideways and up/down together) can be taken at 1.15 / k m/s. CORNER.GRIP < 1 keeps a margin;
 *  CORNER.BRAKE is the deceleration (m/s^2) a rival plans its braking with (the car brakes at 70). */
export const CORNER = { TURN: 1.15, GRIP: 0.8, BRAKE: 45, SPAN: 3 };
const cornerCache = new WeakMap();
/** Fastest speed (m/s) at each track sample. */
export function cornerSpeeds(track) {
  let v = cornerCache.get(track);
  if (v) return v;
  const n = track.n, s = CORNER.SPAN, step = track.length / n;
  v = new Float32Array(n);
  for (let i = 0; i < n; i++) {
    const a = track.tans[(i - s + n) % n], b = track.tans[(i + s) % n];
    const k = Math.acos(THREE.MathUtils.clamp(a.dot(b), -1, 1)) / (2 * s * step);
    v[i] = k > 1e-6 ? (CORNER.TURN * CORNER.GRIP) / k : Infinity;
  }
  cornerCache.set(track, v);
  return v;
}
/** The fastest r may fly now and still slow down in time for every bend ahead. */
function cornerLimit(r, track) {
  const v = cornerSpeeds(track), n = track.n, step = track.length / n;
  const sp = Math.max(0, r.car.speed);
  const reach = (sp * sp) / (2 * CORNER.BRAKE) + 40;
  let lim = Infinity;
  for (let k = 0, d = 0; d < reach; k += 1, d += step) {
    const vc = v[(r.sample + k) % n];
    if (vc < Infinity) lim = Math.min(lim, Math.sqrt(vc * vc + 2 * CORNER.BRAKE * d));
  }
  return lim;
}

/** Steers along the racing line with a per-driver lane, dodging cars ahead; converges to centre near gates. */
export function aiInput(r, track, racers, targetPos, time, dt, coin = null) {
  const car = r.car;
  const look = 14 + Math.round(Math.max(0, car.speed) / 14);
  const f = track.frame((r.sample + look) % track.n);
  const right = f.right;

  // dodge slower cars directly ahead
  for (const o of racers) {
    if (o === r || !o.alive) continue;
    _rel.subVectors(o.car.pos, car.pos);
    const ahead = _rel.dot(car.fwd);
    if (ahead > 0 && ahead < 32) {
      const lat = _rel.dot(right);
      if (Math.abs(lat) < 5.5) r.dodge = lat > 0 ? -7 : 7;
    }
  }
  r.dodge *= Math.exp(-1.2 * dt);

  const gateDist = car.pos.distanceTo(targetPos);
  const laneK = THREE.MathUtils.clamp((gateDist - 60) / 160, 0, 1);
  const off = (r.lane + Math.sin(time * 0.25 + r.seed) * 3 + r.dodge) * laneK;
  _t.copy(f.p).addScaledVector(right, off);
  _t.y += r.laneY * laneK;
  // greedy pilots divert to a coin just ahead of them
  if (coin) _t.lerp(coin, 0.85);
  // the last ~1.5 s before a gate: fly at the gate itself (v0.2.15: fast rivals aimed far ahead along the road and
  // flew over or past the 15 m ring)
  const near = THREE.MathUtils.clamp(1 - gateDist / Math.max(60, Math.max(0, car.speed) * 1.5), 0, 1);
  if (near > 0) _t.lerp(targetPos, near);

  _rel.subVectors(_t, car.pos);
  let dy = Math.atan2(-_rel.x, -_rel.z) - car.yaw;
  dy = Math.atan2(Math.sin(dy), Math.cos(dy));
  // climb toward the target, correcting for where the car is actually going (v0.2.15: fast rivals lagged behind dips
  // and flew over gates)
  const aim = Math.atan2(_rel.y, Math.hypot(_rel.x, _rel.z));
  const sp = car.vel.length();
  const going = sp > 1 ? Math.asin(THREE.MathUtils.clamp(car.vel.y / sp, -1, 1)) : car.pitch;
  const pitch = aim + (aim - going) * 0.8;
  // slow down for the bends ahead (v0.2.15): fast rivals used to fly straight off the road
  const lim = cornerLimit(r, track), over = car.speed - lim;
  return {
    throttle: over > -4 ? 0 : 1,
    brake: Math.abs(dy) > 0.55 || over > 6 ? 1 : 0,
    steer: THREE.MathUtils.clamp(-dy * 2.6, -1, 1),
    climb: THREE.MathUtils.clamp((pitch / 0.62) * 1.4, -1, 1),
    boost: over < -25 && Math.abs(dy) < 0.07 && car.boost > 0.35 && Math.sin(time * 0.7 + r.seed) > 0.2 - r.skill * 0.6,
  };
}
