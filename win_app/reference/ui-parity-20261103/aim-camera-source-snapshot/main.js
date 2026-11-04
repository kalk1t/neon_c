// Roadsters Race: start-up, the game state machine (menu, roads, countdown, race, pause, results, shop), the menus and
// settings, and the main loop. The rest of the game lives in its own modules (D1, v0.2.6):
//   render.js  renderer, post-processing, graphics quality      world.js   the map, roads, coins
//   race.js    race rules, damage, ranking, results               aim.js     mouse aim, lock-on, the player's weapons
//   camera.js  chase / cockpit / showcase cameras                 roam.js    FREE ROAM (online)
//   clock.js   the game clock
import * as THREE from 'three';
import { makeMaterials, makeCarEnvironment } from './materials.js';
import { GRID } from './track.js';
import { ROADS } from './roads.js';
import { DECK_Y } from './course.js';
import { buildSky } from './env.js';
import { Car, CAR, CAR_MODELS, loadCarModel, liveryModel } from './car.js';
import { Shop } from './shop.js';
import { Cockpit } from './cockpit.js';
import { Weapons, additive, tex } from './fx.js';
import { Racer, LIVERIES, DAMAGE, WEAPON, aiInput } from './ai.js';
import { Account } from './account.js';
import { Chat } from './chat.js';
import { KillerAura, auraStyleFor } from './killer.js';
import { analyticsAvailable, consent, setConsent, track as trackEvent } from './online/analytics.js';
import { Input } from './input.js';
import { TouchControls, isTouchDevice } from './touch.js';
import { HUD, fmt } from './hud.js';
import { Sound } from './audio.js';
import {
  initRender, renderer, scene, camera, composer, lens, FOG_DENSITY, GFX, gfx, setQuality, autoQuality, measureGfx,
  applyAtmosphere, volAt, panAt,
} from './render.js';
import {
  initWorld, buildWorld, setRoad, bestKey, map, road, track, tracks, colliders, clouds, course, scenery, ads, coins, showSpot,
} from './world.js';
import {
  initRace, targetOf, setGateText, respawn, damage, raceRules, rivalCombat, carContacts, updateRanking, showResults,
} from './race.js';
import { initAim, centreAim, updateAim, updateLock, playerWeapons, autopilot } from './aim.js';
import { initCamera, CAMS, setInterior, chaseCamera, cinematicCamera, showcaseCamera, show } from './camera.js';
import {
  initRoam, live, avatars, askToSignIn, ROAM_HUB, ROAM_SLOTS, roamPlace, startRoam, endRoam, roamHit, updateRoam,
  updateRoamStats, refreshOnlineCount,
} from './roam.js';
import { time, advance } from './clock.js';
import { muteCommand } from './online/mutes.js';

const $ = (id) => document.getElementById(id);

// ---------------- fatal errors ----------------
/** Replaces the loading screen with a message; used when the game can't run at all. */
function fatal(title, text) {
  $('loading')?.remove();
  $('fatal-title').textContent = title;
  $('fatal-text').textContent = text;
  $('fatal').classList.remove('hidden');
}
$('fatal-reload').addEventListener('click', () => location.reload()); // not an inline onclick: the CSP forbids those
// anything that breaks while the game is still loading would otherwise leave the loading bar stuck forever
addEventListener('error', (e) => { if ($('loading')) fatal('THE GAME COULD NOT START', `Please reload the page. (${e.message})`); });
addEventListener('unhandledrejection', (e) => {
  if ($('loading')) fatal('THE GAME COULD NOT START', `Please reload the page. (${e.reason?.message || e.reason})`);
});
if (!document.createElement('canvas').getContext('webgl2')) {
  fatal('3D GRAPHICS NOT AVAILABLE', 'Roadsters Race needs WebGL 2. Update your browser (Chrome, Edge or Firefox), and make sure hardware acceleration is turned on in its settings.');
  throw new Error('WebGL 2 not supported');
}

initRender(fatal);

// phones and tablets start on LOW graphics (the player can still switch)
const TOUCH = isTouchDevice();
if (TOUCH) document.body.classList.add('touch');
setQuality(localStorage.getItem('neon.gfx') || (TOUCH ? 'low' : 'high'));
$('gfx').addEventListener('change', (e) => {
  localStorage.setItem('neon.gfx', e.target.value);
  setQuality(e.target.value);
});
autoQuality(!(localStorage.getItem('neon.gfx') || TOUCH));

