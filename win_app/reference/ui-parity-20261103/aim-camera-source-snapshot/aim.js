// Mouse aim, lock-on and the player's weapons (pulse blaster, seekers, spin dodge, EMP), plus the demo autopilot.
// Split from main.js (D1).
import * as THREE from 'three';
import { SPIN } from './car.js';
import { WEAPON } from './ai.js';
import { track } from './world.js';
import { cam } from './camera.js';
import { live } from './roam.js';
import { damage } from './race.js';

const _d = new THREE.Vector3(), _tmp = new THREE.Vector3();
const MOUSE_RAD_PER_PX = 0.0021;
let g, player, car, rivals, hud, sound, weapons, input, settings;
/** deps from main.js: g, player, car, rivals, hud, sound, weapons, input, settings. */
export function initAim(deps) {
  ({ g, player, car, rivals, hud, sound, weapons, input, settings } = deps);
}

/** Puts the aim straight ahead of the car (race start, and whenever the mouse is captured) and drops stale movement. */
export function centreAim() {
  g.aim.yaw = player.car.yaw;
  g.aim.pitch = player.car.pitch;
  input.takeMouse();
}

export function updateAim(inp) {
  const c = player.car;
  const m = input.takeMouse();
  const k = MOUSE_RAD_PER_PX * settings.sens;
  if (inp.look) {
    // RMB held: look around the car (the aim stays where it was, so the car keeps its course)
    cam.lookYaw -= m.dx * k * 1.6;
    cam.lookPitch = THREE.MathUtils.clamp(cam.lookPitch + m.dy * k * 1.6 * (settings.invert ? -1 : 1), -0.45, 1.15);
  } else {
    g.aim.yaw -= m.dx * k;
    g.aim.pitch -= m.dy * k * (settings.invert ? -1 : 1);
  }
  cam.looking = !!inp.look;
  g.camAim = !!g.mouseLocked; // mouse-aim camera (v0.2.12): the view follows the aim, the aim ring stays in the centre
  // keyboard/gamepad steering drags the aim along with the car
  if (inp.keySteer) g.aim.yaw = c.yaw;
  if (inp.keyClimb) g.aim.pitch = c.pitch;
  let dy = g.aim.yaw - c.yaw;
  dy = Math.atan2(Math.sin(dy), Math.cos(dy));
  dy = THREE.MathUtils.clamp(dy, -1.1, 1.1);
  g.aim.yaw = c.yaw + dy;
  g.aim.pitch = THREE.MathUtils.clamp(g.aim.pitch, -0.62, 0.62);
  const cp = Math.cos(g.aim.pitch);
  g.aimDir.set(-Math.sin(g.aim.yaw) * cp, Math.sin(g.aim.pitch), -Math.cos(g.aim.yaw) * cp);
  // the car points its nose at the aim, forwards and backwards (aimSteer: no car-style reversed steering, which made
  // the nose turn away from the aim and the car spin while backing up)
  if (!inp.keySteer) {
    inp.steer = THREE.MathUtils.clamp(-dy * 2.8, -1, 1);
    inp.aimSteer = true;
  }
  if (!inp.keyClimb) inp.climb = THREE.MathUtils.clamp(g.aim.pitch / 0.62, -1, 1); // the nose settles at the aim's height
  return inp;
}

/** Angle between the aim and a rival, or null if it can't be targeted. */
function aimAngle(r, maxDist) {
  if (!(g.racers.includes(r) || g.remotes.includes(r)) || !r.alive || r.finished) return null;
  _d.subVectors(r.car.pos, player.car.pos);
  const d = _d.length();
  if (d > maxDist || d < 8) return null;
  return Math.acos(THREE.MathUtils.clamp(_d.dot(g.aimDir) / d, -1, 1));
}
/** Sticky lock-on: acquire within 0.3 rad and LOCK_RANGE, keep until 0.5 rad and LOCK_KEEP; seekers need a full lock (lockT = 1). */
export function updateLock(dt) {
  const keep = g.lock && aimAngle(g.lock, WEAPON.LOCK_KEEP);
  if (!(keep != null && keep !== false && keep < 0.5)) {
    let best = null, ba = 0.3;
    for (const r of g.mode === 'roam' ? g.remotes : rivals) {
      const a = aimAngle(r, WEAPON.LOCK_RANGE);
      if (a != null && a < ba) { ba = a; best = r; }
    }
    if (best !== g.lock) {
      g.lock = best;
      g.lockT = 0;
      if (best) sound.lockTick();
    }
  }
  if (!g.lock) { g.lockT = 0; return; }
  const was = g.lockT;
  g.lockT = Math.min(1, g.lockT + dt / WEAPON.LOCK_TIME);
  if (was < 1 && g.lockT >= 1) sound.locked();
}

/** Lead the target: where to aim so a projectile of `speed` meets it. */
function leadDir(from, r, speed) {
  const t = from.distanceTo(r.car.pos) / speed;
  return _tmp.copy(r.car.pos).addScaledVector(r.car.vel, t * 0.9).sub(from).normalize().clone();
}

