// Race rules shared by the player and the rivals: gates and laps, damage and wrecks, respawns, rival fire, car
// contacts, the ranking and the results screen. Split from main.js (D1).
import * as THREE from 'three';
import { DECK_Y } from './course.js';
import { WEAPON } from './ai.js';
import { CAR, SPIN } from './car.js';
import { rivalTop } from './roads.js';
import { fmt } from './hud.js';
import { track as trackEvent } from './online/analytics.js';
import { track, road, map, course, bestKey } from './world.js';
import { volAt, panAt } from './render.js';
import { time } from './clock.js';
import { live, roamSpawn, ROAM_SAFE } from './roam.js';

const $ = (id) => document.getElementById(id);
const esc = (t) => String(t).replace(/[&<>"']/g, (c) => `&#${c.charCodeAt(0)};`);
const _d = new THREE.Vector3(), _tmp = new THREE.Vector3();
/** The start / finish gate (its height follows the road: world.js setRoad). */
export const GATE = { pos: new THREE.Vector3(0, 126, 0), halfW: 23, y0: DECK_Y - 2, y1: DECK_Y + 31 };
let g, player, rivals, hud, sound, weapons, input, account, D;
/** The game state and objects from main.js (deps: g, player, rivals, hud, sound, weapons, input, account, roadsOpen, unlockNext). */
export function initRace(deps) {
  D = deps;
  ({ g, player, rivals, hud, sound, weapons, input, account } = deps);
}

export function targetOf(r) {
  return r.nextCP < track.checkpoints.length ? track.checkpoints[r.nextCP].pos : GATE.pos;
}
export function setGateText(t) {
  course.gateTex.userData.draw(t);
}
export const ordinal = (n) => n + ({ 1: 'st', 2: 'nd', 3: 'rd' }[n % 10 * (Math.floor(n / 10) % 10 !== 1)] || 'th');

/** A shot reached us mid-spin: no damage. */
export function dodged() {
  if (time - (g.dodgedAt ?? -9) > 0.4) hud.sub('DODGED', 0.7);
  g.dodgedAt = time;
  sound.hitMark(false);
}

// ---------------- race rules (shared by player and rivals) ----------------
/** keepHp: a voluntary reset (R) in FREE ROAM moves you but doesn't heal you. */
export function respawn(r, keepHp = false) {
  if (g.mode === 'roam' && r.isPlayer) {
    r.lastSafe = roamSpawn(live?.me?.slot ?? 0);
    if (!keepHp) g.roamSafeUntil = time + ROAM_SAFE; // after a wreck only, so R is no escape hatch
  }
  const s = r.lastSafe;
  const pos = s.pos.clone();
  if (!r.isPlayer) pos.add(new THREE.Vector3((Math.random() - 0.5) * 12, (Math.random() - 0.5) * 6, 0));
  r.car.place(pos, s.yaw, 35);
  r.car.setVisible(true);
  r.prevPos.copy(pos);
  if (!keepHp) r.hp = 100;
  r.wreck = 0;
  r.sample = track.nearestGlobal(pos);
  r.lostTimer = 0;
  r.stuck = 0;
  if (r.isPlayer) {
    g.aim.yaw = s.yaw;
    g.aim.pitch = 0;
    hud.flash();
  }
}

// how hard each hit type knocks a car: [speed kept, stun seconds]
const KNOCK = { bolt: [0.985, 0.12], missile: [0.5, 1.0], emp: [0.65, 1.6], shot: [0.6, 0.7] };
export function damage(r, amount, attacker, kind = 'shot') {
  if (!r.alive || r.finished || g.state !== 'race') return;
  if (r.car.dodging) { if (r.isPlayer) dodged(); return; }
  r.hp -= amount;
  const [keep, stun] = KNOCK[kind] || KNOCK.shot;
  r.car.speed *= keep;
  r.car.stun = Math.max(r.car.stun, stun);
  if (r.isPlayer) {
    g.shake = Math.min(1.2, g.shake + 0.6);
    hud.flash('rgba(255, 60, 90, 0.55)');
    sound.hit(18);
  } else if (attacker === player) {
    const kill = r.hp <= 0;
    hud.hitmark(kill);
    sound.hitMark(kind !== 'bolt' || kill);
    if (kind === 'missile') hud.sub(`SEEKER HIT ${r.name} · ${Math.max(0, Math.round(r.hp))}%`, 0.9);
  } else if (kind !== 'bolt') sound.damage(volAt(r.car.pos), panAt(r.car.pos));
  if (r.hp <= 0) wreck(r, attacker);
}

export function wreck(r, attacker) {
  weapons.explode(r.car.pos.clone());
  sound.explosion(r.isPlayer ? 1 : volAt(r.car.pos), r.isPlayer ? 0 : panAt(r.car.pos));
  r.wreck = 2.4;
  r.hp = 0;
  r.car.setVisible(false);
  r.car.speed = 0;
  r.car.vel.set(0, 0, 0);
  if (g.lock === r) g.lock = null;
  if (attacker === player && !r.isPlayer) {
    player.takedowns++;
    g.streak = g.raceTime - g.lastKill < 8 ? g.streak + 1 : 1;
    g.lastKill = g.raceTime;
    player.car.boost = Math.min(1, player.car.boost + 0.5);
    g.missiles = Math.min(WEAPON.MISSILES, g.missiles + 1);
    // a takedown also steals a share of the victim's coins
    const loot = Math.ceil(r.coins * 0.25);
    r.coins -= loot;
    player.coins += loot;
    if (loot) hud.coinPop();
    hud.message(['TAKEDOWN', 'TAKEDOWN', 'DOUBLE TAKEDOWN', 'TRIPLE TAKEDOWN'][g.streak] || `${g.streak}x RAMPAGE`, 1.4);
    hud.sub(`${r.name} WRECKED · +BOOST +SEEKER${loot ? ` · +${loot} COINS` : ''}`, 1.8);
  }
  if (r.isPlayer) {
    g.shake = 1.4;
    hud.message('WRECKED', 2, true);
    if (attacker && attacker !== r) hud.sub(`BY ${attacker.name}`, 2);
  }
}

function completeLap(r) {
  r.lapTimes.push(r.lapTime);
  r.lapTime = 0;
  r.lap++;
  r.passed.clear();
  r.nextCP = 0;
  if (r.lap > g.laps) {
    r.finished = true;
    r.finishTime = g.raceTime;
  }
  if (!r.isPlayer) return;
  const lapT = r.lapTimes.at(-1);
  const isBest = !g.best || lapT < g.best;
  if (isBest) {
    g.best = lapT;
    localStorage.setItem(bestKey(), String(g.best));
  }
  hud.laps(r.lapTimes, g.best);
  sound.lap();
  if (r.finished) {
    updateRanking();
    g.state = 'finished';
    g.finishPos = g.rankOf(r);
    setGateText('FINISH');
    hud.message(g.racers.length > 1 ? `${ordinal(g.finishPos)} PLACE` : 'FINISH', 3);
    input.unlock();
    setTimeout(showResults, 2000);
    return;
  }
  hud.message(r.lap === g.laps ? 'FINAL LAP' : `LAP ${r.lap}`, 1.6);
  hud.sub(`${isBest ? '★ BEST LAP ' : 'LAP '}${fmt(lapT)}`, 2.4);
  setGateText(r.lap === g.laps ? 'FINAL LAP' : 'START');
}

/** Gate ring radius (m) that counts a pass: the player's, and the rivals' (v0.2.15: rivals at 200+ m/s miss the ring by
 *  20-35 m on some gates, then circled back and got stuck; nobody can tell at that speed). */
export const GATE_R = 15, GATE_R_RIVAL = 45;
export function raceRules(r, dt) {
  const car = r.car;
  // free roam: the rings and path are still there, but there are no laps, gates or checks
  const cp = r.finished || g.mode === 'roam' ? null : track.checkpoints[r.nextCP];
  if (cp) {
    const a = _d.subVectors(r.prevPos, cp.pos).dot(cp.forward);
    const b = _d.subVectors(car.pos, cp.pos).dot(cp.forward);
    if (a < 0 && b >= 0) {
      const radial = _d.subVectors(car.pos, cp.pos).addScaledVector(cp.forward, -b).length();
      if (radial < (r.isPlayer ? GATE_R : GATE_R_RIVAL)) {
        r.passed.add(cp.index);
        r.nextCP++;
        r.lastSafe = { pos: cp.pos.clone().addScaledVector(cp.forward, 12), yaw: cp.yaw };
        car.boost = Math.min(1, car.boost + 0.3);
        if (r.isPlayer) {
          sound.checkpoint();
          hud.sub(`CHECKPOINT ${r.passed.size}/${track.checkpoints.length} · ${fmt(r.lapTime)}`, 1.4);
        }
      }
    }
  }
  if (!r.finished && g.mode !== 'roam' && r.prevPos.z > 0 && car.pos.z <= 0 && Math.abs(car.pos.x) < GATE.halfW && car.pos.y > GATE.y0 && car.pos.y < GATE.y1) {
    if (r.nextCP >= track.checkpoints.length) completeLap(r);
    else if (r.isPlayer && g.raceTime > 5 && r.passed.size > 0 && g.mode !== 'roam') hud.message('MISSED GATE', 1.4, true);
  }
  r.sample = track.nearestLocal(car.pos, r.sample);
  if (r.isPlayer) {
    const along = car.vel.dot(track.tans[r.sample]);
    r.wrongWay = along < -15 ? r.wrongWay + dt : 0;
    if (r.wrongWay > 1.2 && hud.msgTimer <= 0 && g.mode !== 'roam') hud.message('WRONG WAY', 0.8, true);
  } else {
    r.stuck = car.speed < 12 && g.raceTime > 3 ? r.stuck + dt : 0;
    if (r.stuck > 2.5) respawn(r);
  }
  const off = car.pos.distanceTo(track.pts[r.sample]);
  if (g.mode !== 'roam' && (car.pos.y < map.floor || off > 650)) {
    r.lostTimer += dt;
    if (r.isPlayer && r.lostTimer > 0.05 && r.lostTimer - dt <= 0.05) hud.message('SIGNAL LOST', 1.2, true);
    if (r.lostTimer > 1.1) respawn(r);
  } else r.lostTimer = 0;
}

/** Rivals fight with the player's whole arsenal (WEAPON, the same values as the player's):
 *  - PULSE bolts (v0.2.15): 21 damage, 750 m/s for 1.5 s, from alternating sides of the bumper, the player's heat
 *    rules, but 3x slower (RIVAL_AIM.SLOWER: one bolt every 0.63 s, so they never overheat); they lead the target, off
 *    by up to RIVAL_AIM.SPREAD rad (x road.fire: better aim on later roads);
 *  - seekers (v0.2.16): 3 charges, one back every 4.5 s; a 0.5 s lock on the target, then launched within
 *    RIVAL_AIM.SEEKER_RANGE, at most one every RIVAL_AIM.SEEKER_GAP s;
 *  - spin dodge (v0.2.16): against a seeker homing on it (RIVAL_AIM.DODGE chance per seeker), every 6 s at most;
 *  - EMP (v0.2.16): when RIVAL_AIM.EMP_CARS other cars are within its 95 m, every 11 s at most.
 *  Target: the nearest car within RIVAL_AIM.CONE of the nose (you or another rival), out to the lock range (2 km);
 *  bolts only within their reach. No firing in the first 6 s; fireCD > 0 holds all fire (an EMP: 3 s). */
export const RIVAL_AIM = {
  SPREAD: 0.03, // rad: bolt aim error on road 1 (x road.fire)
  CONE: (35 * Math.PI) / 180, // how far off the nose a target may be (v0.2.16: 35°, was 17°)
  SLOWER: 3, // bolts this many times slower than the player's (0.63 s apart)
  SEEKER_RANGE: 600, SEEKER_GAP: 1.5, // m, s
  DODGE: 0.5, // chance to spin away from a seeker homing on it
  EMP_CARS: 2, // other cars within the EMP's radius to set it off
};
const _lead = new THREE.Vector3(), _up = new THREE.Vector3(0, 0.08, 0);
/** The nearest live car within the cone in front of r and `range` metres, and its distance. */
function rivalTarget(r, range) {
  let best = null, bd = Infinity;
  const cos = Math.cos(RIVAL_AIM.CONE);
  for (const o of g.racers) {
    if (o === r || !o.alive || o.finished) continue;
    _d.subVectors(o.car.pos, r.car.pos);
    const d = _d.length();
    if (d < 15 || d > range || _d.dot(r.car.fwd) / d < cos) continue;
    if (d < bd) { bd = d; best = o; }
  }
  return [best, bd];
}
export function rivalCombat(r, dt) {
  if (g.raceTime < 6 || r.finished || !r.alive || g.state !== 'race') return;
  const W = WEAPON;
  r.fireCD = Math.max(0, (r.fireCD || 0) - dt);
  r.boltCD = (r.boltCD || 0) - dt;
  r.missiles = Math.min(W.MISSILES, (r.missiles ?? W.MISSILES) + dt / W.MISSILE_RECHARGE);
  r.seekerCD = Math.max(0, (r.seekerCD || 0) - dt);
  r.spinCD = Math.max(0, (r.spinCD || 0) - dt);
  r.empCD = Math.max(0, (r.empCD || 0) - dt);
  rivalDodge(r);
  const [best, bd] = rivalTarget(r, W.LOCK_RANGE);
  // lock-on: the player's 0.5 s on the same target
  if (best !== r.lock) { r.lock = best; r.lockT = 0; }
  r.lockT = best ? Math.min(1, (r.lockT || 0) + dt / W.LOCK_TIME) : 0;
  const armed = r.fireCD <= 0;
  if (armed) rivalEmp(r);
  if (armed && best && r.lockT >= 1 && bd <= RIVAL_AIM.SEEKER_RANGE && r.missiles >= 1 && r.seekerCD <= 0) rivalSeeker(r, best);
  // bolts: within their reach
  const firing = armed && !!best && bd <= W.BOLT_SPEED * W.BOLT_LIFE;
  // heat: the player's rules (aim.js playerWeapons)
  r.heat = Math.max(0, (r.heat || 0) - W.COOL_RATE * dt * (r.overheated ? 1.4 : firing ? 0.35 : 1));
  if (r.overheated && r.heat <= 0.25) r.overheated = false;
  if (!firing || r.overheated || r.boltCD > 0) return;
  r.boltCD = W.BOLT_RATE * RIVAL_AIM.SLOWER;
  r.side = -(r.side || 1);
  const from = r.car.muzzle(r.side);
  const speed = Math.max(0, r.car.speed);
  // lead: where the target will be when the bolt gets there
  const t = from.distanceTo(best.car.pos) / (speed + W.BOLT_SPEED);
  const dir = _lead.copy(best.car.pos).addScaledVector(best.car.vel, t * 0.9).sub(from).normalize().clone();
  const err = RIVAL_AIM.SPREAD * road.fire;
  dir.x += (Math.random() - 0.5) * 2 * err;
  dir.y += (Math.random() - 0.5) * 2 * err;
  dir.z += (Math.random() - 0.5) * 2 * err;
  weapons.fire(r, from, dir, speed, null, {
    kind: 'bolt', speed: W.BOLT_SPEED, turn: 0, life: W.BOLT_LIFE, damage: W.BOLT_DMG, len: W.BOLT_LEN, wid: W.BOLT_WID, radius: 3.6,
  });
  r.heat += W.HEAT_PER_BOLT;
  if (r.heat >= 1) { r.heat = 1; r.overheated = true; }
  sound.zap(volAt(r.car.pos) * 0.25, panAt(r.car.pos));
}
/** A seeker on a full lock: the player's missile (aim.js playerWeapons). */
function rivalSeeker(r, tgt) {
  const W = WEAPON, speed = Math.max(0, r.car.speed);
  r.missiles -= 1;
  r.seekerCD = RIVAL_AIM.SEEKER_GAP;
  weapons.fire(r, r.car.muzzle(), r.car.fwd.clone().add(_up), speed, tgt, {
    kind: 'missile', mat: weapons.missileMat, speed: 60, accel: 420, maxSpeed: speed + 330, turn: 7, always: true,
    life: 3.6, damage: W.MISSILE_DMG, len: 12, wid: 6, radius: 5.5, trail: true,
  });
  sound.zap(volAt(r.car.pos) * 0.6, panAt(r.car.pos));
  if (tgt === player) hud.sub(`SEEKER INCOMING · ${r.name}`, 1.2);
}
/** Spin away from a seeker homing on r (once per seeker: a RIVAL_AIM.DODGE chance), when the spin is ready. */
function rivalDodge(r) {
  if (r.spinCD > 0 || !Array.isArray(weapons.shots)) return;
  for (const p of weapons.shots) {
    if (p.kind !== 'missile' || p.target !== r || p.dodgeTried) continue;
    if (p.pos.distanceTo(r.car.pos) > 120) continue;
    p.dodgeTried = true;
    if (Math.random() >= RIVAL_AIM.DODGE) continue;
    r.spinCD = SPIN.COOLDOWN;
    r.car.dodge(Math.random() < 0.5 ? -1 : 1);
    return;
  }
}
/** The player's EMP (aim.js playerWeapons), set off when enough cars are close. */
function rivalEmp(r) {
  const W = WEAPON;
  if (r.empCD > 0) return;
  const near = g.racers.filter((o) => o !== r && o.alive && !o.finished && o.car.pos.distanceTo(r.car.pos) <= W.EMP_RADIUS);
  if (near.length < RIVAL_AIM.EMP_CARS) return;
  r.empCD = W.EMP_COOLDOWN;
  weapons.shockwave(r.car.pos.clone(), W.EMP_RADIUS);
  weapons.purge(r.car.pos, W.EMP_RADIUS, r);
  sound.zap(volAt(r.car.pos), panAt(r.car.pos));
  for (const o of near) {
    _d.subVectors(o.car.pos, r.car.pos);
    const d = _d.length();
    if (!o.isPlayer) o.fireCD = (o.fireCD || 0) + 3;
    o.car.vel.addScaledVector(_d.normalize(), 30 * (1 - d / W.EMP_RADIUS) + 8);
    damage(o, W.EMP_DMG * (1 - 0.5 * d / W.EMP_RADIUS), r, 'emp');
    if (o === player) hud.sub(`EMP · ${r.name}`, 1.2);
  }
}

/** Sphere contacts between cars. */
export function carContacts() {
  const R = 4.6;
  for (let i = 0; i < g.racers.length; i++) {
    const a = g.racers[i];
    if (!a.alive) continue;
    for (let j = i + 1; j < g.racers.length; j++) {
      const b = g.racers[j];
      if (!b.alive) continue;
      _d.subVectors(a.car.pos, b.car.pos);
      const dist = _d.length();
      if (dist >= R || dist < 1e-3) continue;
      _d.divideScalar(dist);
      a.car.pos.addScaledVector(_d, (R - dist) / 2);
      b.car.pos.addScaledVector(_d, -(R - dist) / 2);
      const vrel = _d.dot(_tmp.subVectors(a.car.vel, b.car.vel));
      if (vrel < 0) {
        a.car.vel.addScaledVector(_d, -vrel * 0.6);
        b.car.vel.addScaledVector(_d, vrel * 0.6);
        if ((a.isPlayer || b.isPlayer) && -vrel > 5) {
          g.shake = Math.min(1.2, g.shake - vrel / 40);
          sound.hit(-vrel * 0.6);
        }
      }
    }
  }
}

/** Most extra speed a rival behind the player gets (a fraction of its top speed). */
export const RUBBER_BAND = 0.07;
export function updateRanking() {
  const prog = new Map(g.racers.map((r) => [r, r.progress(track)]));
  g.ranking = [...g.racers].sort((a, b) => {
    if (a.finished || b.finished) return (a.finished ? a.finishTime : Infinity) - (b.finished ? b.finishTime : Infinity);
    return prog.get(b) - prog.get(a);
  });
  g.position = g.rankOf(player);
  // rivals fly at the road's rival top speed (RIVAL_SPEED, roads.js; skill only shapes how they boost). A rival behind
  // the player gets up to 7% more to catch up; one ahead is never slowed down (v0.2.15: it used to lose up to 5%)
  const pp = prog.get(player);
  for (const r of rivals) {
    const gap = (pp - prog.get(r)) / (track.n * 0.15);
    r.car.maxMul = (rivalTop(road.id) / CAR.MAX) * (1 + THREE.MathUtils.clamp(gap * 0.05, 0, RUBBER_BAND));
  }
}

export function showResults() {
  if (g.state !== 'finished') return;
  const total = player.lapTimes.reduce((a, b) => a + b, 0);
  const bi = player.lapTimes.indexOf(Math.min(...player.lapTimes));
  $('res-total').textContent = fmt(total);
  const multi = g.racers.length > 1;
  $('res-place').textContent = multi ? `${ordinal(g.finishPos)} OF ${g.racers.length} · ${player.takedowns} TAKEDOWN${player.takedowns === 1 ? '' : 'S'}` : '';
  $('res-laps').innerHTML = player.lapTimes.map((t, i) => `<li class="${i === bi ? 'best' : ''}">LAP ${i + 1} — ${fmt(t)}</li>`).join('');
  // rivals still racing get a projected time from their average pace so far
  const rows = multi ? g.racers.map((r) => {
    if (r.finished) return { r, t: r.finishTime, est: false };
    const done = Math.max(1, r.progress(track));
    return { r, t: Math.max(g.raceTime, (g.raceTime * g.laps * track.n) / done), est: true };
  }) : [];
  rows.sort((a, b) => a.t - b.t);
  $('res-standings').innerHTML = rows.map(({ r, t, est }) =>
    `<li class="${r.isPlayer ? 'me' : ''}">${esc(r.name)}<span>${est ? '≈ ' : ''}${fmt(t)}</span></li>`).join('');
  const coinRows = [...g.racers].sort((a, b) => b.coins - a.coins);
  $('res-coins').innerHTML = coinRows.map((r, i) =>
    `<li class="${r.isPlayer ? 'me' : ''}">${i === 0 ? '♛ ' : ''}${esc(r.name)}<span>${r.coins}</span></li>`).join('');
  const coinPos = coinRows.indexOf(player) + 1;
  // your coins plus a podium bonus in the coin race go to your online wallet
  const bonus = multi ? [0, 15, 8, 4][coinPos] || 0 : 0;
  $('res-bank').textContent = `COIN RACE ${ordinal(coinPos)}${bonus ? ` · BONUS +${bonus}` : ''}`;
  // the time ranking only takes full 2-lap races in RACE mode
  const raceTime = g.mode === 'race' && player.lapTimes.length === 2 ? Math.round(total * 1000) / 1000 : null;
  account.recordRace(player.coins + bonus, Math.min(...player.lapTimes), $('res-online'), raceTime, multi ? g.finishPos : null);
  const opened = D.unlockNext();
  $('res-unlock').textContent = opened ? `🔓 ROAD ${opened.id} · ${opened.name} UNLOCKED` : '';
  $('res-unlock').classList.toggle('hidden', !opened);
  const next = road.id < D.roadsOpen();
  $('res-next').classList.toggle('hidden', !next);
  $('res-again').classList.toggle('primary', !next);
  trackEvent('race_finish', {
    mode: g.mode, position: multi ? g.finishPos : 1, coins: player.coins, takedowns: player.takedowns, race_seconds: Math.round(total),
  });
  $('results').classList.remove('hidden');
}