// ---------------- world ----------------
const M = makeMaterials();
const carEnv = makeCarEnvironment(renderer);
for (const m of [M.carPaint, M.steel, M.carChrome, M.carGun, M.fan, M.carGlass]) m.envMap = carEnv;
const sky = buildSky(scene, FOG_DENSITY);
const car = new Car(scene, M);
const weapons = new Weapons(scene, null);
let roadUI = null; // menu labels, minimap and best lap of the road (set once the HUD and game state exist)
initWorld({
  scene, M, weapons, applyAtmosphere: (a) => applyAtmosphere(a, sky), onCoin: (r, c) => onCoin(r, c), onRoad: () => roadUI?.(),
});
/** The car in the menu showcase spot. */
function toShowSpot() {
  player.reset(showSpot.pos, showSpot.yaw, track);
}
buildWorld('skyline');
const input = new Input();
// phones: tilt steering + on-screen buttons (see touch.js)
let touch = TOUCH ? new TouchControls(input, { onPause: () => { if (racing()) pause(); }, onChat: () => openChat() }) : null;
// FREE ROAM chat (Enter / the CHAT button)
const chat = new Chat({
  send: (s) => {
    // S6: /mute, /unmute and /muted stay on this device
    const muted = muteCommand(s, (name) => g.remotes.find((r) => r.name.toLowerCase() === name)?.name);
    if (muted) {
      chat.add(null, muted);
      return true;
    }
    // KILLER STATUS: /pkclear without a connection to the relay. Only the relay saves the status (v0.2.4), so the live
    // game asks to wait for the connection; the test backend (?mock) still clears it right here
    if (!live?.me && /^\/pkclear$/i.test(s)) {
      if (account.backend?.setKiller) clearKiller(null);
      else chat.add(null, 'NOT CONNECTED · /pkclear WORKS ONCE YOU ARE ONLINE AGAIN');
      return true;
    }
    return live?.chat(s) ?? false;
  },
  onSlow: () => hud.sub('SLOW DOWN', 0.8),
});
function openChat() {
  if (g.mode === 'roam' && g.state === 'race') chat.open();
}

// ---------------- KILLER STATUS (killer.js) ----------------
// 3 player kills in FREE ROAM make a Killer: the CRIMSON FLAME, from 10 the VOID CROWN (killer.js auraStyleFor) is on
// the car everywhere (FREE ROAM, races, menu, shop) and R can't respawn, until /pkclear.
// The status is on the profile (account.isKiller), so it survives mode changes, car switches, reloads and
// reconnects; the relay counts the kills (server/live.js).
/** How much aura the player's car draws: less in the menu showcase, least on LOW graphics. */
const playerAuraTier = () => (gfx === 'low' ? 'low' : g.state === 'menu' || g.state === 'shop' ? 'menu' : 'high');
const remoteAuraTier = () => (gfx === 'low' ? 'low' : 'remote');
/** Which aura our own car wears: by the saved Killer kill count. */
const playerAuraStyle = () => auraStyleFor(account.profile?.killerKillCount);
/** Keeps the player's aura in step with the profile (every frame: a cheap check, rebuilt only when it changes). */
function syncKiller() {
  const on = account.isKiller, tier = playerAuraTier(), style = playerAuraStyle();
  if (on !== car.isKiller || (on && (car.aura.tier !== tier || car.aura.style !== style))) car.setKiller(on, tier, style);
}
/** /pkclear: msg = the relay's answer (it knows if we were a Killer: its broadcast may already have cleared us). */
function clearKiller(msg) {
  const was = account.isKiller || /CLEARED/.test(msg ?? '');
  account.setKiller(false, 0);
  syncKiller();
  chat.add(null, msg ?? (was ? 'KILLER STATUS CLEARED' : 'KILLER COUNT RESET · YOU WERE NOT A KILLER'));
  if (was) hud.sub('KILLER STATUS CLEARED · R RESPAWN BACK', 2);
}
// a real mouse or keyboard means a computer (a touchscreen laptop, or a browser that reports touch): switch to them
if (touch) {
  const toDesktop = (e) => {
    if (e.type === 'pointermove' && e.pointerType !== 'mouse') return;
    touch.show(false);
    touch = null;
    if (window.NEON) NEON.touch = null;
    document.body.classList.remove('touch');
    if (!localStorage.getItem('neon.gfx')) setQuality('high');
    if (racing()) captureMouse();
    removeEventListener('pointermove', toDesktop);
    removeEventListener('keydown', toDesktop);
  };
  addEventListener('pointermove', toDesktop);
  addEventListener('keydown', toDesktop);
}
const hud = new HUD(track);
const sound = new Sound();
function showMapInfo() {
  hud.setTrack(track, { race: `ROAD ${road.id} · ${road.name}`, roam: map.roamName });
  $('track-name').textContent = `ROAD ${road.id} · ${road.name} · ${track.checkpoints.length} GATES`;
  $('track-len').textContent = `${(track.length / 1000).toFixed(2)} KM`;
}
showMapInfo();
$('app-version').textContent = `v${__APP_VERSION__.replace(/\.0$/, '')} · ONLINE`; // "0.1.0" -> "v0.1 · ONLINE"