const _side = new THREE.Vector3();
export function playerWeapons(dt, inp) {
  const W = WEAPON;
  const speed = Math.max(0, car.speed);
  // pulse blaster: hold to fire, builds heat
  g.boltCD -= dt;
  g.heat = Math.max(0, g.heat - W.COOL_RATE * dt * (g.overheated ? 1.4 : inp.fire ? 0.35 : 1));
  if (g.overheated && g.heat <= 0.25) g.overheated = false;
  if (inp.fire && !g.overheated && g.boltCD <= 0) {
    g.boltCD = W.BOLT_RATE;
    g.side = -g.side;
    _side.set(1, 0, 0).applyQuaternion(car.root.quaternion);
    const from = car.muzzle().addScaledVector(_side, g.side * 1.3);
    const dir = g.lock ? leadDir(from, g.lock, speed + W.BOLT_SPEED) : g.aimDir.clone();
    dir.x += (Math.random() - 0.5) * 0.008;
    dir.y += (Math.random() - 0.5) * 0.008;
    weapons.fire(player, from, dir, speed, g.lock, {
      kind: 'bolt', mat: player.shotMat || weapons.boltMat, speed: W.BOLT_SPEED, turn: 2.5, life: W.BOLT_LIFE, damage: W.BOLT_DMG, len: 24, wid: 5, radius: 3.6,
    });
    weapons.flash(from, player.shotColor); // in our car's colour, like the bolts
    live?.fire('bolt', from, dir);
    sound.blaster();
    g.shake = Math.min(0.5, g.shake + 0.03);
    g.heat += W.HEAT_PER_BOLT;
    if (g.heat >= 1) {
      g.heat = 1;
      g.overheated = true;
      sound.overheat();
    }
  }
  // seeker missiles: charges refill over time, home hard on a full lock
  g.missiles = Math.min(W.MISSILES, g.missiles + dt / W.MISSILE_RECHARGE);
  if (input.hit('Space', 'Mouse1')) {
    if (g.missiles >= 1) {
      g.missiles -= 1;
      const tgt = g.lockT >= 1 ? g.lock : null;
      const dir = g.aimDir.clone().add(new THREE.Vector3(0, 0.08, 0));
      live?.fire('missile', car.muzzle(), dir, tgt?.remote);
      weapons.fire(player, car.muzzle(), dir, speed, tgt, {
        kind: 'missile', mat: weapons.missileMat, speed: 60, accel: 420, maxSpeed: speed + 330, turn: tgt ? 7 : 0, always: true,
        life: 3.6, damage: W.MISSILE_DMG, len: 12, wid: 6, radius: 5.5, trail: true,
      });
      sound.missile();
      g.shake += 0.25;
      if (!tgt) hud.sub(g.lock ? 'NO LOCK · DUMBFIRE' : 'NO TARGET · DUMBFIRE', 0.8);
    } else sound.deny();
  }
  // spin dodge (F): barrel roll toward the way you're steering (a random side when flying straight)
  g.spinCD = Math.max(0, g.spinCD - dt);
  if (input.hit('KeyF')) {
    if (g.spinCD > 0) sound.deny();
    else {
      g.spinCD = SPIN.COOLDOWN;
      const side = Math.sign(inp.steer) || (Math.random() < 0.5 ? -1 : 1);
      car.dodge(side);
      live?.fire('spin', car.pos, new THREE.Vector3(side, 0, 0));
      sound.boostOn();
      g.kick = Math.max(g.kick || 0, 0.6);
      hud.flash('rgba(25, 230, 255, 0.3)');
      hud.sub('SPIN · 1.5× SPEED', 0.9);
    }
  }
  // EMP: hits everyone nearby, wipes incoming fire, shoves cars away
  g.empCD = Math.max(0, g.empCD - dt);
  if (input.hit('KeyX', 'KeyV')) {
    if (g.empCD > 0) { sound.deny(); return; }
    g.empCD = W.EMP_COOLDOWN;
    weapons.shockwave(car.pos.clone(), W.EMP_RADIUS);
    const purged = weapons.purge(car.pos, W.EMP_RADIUS, player);
    sound.emp();
    g.shake += 0.5;
    live?.fire('emp', car.pos, car.fwd);
    // free roam: the other players feel it on their side; count who was in range
    let n = g.mode === 'roam' ? g.remotes.filter((r) => r.car.root.visible && r.car.pos.distanceTo(car.pos) < W.EMP_RADIUS).length : 0;
    for (const r of g.racers) {
      if (r.isPlayer || !r.alive || r.finished) continue;
      _d.subVectors(r.car.pos, car.pos);
      const d = _d.length();
      if (d > W.EMP_RADIUS) continue;
      n++;
      r.fireCD += 3;
      r.car.vel.addScaledVector(_d.normalize(), 30 * (1 - d / W.EMP_RADIUS) + 8);
      damage(r, W.EMP_DMG * (1 - 0.5 * d / W.EMP_RADIUS), player, 'emp');
    }
    hud.sub(`EMP · ${n} HIT${n === 1 ? '' : 'S'}${purged ? ` · ${purged} SHOTS NEUTRALISED` : ''}`, 1.2);
  }
}

// Debug/demo pilot for the player car: chases a point ahead on the racing line.
export function autopilot() {
  const target = track.pts[(player.sample + 16) % track.n];
  const to = target.clone().sub(car.pos);
  let dy = Math.atan2(-to.x, -to.z) - car.yaw;
  dy = Math.atan2(Math.sin(dy), Math.cos(dy));
  const pitch = Math.atan2(to.y, Math.hypot(to.x, to.z));
  g.aim.yaw = car.yaw;
  g.aim.pitch = car.pitch;
  g.aimDir.copy(car.fwd);
  return {
    throttle: 1, brake: Math.abs(dy) > 0.5 ? 1 : 0, steer: THREE.MathUtils.clamp(-dy * 3, -1, 1),
    climb: THREE.MathUtils.clamp((pitch / 0.62) * 1.4, -1, 1), boost: Math.abs(dy) < 0.08,
  };
}
