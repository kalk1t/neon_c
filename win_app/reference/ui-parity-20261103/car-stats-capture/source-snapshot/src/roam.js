// FREE ROAM: the online city. Other players' cars, spawn slots, the relay connection (online/live.js), hits, wrecks
// and kills, the roster and stats, and the menu's online count. Split from main.js (D1).
import * as THREE from 'three';
import { Car, CAR_MODELS, liveryModel } from './car.js';
import { additive, tex } from './fx.js';
import { Racer, LIVERIES, WEAPON } from './ai.js';
import { LiveRoom, onlineCount } from './online/live.js';
import { AURA_STYLES, auraStyleFor } from './killer.js';
import { VOID_KILLS } from './online/protocol.js';
import { track, map, course } from './world.js';
import { scene, volAt, panAt } from './render.js';
import { time } from './clock.js';
import { wreck, dodged } from './race.js';
import { centreAim } from './aim.js';
import { isMuted } from './online/mutes.js';

const $ = (id) => document.getElementById(id);
const _d = new THREE.Vector3(), _tmp = new THREE.Vector3();
let g, player, car, hud, sound, chat, weapons, account, shop, M, D;
/**
 * deps from main.js: g, player, car, hud, sound, chat, weapons, account, shop, M (materials), and the paint / car
 * helpers: paint and carModels (getters), loadCar, PAINTS, validPaint, syncKiller, clearKiller, remoteAuraTier,
 * playerAuraStyle.
 */
export function initRoam(deps) {
  D = deps;
  ({ g, player, car, hud, sound, chat, weapons, account, shop, M } = deps);
}