// ---------------- racers ----------------
const player = new Racer(car, 'YOU', 0xffffff, true);
const rivals = LIVERIES.map((L) => {
  const r = new Racer(new Car(scene, M, { livery: L }), L.name, L.accent, false, L.skill);
  r.shotMat = additive(tex('projectile'), new THREE.Color(L.accent).lerp(new THREE.Color(1, 1, 1), 0.35).multiplyScalar(2.2));
  r.car.setVisible(false);
  return r;
});
// the sculpted car replaces the code-built ones once loaded (the built-in model stays if loading fails)
// (5-6 MB): the loading screen shows its progress; on a very slow connection the game starts after 20 s with the
// built-in cars and swaps the sculpted ones in when they arrive
// Shop cars (CAR_MODELS) load on first use: the ROADSTER now, others only once previewed in the shop or equipped.
const carLoads = {};
function loadCar(id, onProgress) {
  const spec = CAR_MODELS[id] ?? CAR_MODELS.roadster;
  carLoads[id] ??= loadCarModel(`${import.meta.env.BASE_URL}${spec.url}`, carEnv, onProgress, spec)
    .catch((e) => { delete carLoads[id]; throw e; }); // a failed download can be tried again next time
  return carLoads[id];
}
const sculptLoad = loadCar('roadster', (e) => {
  if (e.total && $('loading')) $('loading').querySelector('.ld-txt').textContent = `LOADING CAR ${Math.round((e.loaded / e.total) * 100)}%`;
})
  .then(({ hi, lo }) => {
    carModels = { hi, lo };
    rivals.forEach((r, i) => r.car.useModel(liveryModel(lo, LIVERIES[i]), LIVERIES[i].accent));
    paintPlayer();
  })
  .catch((e) => console.warn('sculpted car not loaded, using the built-in model', e));
let carModels = null; // the ROADSTER: rivals, and other players' cars until their own shop car has loaded

// ---------------- car colour ----------------
// -1 = the original paint, 0-9 = the rival liveries. In FREE ROAM everyone sees the colour you picked.
const PAINTS = [{ name: 'ORIGINAL', paint: 0x1a1030, accent: 0xff26d4 }, ...LIVERIES];
const validPaint = (c) => (Number.isInteger(c) && c >= -1 && c < LIVERIES.length ? c : -1);
let paint = validPaint(Number(localStorage.getItem('neon.paint') ?? -1));
let shownCar = ''; // `${car id}:${paint}` on the player's car right now
/** Puts the player's car on screen: the one previewed in the shop, otherwise the equipped one, in the chosen paint. */
async function paintPlayer() {
  const want = () => shop?.previewId ?? shop?.equipped ?? 'roadster';
  const id = want();
  let models;
  try {
    models = await loadCar(id);
  } catch (e) {
    console.warn(`car ${id} not loaded`, e);
    return;
  }
  if (id !== want() || shownCar === `${id}:${paint}`) return; // another car was picked meanwhile, or no change
  shownCar = `${id}:${paint}`;
  const P = PAINTS[paint + 1];
  car.useModel(paint < 0 ? models.hi : liveryModel(models.hi, P), P.accent);
  car.glow?.color.set(P.accent);
}
const hexCss = (c) => `#${c.toString(16).padStart(6, '0')}`;
PAINTS.forEach((P, i) => {
  const b = document.createElement('button');
  b.className = 'swatch';
  b.title = P.name;
  b.style.background = `radial-gradient(circle at 35% 35%, ${hexCss(P.paint)} 45%, ${hexCss(P.accent)} 100%)`;
  b.style.color = hexCss(P.accent);
  b.addEventListener('click', () => {
    paint = i - 1;
    localStorage.setItem('neon.paint', String(paint));
    showPaint();
    paintPlayer();
  });
  $('paints').appendChild(b);
});
function showPaint() {
  [...$('paints').children].forEach((b, i) => b.classList.toggle('on', i === paint + 1));
  $('paint-name').textContent = PAINTS[paint + 1].name;
  // our bolts and muzzle flash in the car's colour (v0.2.14), the same colour other players see them in FREE ROAM
  player.shotColor = new THREE.Color(PAINTS[paint + 1].accent).lerp(new THREE.Color(1, 1, 1), 0.35).multiplyScalar(2.2);
  player.shotMat = additive(tex('projectile'), player.shotColor);
}
showPaint();
const sculptReady = Promise.race([sculptLoad, new Promise((r) => setTimeout(r, 20000))]);
const PLAYER_SLOT = 7;
const SOLO_SPAWN = new THREE.Vector3(0, DECK_Y + 4.5, 42);
const gridPos = (slot) => new THREE.Vector3(GRID[slot].x, DECK_Y + 4.5, GRID[slot].z);

