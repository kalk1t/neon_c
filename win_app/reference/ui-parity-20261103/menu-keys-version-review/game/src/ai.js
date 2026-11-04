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

// v0.2.3: rivals hit 1.5x harder and fire 1.5x as often as before, on every road (each road's own `fire` factor in
// roads.js still applies on top, so later roads stay harder)
export const RIVAL_POWER = 1.5;
export const DAMAGE = { RIVAL_SHOT: 34 * RIVAL_POWER, RIVAL_SHOT_ON_PLAYER: 20 * RIVAL_POWER }; // 51 and 30 hp
// Player arsenal. Rivals have 100 hp.
export const WEAPON = {
  BOLT_DMG: 21, BOLT_RATE: 0.21, BOLT_SPEED: 750, HEAT_PER_BOLT: 0.08, COOL_RATE: 0.45, // pulse blaster (hold LMB); v0.2.14: 5 bolts wreck a car
  BOLT_LIFE: 1.5, // s: with BOLT_SPEED 750, ~1,125 m standing still, more at speed (the car's speed is added); v0.2.14, was 430 m/s × 1.3 s
  BOLT_LEN: 45, BOLT_WID: 7, // m: the bolt's streak (looks only; the hit radius is 3.6 m); v0.2.14, was 24 / 5
  LOCK_RANGE: 2000, LOCK_KEEP: 2100, // m: lock-on acquired within / kept until (v0.2.14, was 400 / 460)
  MISSILE_DMG: 48, MISSILES: 3, MISSILE_RECHARGE: 4.5, LOCK_TIME: 0.5, // seekers (Space / MMB; F is the spin dodge)
  EMP_DMG: 25, EMP_RADIUS: 95, EMP_COOLDOWN: 11, // shockwave (X / V)
};
export const COIN = { PER_GATE_GAP: 6, VALUE: 1, RESPAWN: 6, RADIUS: 6.5 }; // coins between each pair of gates
export const AI_FIRE_INTERVAL = [6 / RIVAL_POWER, 12 / RIVAL_POWER]; // seconds between rival shots (min, max): 4-8 s

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
    this.fireCD = AI_FIRE_INTERVAL[0] + Math.random() * (4 / RIVAL_POWER);
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

  _rel.subVectors(_t, car.pos);
  let dy = Math.atan2(-_rel.x, -_rel.z) - car.yaw;
  dy = Math.atan2(Math.sin(dy), Math.cos(dy));
  const pitch = Math.atan2(_rel.y, Math.hypot(_rel.x, _rel.z));
  return {
    throttle: 1,
    brake: Math.abs(dy) > 0.55 ? 1 : 0,
    steer: THREE.MathUtils.clamp(-dy * 2.6, -1, 1),
    climb: THREE.MathUtils.clamp((pitch / 0.62) * 1.4, -1, 1),
    boost: Math.abs(dy) < 0.07 && car.boost > 0.35 && Math.sin(time * 0.7 + r.seed) > 0.2 - r.skill * 0.6,
  };
}