// Signed-in players share the city: fly anywhere (no laps), see each other, bump into each other and shoot. Hits knock
// you about and cost hp; at 0 you're wrecked, the shooter gets the kill and you respawn at the hub. Kills are counted per
// session (from joining to leaving) and banked to your profile for the KILLS world ranking. Each other player gets a car
// from a pool (the rival cars, plus extra ones beyond ten) and is drawn from the network a fraction of a second in the
// past. The shooter decides whether a shot hit (on the car as they see it) and tells the victim, so hits land where cars
// are drawn; the victim still applies spawn protection and spin dodges.
export let live = null;
export const avatars = []; // other players' cars (built on demand, repainted in each player's colour)
export function askToSignIn() {
  $('menu').classList.remove('hidden');
  account.toast(account.state.user ? 'CHOOSE A NICKNAME TO DRIVE ONLINE' : 'SIGN IN WITH GOOGLE TO DRIVE ONLINE');
  if (account.state.user) account.askNick(false);
  else account.signIn();
}
/** Another player's shop car in their colour on avatar r, once its model has loaded (the ROADSTER until then). */
function dressAvatar(r, id, c) {
  const put = (lo) => r.car.useModel(c < 0 ? lo.clone() : liveryModel(lo, LIVERIES[c]), r.color);
  if (id === 'roadster') {
    if (D.carModels) put(D.carModels.lo);
    return;
  }
  D.loadCar(id)
    .then(({ lo }) => { if (r.carId === id && r.paint === c) put(lo); }) // still the same look by the time it loaded
    .catch((e) => console.warn(`car ${id} not loaded for another player`, e));
}
/** A free car for another player: their shop car (D11, v0.2.4), in the colour they picked. */
export function takeAvatar(c, id = 'roadster') {
  c = D.validPaint(c);
  if (!CAR_MODELS[id]) id = 'roadster';
  let r = avatars.find((a) => !a.remote);
  if (!r) {
    r = new Racer(new Car(scene, M, { livery: LIVERIES[Math.max(0, c)] }), '', 0, false);
    r.carId = 'roadster';
    avatars.push(r);
  }
  if (r.paint !== c || r.carId !== id) {
    r.paint = c;
    r.carId = id;
    r.car.useStats(id); // their car's health, EMP reach (CAR_STATS)
    r.color = D.PAINTS[c + 1].accent;
    r.shotMat = additive(tex('projectile'), new THREE.Color(r.color).lerp(new THREE.Color(1, 1, 1), 0.35).multiplyScalar(3)); // x 3 (v0.2.14)
    dressAvatar(r, id, c);
  }
  return r;
}
// FREE ROAM spawn: one hub in the sky above the start deck; players line up around it in a formation of slots
// (4 across, rows behind, every other row a little higher). The server hands every player their own slot.
export const ROAM_HUB = new THREE.Vector3(0, 175, 70);
export const ROAM_SLOTS = 24;
function roamSlotPos(slot) {
  const k = slot % ROAM_SLOTS, row = Math.floor(k / 4), col = k % 4;
  return new THREE.Vector3(ROAM_HUB.x + (col - 1.5) * 14, ROAM_HUB.y + (row % 2) * 5, ROAM_HUB.z + row * 17);
}
/** First free slot from `slot` on that no other car is sitting in right now. */
export function roamSpawn(slot) {
  for (let i = 0; i < ROAM_SLOTS; i++) {
    const p = roamSlotPos(slot + i);
    if (!g.remotes.some((r) => r.car.root.visible && r.car.pos.distanceTo(p) < 9)) return { pos: p, yaw: 0 };
  }
  return { pos: roamSlotPos(slot), yaw: 0 };
}
/** Put the player on a spawn slot (and make it where R / a reset brings them back). */
export function roamPlace(slot) {
  const { pos, yaw } = roamSpawn(slot);
  player.reset(pos, yaw, track);
  player.lastSafe = { pos: pos.clone(), yaw };
  g.roamSpawnAt = pos.clone();
  g.roamSafeUntil = time + ROAM_SAFE;
  centreAim();
}
export function startRoam() {
  course.setRoam(true);
  g.remotes = [];
  setLiveStatus('CONNECTING…', false);
  live = new LiveRoom((fresh) => account.backend?.idToken?.(fresh), {
    appCheck: () => account.backend?.appCheckToken?.() ?? null,
    paint: () => D.paint,
    car: () => shop?.equipped ?? 'roadster', // the others see the car you drive
    status: (text, ok, code) => {
      setLiveStatus(text, ok);
      if (code) hud.message(text, 3, true);
      if (code === 'banned') chat.add(null, 'YOU ARE BANNED FROM FREE ROAM · RACES STILL WORK');
    },
    join(p) {
      const r = takeAvatar(p.c, p.car);
      p.racer = r;
      r.remote = p;
      r.name = p.name;
      r.hp = r.maxHp;
      r.wreck = 0;
      r.kills = p.kills;
      r.car.setKiller(p.killer, D.remoteAuraTier(), auraStyleFor(p.kc));
      r.car.setVisible(false); // shown once their first position arrives
      g.remotes.push(r);
      hud.sub(`${p.name} JOINED`, 1.6);
      // not for the players already here when we connected (they all arrive right after our welcome)
      if (performance.now() - g.welcomeAt > 1500) chat.add(null, `${p.name} joined`);
      renderRoster();
    },
    leave(p) {
      const r = p.racer;
      if (!r) return;
      r.car.setVisible(false);
      r.car.setKiller(false); // the pooled car may go to a player who isn't one
      r.remote = null;
      g.remotes = g.remotes.filter((x) => x !== r);
      if (g.lock === r) g.lock = null;
      if (live && !live.closed) {
        hud.sub(`${p.name} LEFT`, 1.6);
        chat.add(null, `${p.name} left`);
      }
      renderRoster();
    },
    fire: remoteFire,
    kill: roamKill,
    // test logins have no profile on the relay's side: they tell it their saved status
    saved: () => (account.backend?.kind === 'mock' ? { x: account.isKiller, kc: account.profile?.killerKillCount ?? 0 } : {}),
    killer(m) {
      if (m.id === live?.me?.id) {
        const was = account.isKiller, wasKc = account.profile?.killerKillCount ?? 0;
        account.setKiller(m.on, m.kc);
        D.syncKiller();
        if (m.on && was && wasKc < VOID_KILLS && m.kc >= VOID_KILLS) {
          hud.message('VOID CROWN', 2.2, true);
          hud.sub(`${VOID_KILLS} KILLS · THE CRIMSON FLAME BECOMES THE VOID CROWN`, 3.5);
          chat.add(null, `VOID CROWN: ${VOID_KILLS} Killer kills. /pkclear still clears it`);
          sound.hitMark?.(true);
        }
        if (m.on && !was) {
          hud.message('KILLER', 2.2, true);
          hud.sub(`${AURA_STYLES[D.playerAuraStyle()].name} · R RESPAWN LOCKED · TYPE /pkclear TO CLEAR`, 3.5);
          chat.add(null, 'You are a KILLER: 3 takedowns. R respawn is locked until you type /pkclear');
          sound.hitMark?.(true);
        }
        return;
      }
      const r = live?.players.get(m.id)?.racer;
      if (!r) return;
      r.car.setKiller(m.on, D.remoteAuraTier(), auraStyleFor(r.remote?.kc ?? m.kc));
      if (m.on) hud.feed(`${r.name} IS A KILLER`);
    },
    cmd(m) {
      if (m.c === 'pkclear' && m.ok) return D.clearKiller(m.msg);
      chat.add(null, m.msg);
    },
    hit(p, m) {
      if (p.racer) roamHit(m.k, p.racer, new THREE.Vector3(...m.d).normalize());
    },
    chat(m) {
      const me = m.id === live?.me?.id;
      if (!me && isMuted(m.name)) return; // muted on this device (S6)
      const r = me ? null : live?.players.get(m.id)?.racer;
      chat.add(m.name, m.s, { me, color: r ? `#${new THREE.Color(r.color).getHexString()}` : null });
      if (!me) sound.hitMark?.(false);
    },
    // our own slot: move there, unless we have already flown off
    welcome(m) {
      g.welcomeAt = performance.now();
      if (g.roamSpawnAt && car.pos.distanceTo(g.roamSpawnAt) < 30) roamPlace(m.slot);
    },
  });
  renderRoster();
}
export function endRoam() {
  // the session's kills are already banked one by one (account.addKills); show the tally on the menu
  if (live?.me) showRoamTally(live.me.kills);
  course.setRoam(false);
  live?.close();
  live = null;
  chat.clear();
  g.remotes = [];
  weapons.racers = g.racers;
}
function setLiveStatus(text, ok) {
  $('live-status').textContent = text;
  $('live-panel').classList.toggle('ok', ok);
  if (ok !== g.liveOk) {
    g.liveOk = ok;
    renderRoster();
  }
}
/** Who is driving, most kills this session first (you included), in their car colours. */
function renderRoster() {
  const list = $('live-list');
  const rows = g.remotes.map((r) => ({ name: r.name, kills: r.remote?.kills ?? 0, color: `#${new THREE.Color(r.color).getHexString()}` }));
  if (live?.me) rows.push({ name: `${live.me.name} (YOU)`, kills: live.me.kills, color: '#ffffff', me: true });
  rows.sort((a, b) => b.kills - a.kills || !!b.me - !!a.me);
  list.replaceChildren(...rows.map(({ name, kills, color, me }) => {
    const li = document.createElement('li');
    li.style.color = color;
    if (me) li.className = 'me';
    const n = document.createElement('span'), k = document.createElement('b');
    n.textContent = name; // nicknames only ever go through textContent
    k.textContent = `${kills}`;
    li.append(n, k);
    return li;
  }));
}
/** After a FREE ROAM session: its kills and the lifetime total on the menu. */
function showRoamTally(kills) {
  const total = (account.profile?.kills ?? 0) + account.pendingKills; // saved + still to save
  $('menu-kills').textContent = `LAST FREE ROAM ${kills} KILL${kills === 1 ? '' : 'S'}`;
  $('menu-kills').append(Object.assign(document.createElement('span'), { textContent: ` · LIFETIME ${total}` }));
  $('menu-kills').classList.remove('hidden');
}