// ---------------- settings (mouse) ----------------
const settings = { sens: Number(localStorage.getItem('neon.sens')) || 1, invert: localStorage.getItem('neon.invert') === '1' };
$('sens').value = settings.sens;
$('invert').checked = settings.invert;
$('sens').addEventListener('input', (e) => localStorage.setItem('neon.sens', (settings.sens = Number(e.target.value))));
$('invert').addEventListener('change', (e) => localStorage.setItem('neon.invert', (settings.invert = e.target.checked) ? '1' : '0'));
const storedVol = localStorage.getItem('neon.vol');
sound.volume = storedVol == null ? 0.7 : Number(storedVol);
$('vol').value = sound.volume;
$('vol').addEventListener('input', (e) => {
  localStorage.setItem('neon.vol', e.target.value);
  sound.init();
  sound.setVolume(Number(e.target.value));
  sound.coin(0.8);
});

// ---------------- online account ----------------
const account = new Account({
  onClick: () => sound.init(),
  onChange: (s) => {
    player.name = s.profile?.nickname || 'YOU';
    shop.refresh(); // wallet, cars owned
    if (g.state === 'menu' || g.state === 'shop') paintPlayer(); // an equipped shop car shows once ownership is known
  },
});
// ---------------- SHOP ----------------
// the shop screen sits over the menu showcase: same car, same orbit camera, the menu card swapped for the garage
const shop = new Shop({
  account,
  onClick: () => sound.init(),
  onPreview: () => paintPlayer(),
  onEquip: () => paintPlayer(),
  onOpen: () => {
    g.state = 'shop';
    $('menu').classList.add('hidden');
    Object.assign(show, { yaw: 0.9, pitch: 0.12, dist: 11.5 });
  },
  onClose: () => {
    g.state = 'menu';
    $('menu').classList.remove('hidden');
    Object.assign(show, { yaw: 0.6, pitch: 0.14, dist: 10.5, swing: 0 });
  },
});
// analytics only after the visitor agrees (dev builds show the banner too, so it can be tested)
const askConsent = () => $('consent').classList.toggle('hidden', !(analyticsAvailable() || import.meta.env.DEV));
if (consent() === null) askConsent();
for (const [id, yes] of [['consent-yes', true], ['consent-no', false]]) {
  $(id).addEventListener('click', () => { setConsent(yes); $('consent').classList.add('hidden'); });
}
$('btn-cookies').addEventListener('click', (e) => { e.preventDefault(); askConsent(); });