/** Someone else fired: show it here too (their shots can hit us; we decide). */
function remoteFire(p, m) {
  const r = p.racer;
  if (!r) return;
  const o = new THREE.Vector3(...m.o), d = new THREE.Vector3(...m.d);
  const W = WEAPON;
  if (m.k === 'emp') {
    const R = W.EMP_RADIUS * r.car.stats.emp; // their car's EMP reach
    weapons.shockwave(o, R);
    sound.damage(volAt(o), panAt(o));
    _d.subVectors(car.pos, o);
    const dist = _d.length();
    if (dist < R && inCity()) roamHit('emp', r, _d.normalize(), 1 - 0.5 * dist / R);
    return;
  }
  if (m.k === 'spin') { r.car.barrelRoll(m.d[0]); return; } // just the look: they handle their own dodge
  const speed = Math.max(0, r.car.speed);
  const tg = m.tg == null ? null : m.tg === live?.me?.id ? player : live?.players.get(m.tg)?.racer || null;
  if (m.k === 'bolt') {
    weapons.fire(r, o, d, speed, tg, {
      kind: 'bolt', mat: r.shotMat, speed: W.BOLT_SPEED, turn: 2.5, life: W.BOLT_LIFE, len: W.BOLT_LEN, wid: W.BOLT_WID, radius: 3.6,
    });
  } else if (m.k === 'missile') {
    weapons.fire(r, o, d, speed, tg, {
      kind: 'missile', mat: weapons.missileMat, speed: 60, accel: 420, maxSpeed: speed + 330, turn: tg ? 7 : 0, always: true,
      life: 3.6, len: 12, wid: 6, radius: 5.5, trail: true,
    });
    if (tg === player) hud.sub(`SEEKER INCOMING · ${r.name}`, 1.2);
  }
}

// how hard a hit knocks you in free roam: [speed kept, stun seconds, push m/s, damage]
const ROAM_KNOCK = {
  bolt: [0.9, 0.25, 6, WEAPON.BOLT_DMG], missile: [0.45, 1.0, 26, WEAPON.MISSILE_DMG], emp: [0.6, 1.2, 34, WEAPON.EMP_DMG],
};
const ROAM_WRECK = 3; // seconds down after being wrecked (the respawn cooldown)
export const ROAM_SAFE = 3; // seconds untouchable after (re)spawning, so nobody can camp the spawn hub
const ROAM_REGEN = [5, 8]; // health comes back after this many seconds without a hit, at this many hp per second
/** We got hit in free roam: knocked about and damaged; at 0 hp we're wrecked and the shooter gets the kill. */
/** In the city and driving, or paused there: FREE ROAM doesn't stop for a pause, so a paused car can be hit too (S5). */
const inCity = () => g.state === 'race' || (g.state === 'paused' && g.prevState === 'race');
export function roamHit(kind, from, dir, k = 1) {
  if (!inCity() || !player.alive || time < (g.roamSafeUntil || 0)) return;
  if (car.dodging) return dodged();
  const [keep, stun, push, dmg] = ROAM_KNOCK[kind] || ROAM_KNOCK.bolt;
  player.hp -= dmg * k;
  if (player.hp <= 0) return roamWreck(from);
  const st = car.stats; // a heavy car loses less speed, is pushed less and stunned shorter (v0.2.17)
  car.speed *= 1 - Math.min(1, (1 - keep) * k * st.knock);
  car.stun = Math.max(car.stun, stun * k * st.stun);
  car.vel.addScaledVector(dir, push * k * st.knock);
  g.shake = Math.min(1.2, g.shake + (kind === 'bolt' ? 0.15 : 0.7) * k);
  hud.flash(kind === 'bolt' ? 'rgba(255, 60, 90, 0.3)' : 'rgba(255, 60, 90, 0.55)');
  sound.hit(kind === 'bolt' ? 6 : 18);
  // name the shooter, but not for every bolt of a burst
  if (kind !== 'bolt' || g.hitBy !== from || time - (g.hitAt || 0) > 2) {
    hud.sub(`${kind === 'emp' ? 'EMP' : kind === 'missile' ? 'SEEKER' : 'HIT'} BY ${from.name}`, 1.2);
  }
  g.hitBy = from;
  g.hitAt = time;
}
/** Our hp ran out in free roam: explode, tell the server who did it, respawn at the hub after ROAM_WRECK. */
function roamWreck(from) {
  wreck(player, from);
  player.wreck = ROAM_WRECK;
  hud.sub(`RESPAWN IN ${ROAM_WRECK}`, 1.1);
  live?.die(from.remote);
}
/** Someone was wrecked in free roam (the server's word: either of them may be us). */
function roamKill(m) {
  const me = live?.me?.id;
  const victim = m.v === me ? player : live?.players.get(m.v)?.racer;
  const killer = m.k === me ? player : live?.players.get(m.k)?.racer;
  if (victim && victim !== player) {
    weapons.explode(victim.car.pos.clone());
    sound.explosion(volAt(victim.car.pos), panAt(victim.car.pos));
    victim.car.setVisible(false); // back once their respawn arrives (hp > 0)
    victim.hp = 0;
    victim.wreck = 1; // shots fly through until then
    victim.downUntil = time + 1.2; // their own "hp 0" can still be on its way
    if (g.lock === victim) g.lock = null;
  }
  if (killer === player) {
    g.streak = g.raceTime - g.lastKill < 8 ? g.streak + 1 : 1;
    g.lastKill = g.raceTime;
    player.takedowns = m.n;
    player.car.boost = Math.min(1, player.car.boost + 0.5);
    g.missiles = Math.min(car.stats.missiles, g.missiles + 1);
    hud.hitmark(true);
    hud.message(['TAKEDOWN', 'TAKEDOWN', 'DOUBLE TAKEDOWN', 'TRIPLE TAKEDOWN'][g.streak] || `${g.streak}x RAMPAGE`, 1.4);
    hud.sub(`${victim?.name || 'PLAYER'} WRECKED · ${m.n} KILL${m.n === 1 ? '' : 'S'} THIS SESSION`, 1.8);
    account.addKills(1);
  } else if (killer && victim !== player) hud.feed(`${killer.name} ✖ ${victim?.name || '?'}`);
  // another Killer's count went up: the 10th kill turns their CRIMSON FLAME into the VOID CROWN
  if (killer && killer !== player && killer.remote?.killer) {
    const style = auraStyleFor(killer.remote.kc);
    if (killer.car.aura?.style !== style) {
      killer.car.setKiller(true, D.remoteAuraTier(), style);
      if (style === 'void') hud.feed(`${killer.name} WEARS THE VOID CROWN`);
    }
  }
  renderRoster();
}