// ---------------- game state ----------------
const g = {
  state: 'menu', mode: 'race', laps: 2, raceTime: 0, best: Number(localStorage.getItem(bestKey())) || null,
  countdown: 0, camMode: 0, shake: 0,
  heat: 0, overheated: false, boltCD: 0, side: 1, missiles: WEAPON.MISSILES, empCD: 0, spinCD: 0, lockT: 0, streak: 0, lastKill: -99,
  player, racers: [player], remotes: [], position: 1, ranking: [player], lock: null,
  aim: { yaw: 0, pitch: 0 }, aimDir: new THREE.Vector3(0, 0, -1), mouseLocked: false,
  targetPos: () => targetOf(player),
  rankOf: (r) => g.ranking.indexOf(r) + 1,
};
roadUI = () => {
  g.best = Number(localStorage.getItem(bestKey())) || null;
  $('menu-best').textContent = fmt(g.best);
  showMapInfo();
  account.rankRoad = road.id; // the TIME ranking opens on the road being raced
};
roadUI();
// cockpit interior view: one dashboard per shop car, remembered between visits
const dash = new Cockpit($('hud'));
// the split-out parts of the game (D1) get the objects made here
const deps = {
  g, player, car, rivals, hud, sound, weapons, input, account, shop, chat, M, settings, dash,
  get touch() { return touch; }, get paint() { return paint; }, get carModels() { return carModels; },
  loadCar, PAINTS, validPaint, syncKiller, clearKiller, remoteAuraTier, playerAuraStyle, roadsOpen, unlockNext,
};
initRace(deps);
initAim(deps);
initCamera(deps);
initRoam(deps);
hud.setCity(scenery.towers, ROAM_HUB);
weapons.onHit = (shot, victim) => {
  if (g.mode === 'roam') {
    // the shooter decides: a hit on the car as we see it counts (it reaches them as a 'hit' message). Other players'
    // shots here are just the look; theirs on us arrive the same way (see `hit` in startRoam).
    if (shot.owner === player && victim.remote) {
      // their hp is theirs to keep (it arrives with their position); the kill is confirmed by the server
      live?.hit(victim.remote, shot.kind, shot.vel.clone().normalize());
      hud.hitmark(false);
      sound.hitMark(shot.kind !== 'bolt');
    }
    return;
  }
  const dmg = shot.damage ?? (victim.isPlayer ? DAMAGE.RIVAL_SHOT_ON_PLAYER : DAMAGE.RIVAL_SHOT);
  damage(victim, dmg, shot.owner, shot.kind);
};
function onCoin(r, c) {
  weapons.impact(c.pos, false, r.isPlayer ? 12 : 8);
  if (r.isPlayer) {
    sound.coin();
    hud.coinPop();
  } else sound.coin(volAt(c.pos) * 0.4, panAt(c.pos));
}


function startRace(mode) {
  if (mode === 'roam' && !account.profile) return askToSignIn();
  endRoam();
  g.mode = mode;
  // the server stamps when the race began; the results are only saved with this ticket (see account.recordRace)
  if (mode !== 'roam') account.startRace(mode, road.id);
  document.body.classList.toggle('roam', mode === 'roam');
  dash.setCar(shop.equipped); // the cockpit of the car you drive (its art loads the first time you look inside)
  leaveShowcase();
  g.laps = mode === 'race' ? map.laps : Infinity;
  g.raceTime = 0;
  g.lastNitro = -9;
  Object.assign(g, { heat: 0, overheated: false, boltCD: 0, missiles: WEAPON.MISSILES, empCD: 0, spinCD: 0, lockT: 0, streak: 0, lastKill: -99, lock: null });
  coins.reset();
  coins.setVisible(mode === 'race');
  g.racers = mode === 'race' ? [player, ...rivals] : [player];
  weapons.clear();
  weapons.racers = g.racers;
  for (const r of rivals) r.car.setVisible(false);
  if (mode === 'race') {
    player.reset(gridPos(PLAYER_SLOT), 0, track);
    let slot = 0;
    for (const r of rivals) {
      if (slot === PLAYER_SLOT) slot++;
      r.reset(gridPos(slot++), 0, track);
    }
  } else if (mode === 'roam') {
    // a back-row slot until the server assigns ours (a moment later)
    roamPlace(ROAM_SLOTS - 4 + Math.floor(Math.random() * 4));
    startRoam();
  } else player.reset(SOLO_SPAWN, 0, track);
  centreAim();
  updateRanking();
  trackEvent('race_start', { mode });
  setGateText('START');
  hud.laps([], g.best);
  for (const id of ['menu', 'pause', 'results']) $(id).classList.add('hidden');
  hud.show(true);
  g.state = 'countdown';
  g.countdown = 3.999;
  g.lastCount = 4;
  if (mode === 'roam') {
    // free roam: no countdown, just fly
    g.state = 'race';
    g.lastCount = 0;
    touch?.calibrate();
  }
  sound.init();
  captureMouse();
}

function toMenu() {
  car.model.visible = true; // may be hidden by the first-person camera
  endRoam();
  document.body.classList.remove('roam');
  g.state = 'menu';
  car.showFX(false); // the showcase (back on when a race starts)
  Object.assign(show, { yaw: 0.6, pitch: 0.14, dist: 10.5, swing: 0 });
  input.unlock();
  hud.show(false);
  for (const id of ['pause', 'results']) $(id).classList.add('hidden');
  $('menu').classList.remove('hidden');
  $('menu-best').textContent = fmt(g.best);
  toShowSpot();
  for (const r of rivals) r.car.setVisible(false);
  weapons.clear();
  setGateText('START');
}