const ROAM_CEIL = 1600, ROAM_RADIUS = 9000; // the floor is per map (maps.js roamFloor)
/** FREE ROAM stats under the roster: frame rate, ping to the server, time online this session (4 times a second). */
const roamStats = { t: 0, frames: 0, time: 0 };
export function updateRoamStats(raw) {
  roamStats.frames++;
  roamStats.time += raw;
  if ((roamStats.t -= raw) > 0) return;
  roamStats.t = 0.25;
  if (roamStats.time > 0) $('ls-fps').textContent = Math.round(roamStats.frames / roamStats.time);
  roamStats.frames = roamStats.time = 0;
  const ping = live?.me && live.ping != null ? Math.round(live.ping) : null;
  const el = $('ls-ping');
  el.textContent = ping ?? '–';
  el.className = ping == null ? '' : ping < 80 ? 'good' : ping < 160 ? 'mid' : 'bad';
  const s = live?.since == null ? 0 : Math.floor((performance.now() - live.since) / 1000);
  const h = Math.floor(s / 3600), m = Math.floor(s / 60) % 60, sec = String(s % 60).padStart(2, '0');
  $('ls-time').textContent = h ? `${h}:${String(m).padStart(2, '0')}:${sec}` : `${m}:${sec}`;
}
export function updateRoam(dt) {
  if (!live) return;
  live.send(car, g.state === 'race' && player.alive, player.alive ? (100 * player.hp) / player.maxHp : 0); // health in % (cars differ, v0.2.17)
  // health comes back slowly once nobody has hit us for a while
  if (player.alive && time - (g.hitAt ?? -99) > ROAM_REGEN[0]) player.hp = Math.min(player.maxHp, player.hp + ROAM_REGEN[1] * dt);
  for (const r of g.remotes) {
    const d = live.sample(r.remote);
    if (!d) continue;
    r.hp = ((d[10] ?? 100) / 100) * r.maxHp; // sent in % of their car's health
    // wrecked: stays hidden until their respawn arrives
    if (r.hp <= 0 || time < (r.downUntil ?? 0)) {
      if (r.car.root.visible) r.car.setVisible(false);
      r.wreck = 1;
      continue;
    }
    r.wreck = 0;
    if (!r.car.root.visible) {
      r.car.place(new THREE.Vector3(d[0], d[1], d[2]), d[3]);
      r.car.setVisible(true);
    }
    r.car.follow(d, dt);
  }
  weapons.racers = [player, ...g.remotes];
  if (g.state === 'race') {
    // bump into other players (each side pushes only itself, so both feel it)
    const R = 4.6;
    for (const r of g.remotes) {
      if (!r.car.root.visible) continue;
      _d.subVectors(car.pos, r.car.pos);
      const dist = _d.length();
      if (dist >= R || dist < 1e-3) continue;
      _d.divideScalar(dist);
      car.pos.addScaledVector(_d, R - dist);
      const vrel = _d.dot(_tmp.subVectors(car.vel, r.car.vel));
      if (vrel < 0) {
        car.vel.addScaledVector(_d, -vrel * 1.2);
        car.speed = Math.min(car.speed, Math.max(0, car.vel.dot(car.fwd)) + 4);
        if (-vrel > 5) {
          g.shake = Math.min(1.2, g.shake - vrel / 40);
          sound.hit(-vrel * 0.6);
          weapons.impact(car.pos.clone().addScaledVector(_d, -2.3), false, 6 + -vrel * 0.2);
        }
      }
    }
    // fly anywhere: just stay above the cloud floor and inside a (very) large sky
    if (car.pos.y < map.roamFloor) { car.pos.y = map.roamFloor; car.vel.y = Math.max(0, car.vel.y); }
    if (car.pos.y > ROAM_CEIL) { car.pos.y = ROAM_CEIL; car.vel.y = Math.min(0, car.vel.y); }
    const h = Math.hypot(car.pos.x, car.pos.z);
    if (h > ROAM_RADIUS) {
      car.pos.x *= ROAM_RADIUS / h;
      car.pos.z *= ROAM_RADIUS / h;
    }
  }
  if (live.me) setLiveStatus(`ONLINE · ${live.online} DRIVER${live.online === 1 ? '' : 'S'}`, true);
}

// the menu shows how many people are in FREE ROAM right now
export async function refreshOnlineCount() {
  if (g.state === 'menu' && !document.hidden) {
    const n = await onlineCount();
    $('roam-count').textContent = n ? `${n} DRIVING NOW` : 'SIGN-IN';
  }
  setTimeout(refreshOnlineCount, 10000);
}