function pause() {
  car.model.visible = true; // may be hidden by the first-person camera
  g.pausedAt = performance.now();
  g.prevState = g.state;
  g.state = 'paused';
  chat.close();
  input.unlock();
  Object.assign(show, { yaw: 0.6, pitch: 0.14, dist: 9.5 });
  hud.show(false);
  car.showFX(false);
  $('pause').classList.remove('hidden');
}
function leaveShowcase() {
  car.showFX(true);
}
function resume() {
  g.state = g.prevState;
  leaveShowcase();
  hud.show(true);
  $('pause').classList.add('hidden');
  captureMouse();
}

document.querySelectorAll('#menu button[data-mode]').forEach((b) =>
  b.addEventListener('click', () => {
    sound.init();
    if (touch) touch.enable(); // tilt permission + full screen need this tap
    if (b.dataset.mode === 'roam') startRace('roam');
    else openRoads(b.dataset.mode);
  }),
);

// ---------------- roads (roads.js) ----------------
// RACE and TIME ATTACK start with choosing a road. Road 1 is open; each further road opens once the one before it is
// won in 1st place in RACE mode. Progress is kept on this device and, for signed-in players, on the profile (roadsWon,
// written by the server), so it follows them to other devices.
const ROAD_OPEN = 'neon.roadsOpen';
function roadsOpen() {
  const local = Number(localStorage.getItem(ROAD_OPEN)) || 1;
  return Math.min(ROADS.length, Math.max(1, local, (account.profile?.roadsWon ?? 0) + 1));
}
let roadsMode = 'race';
function openRoads(mode) {
  roadsMode = mode;
  $('roads-title').textContent = mode === 'race' ? 'RACE · CHOOSE YOUR ROAD' : 'TIME ATTACK · CHOOSE YOUR ROAD';
  const open = roadsOpen();
  $('road-list').replaceChildren(...tracks.map((t, i) => {
    const r = ROADS[i], locked = r.id > open;
    const best = Number(localStorage.getItem(bestKey(r.id))) || null;
    const b = document.createElement('button');
    b.className = `${r.id === road.id ? 'cur' : ''} ${locked ? 'locked' : ''}`;
    b.innerHTML = `<span class="num">${r.id}</span><span class="name">${locked ? '🔒 ' : ''}${r.name}</span>`
      + `<span class="diff">${'<i class="on"></i>'.repeat(r.id)}${'<i></i>'.repeat(ROADS.length - r.id)}</span>`
      + `<span class="info">${(t.length / 1000).toFixed(2)} KM · ${t.checkpoints.length} GATES · 2 LAPS</span>`
      + `<span class="best">${locked ? `WIN ROAD ${r.id - 1} IN 1ST` : best ? `BEST LAP ${fmt(best)}` : 'NO LAP YET'}</span>`;
    b.addEventListener('click', () => {
      sound.init();
      if (locked) { b.animate([{ transform: 'translateX(-6px)' }, { transform: 'translateX(6px)' }, { transform: 'none' }], 180); return; }
      pickRoad(r.id);
    });
    return b;
  }));
  $('roads').classList.remove('hidden');
  ($('road-list').querySelector('.cur:not(.locked)') ?? $('road-list').firstChild).focus({ preventScroll: true });
}
function closeRoads() {
  $('roads').classList.add('hidden');
}
function pickRoad(id) {
  closeRoads();
  if (id !== road.id) setRoad(id);
  startRace(roadsMode);
}
$('roads-close').addEventListener('click', () => { sound.init(); closeRoads(); });
addEventListener('keydown', (e) => { if (e.code === 'Escape') closeRoads(); });
/** After a race: a 1st place in RACE mode opens the next road. Returns the road it opened, if any. */
function unlockNext() {
  if (g.mode !== 'race' || g.finishPos !== 1 || road.id >= ROADS.length || road.id + 1 <= roadsOpen()) return null;
  localStorage.setItem(ROAD_OPEN, String(road.id + 1));
  trackEvent('road_unlock', { road: road.id + 1 });
  return ROADS[road.id];
}

document.querySelectorAll('[data-act]').forEach((b) =>
  b.addEventListener('click', () => {
    const a = b.dataset.act;
    if (a === 'resume') resume();
    if (a === 'restart') startRace(g.mode);
    if (a === 'next' && road.id < roadsOpen()) { setRoad(road.id + 1); startRace(g.mode); }
    if (a === 'quit') toMenu();
  }),
);
// Losing pointer lock mid-race (browser Esc) pauses; clicking the game re-captures the mouse.
document.addEventListener('pointerlockchange', () => {
  g.mouseLocked = input.locked;
  if (input.locked) centreAim();
  if (!input.locked && (g.state === 'race' || g.state === 'countdown')) pause();
});
renderer.domElement.addEventListener('click', () => {
  if (g.state === 'race' || g.state === 'countdown') captureMouse();
});

/** Capture the mouse for steering (not on phones: they steer by tilt). */
function captureMouse() {
  if (!touch) input.lock(renderer.domElement, racing);
}
function racing() {
  return g.state === 'race' || g.state === 'countdown';
}


// ---------------- main loop ----------------
const timer = new THREE.Timer();
const IDLE = { throttle: 0, brake: 0, steer: 0, climb: 0, boost: false };
let rankT = 0;

function stepRacer(r, dt, ctl) {
  if (!r.alive) {
    const left = Math.ceil(r.wreck);
    r.wreck -= dt;
    // FREE ROAM: the wrecked player counts down to the respawn
    if (r.isPlayer && g.mode === 'roam' && r.wreck > 0 && Math.ceil(r.wreck) !== left) hud.sub(`RESPAWN IN ${Math.ceil(r.wreck)}`, 1.1);
    if (r.wreck <= 0) respawn(r);
    return null;
  }
  r.prevPos.copy(r.car.pos);
  const res = r.car.update(dt, ctl, colliders);
  if (!r.finished) r.lapTime += dt;
  if (g.state === 'race' || r.finished || !r.isPlayer) raceRules(r, dt);
  if (r.hp < 60) {
    r.smokeT -= dt;
    if (r.smokeT <= 0) {
      weapons.smoke(r.car.pos.clone().addScaledVector(r.car.fwd, -2.5), (60 - r.hp) / 60);
      r.smokeT = 0.04 + r.hp / 500;
    }
  }
  return res;
}

function frame() {
  timer.update();
  const raw = timer.getDelta();
  const dt = Math.min(raw, 1 / 20);
  measureGfx(raw, g.state === 'menu');
  if (live) updateRoamStats(raw);
  advance(dt);
  const inp = touch && (g.state === 'race' || g.state === 'countdown') ? touch.state() : input.state();
  if (touch) {
    touch.invert = settings.invert;
    touch.show(g.state === 'race' || g.state === 'countdown');
  }

  if (input.hit('KeyM')) sound.setMuted(!sound.muted);
  if (input.hit('KeyC')) {
    if (g.interior) setInterior(false); // C leaves the cockpit for the chase camera it was on
    else g.camMode = (g.camMode + 1) % CAMS.length;
  }
  if (input.hit('KeyI') && (g.state === 'race' || g.state === 'countdown' || g.state === 'finished')) setInterior(!g.interior);
  if (input.hit('Escape', 'KeyP')) {
    if (g.state === 'race' || g.state === 'countdown') pause();
    else if (g.state === 'paused' && performance.now() - g.pausedAt > 400) resume();
  }
  if (g.state === 'menu' && input.hit('Enter') && $('roads').classList.contains('hidden')) openRoads('race');
  if (input.hit('Enter', 'KeyT')) openChat();

  const playing = g.state === 'race' || g.state === 'finished';
  if (g.state === 'countdown') {
    g.countdown -= dt;
    const n = Math.ceil(g.countdown);
    if (n !== g.lastCount) {
      g.lastCount = n;
      if (n > 0) { hud.message(String(n), 0.9); sound.countdown(false); }
      else { hud.message('GO!', 0.9); sound.countdown(true); g.state = 'race'; touch?.calibrate(); }
    }
    input.takeMouse();
    for (const r of g.racers) r.car.update(dt, IDLE, colliders);
  } else if (playing) {
    g.raceTime += dt;
    // player
    const ctl = g.state === 'finished' || g.autopilot ? autopilot() : updateAim(inp);
    const wasBoost = car.boosting;
    const res = stepRacer(player, dt, ctl);
    g.steerCtl = ctl.steer;
    if (car.boosting && !wasBoost && g.raceTime - (g.lastNitro ?? -9) > 1) {
      // nitro ignition: camera punch + shake, edge flash, HUD call-out (not again if a bump re-lights it at once)
      g.lastNitro = g.raceTime;
      sound.boostOn();
      g.kick = 1;
      g.shake = Math.min(1.2, g.shake + 0.35);
      hud.flash('rgba(25, 230, 255, 0.35)');
      hud.nitro();
    } else if (!car.boosting && wasBoost) sound.boostOff();
    if (res && res.impact > 4) {
      g.hits = (g.hits || 0) + 1;
      g.lastHit = car.pos.clone();
      g.shake = Math.min(1.2, g.shake + res.impact / 25);
      sound.hit(res.impact);
      weapons.impact(car.pos.clone().addScaledVector(res.n, -2), true, 6 + res.impact * 0.2);
    }
    // rivals
    for (const r of g.racers) {
      if (r.isPlayer) continue;
      const go = g.raceTime > r.react;
      stepRacer(r, dt, go ? aiInput(r, track, g.racers, targetOf(r), time, dt, coins.ahead(r)) : IDLE);
      rivalCombat(r, dt);
    }
    carContacts();
    if ((rankT -= dt) <= 0) {
      updateRanking();
      rankT = 0.2;
    }
    if (g.state === 'race') {
      if (player.alive) {
        updateLock(dt);
        playerWeapons(dt, ctl);
      } else g.lock = null;
      if (input.hit('KeyR') && player.alive) {
        if (account.isKiller) hud.sub('RESPAWN UNAVAILABLE WHILE KILLER', 1.4);
        else respawn(player, g.mode === 'roam');
      }
    }
  }

  // debug/screenshot hook: a fixed camera (set from the console / test scripts only)
  if (g.debugCam) {
    camera.position.copy(g.debugCam.pos);
    camera.lookAt(g.debugCam.look);
    camera.fov = g.debugCam.fov || 50;
    camera.updateProjectionMatrix();
  } else if (g.state === 'menu' || g.state === 'paused' || g.state === 'shop') {
    // main menu, pause and shop: the car showcase
    car.idle(dt);
    showcaseCamera(dt);
  } else chaseCamera(dt);

  updateRoam(dt);
  syncKiller();
  if (g.state !== 'paused') {
    for (const r of g.racers) r.car.updateFX(camera.position);
    for (const r of g.remotes) r.car.updateFX(camera.position);
    weapons.update(dt, camera.position);
    coins.update(dt, time, g.racers, playing && g.mode === 'race');
    course.update(time, g.state === 'menu' || g.state === 'shop' ? null : player);
    scenery.update(time);
    ads.update(time);
  }
  sky.update(time, camera.position);
  clouds.update(time, camera.position);
  sound.engine(Math.max(0, car.speed) / CAR.MAX, car.boosting, (playing || g.state === 'countdown') && player.alive);
  if (g.state !== 'menu' && g.state !== 'shop') hud.update(dt, g, camera);
  if (g.state === 'race' || g.state === 'countdown' || g.state === 'finished') {
    dash.update({
      speed: Math.hypot(car.speed, car.side ?? 0) * 3.6, // forwards and sideways (a slide counts too)
      boost: car.boost, lap: Math.min(player.lap, g.laps), laps: g.mode === 'roam' ? 0 : g.laps,
      time: g.mode === 'roam' ? g.raceTime : player.lapTime, heat: g.heat, overheated: g.overheated, steer: (g.steerVis += ((playing ? g.steerCtl ?? 0 : 0) - g.steerVis) * Math.min(1, dt * 8)),
    });
  } else dash.show(false);
  document.body.classList.toggle('interior', g.interior && dash.visible);
  lens.uniforms.uTime.value = time;
  if (!GFX[gfx].blur) lens.uniforms.uSpeed.value = 0;
  composer.render(dt);
  input.endFrame();
  requestAnimationFrame(frame);
}

refreshOnlineCount();

// warm up shaders before revealing (rivals visible so their materials compile too), once the car model is in
sculptReady.then(() => {
  player.reset(SOLO_SPAWN, 0, track);
  rivals.forEach((r, i) => r.reset(gridPos(i), 0, track));
  cinematicCamera(0, 0);
  renderer.compile(scene, camera);
  for (const r of rivals) r.car.setVisible(false);
  toShowSpot();
  requestAnimationFrame(() => {
    $('loading').remove();
    $('menu').classList.remove('hidden');
    car.showFX(false); // the menu shows the car like the pause showcase
    frame();
  });
});

// debug handle for the browser tests and screen captures (both run on the dev server). Never in a production build
// (D9 / security C1): it would hand the console the game state, the account and the weapons.
if (import.meta.env.DEV) window.NEON = {
  g, car, player, rivals, camera, touch, startRace, respawn, input, account, show, pause, roamHit, weapons, chat, showResults, toMenu,
  get track() { return track; }, get ads() { return ads; }, get scenery() { return scenery; }, get course() { return course; }, get colliders() { return colliders; }, get coins() { return coins; }, get map() { return map; },
  get live() { return live; }, get road() { return road; }, get tracks() { return tracks; }, setRoad, openRoads, roadsOpen,
  KillerAura, avatars: () => avatars, clearKiller, shop,
};
