# 7. Components and functions

[← Index](TECHNICAL_DOCUMENTATION.md)

Line counts and exports are [confirmed] (`wc -l`, `grep export`). Each file's role comes from its header comment and its exports.

## 7.1 Client: game core

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/main.js` | ~700 | (entry) `window.NEON` (dev builds only since v0.2.4) | Start-up; game state machine (menu, roads, countdown, race, pause, results, shop); menus and settings; car paint and loading; KILLER STATUS sync; main loop. Split in v0.2.6 (D1) |
| `src/render.js` | ~150 | `initRender`, `renderer`, `scene`, `camera`, `lens`, `GFX`, `setQuality`, `measureGfx`, `applyAtmosphere`, `volAt`, `panAt` | Renderer, post-processing, graphics quality, map atmosphere |
| `src/world.js` | ~95 | `initWorld`, `buildWorld`, `setRoad`, `bestKey`, live bindings `map`, `road`, `track`, `course`, `coins`… | The map and its nine roads |
| `src/race.js` | ~290 | `initRace`, `GATE`, `raceRules`, `respawn`, `damage`, `wreck`, `rivalCombat`, `carContacts`, `updateRanking`, `showResults` | Race rules and results |
| `src/aim.js` | ~200 | `initAim`, `centreAim`, `updateAim`, `updateLock`, `playerWeapons`, `autopilot` | Mouse aim (the car flies to the aim, forwards and backwards: `inp.aimSteer`, v0.2.12), lock-on, the player's weapons |
| `src/camera.js` | ~150 | `initCamera`, `cam`, `CAMS`, `show`, `setInterior`, `chaseCamera`, `showcaseCamera`, `cinematicCamera` | Cameras; with the mouse captured the chase camera follows the aim (`g.camAim`, v0.2.12) |
| `src/roam.js` | ~410 | `initRoam`, `live`, `avatars`, `startRoam`, `endRoam`, `updateRoam`, `roamHit`, `roamSpawn`… | FREE ROAM (avatars wear each player's shop car since v0.2.4) |
| `src/clock.js` | 7 | `time`, `advance` | The game clock |
| `src/car.js` | 818 | `Car`, `CAR`, `SPIN`, `CAR_MODELS`, `loadCarModel`, `liveryModel`, `sculptShader` | Player and rival car: flight model (speed, boost, climb, roll), spin dodge, GLB model loading and livery shader, nitro and ribbons, killer aura attachment |
| `src/ai.js` | 117 | `Racer`, `aiInput`, `LIVERIES`, `RIVAL_POWER`, `DAMAGE`, `WEAPON`, `COIN`, `AI_FIRE_INTERVAL` | 10 rival liveries with skill (boost eagerness only since v0.2.3; top speed = player × `road.ai`), `RIVAL_POWER` 1.5 on rival damage and fire rate; AI steering along the track, rubber band (in `race.js`), weapon and coin constants |
| `src/physics.js` | 162 | `OBB`, `Cyl`, `Torus`, `Colliders` | Static collision world: uniform grid (`CELL` 120 m), swept segment tests (3 m steps) |
| `src/track.js` | 155 | `Track`, `Corridor`, `SAMPLE_STEP`, `GRID` | Road spline (sampled every 4 m), start grid (12 slots), corridor grid (`CELL` 160) used to keep scenery off the road |
| `src/roads.js` | 128 | `ROADS` | 9 roads: id, name, control points, AI speed factor `ai`, fire factor `fire`, ring count |
| `src/maps.js` | 24 | `MAPS` | The single map: laps 2, floor 34, roamFloor 40, atmosphere options |
| `src/course.js` | 251 | `buildCourse` | Race furniture: gates, path dots (7 m), chevrons (84 m), beacons (125 m), barriers |
| `src/coins.js` | 128 | `Coins` | Track coins (instanced), pickup, respawn |
| `src/fx.js` | 314 | `Weapons`, `Ribbon`, `tex`, `additive`, `axisBillboard` | Projectiles (bolts, seekers, EMP), explosions, trails |
| `src/killer.js` | 366 | `KillerAura`, `KILLER_AURA` | Killer auras: `AURA_STYLES` CRIMSON FLAME and VOID CROWN (adds a crown sprite and orbiting crystal shards); `auraStyleFor(kc)`: VOID CROWN from 10 Killer kills; tiers high / menu / remote / low |
| `src/nitro.js` | 199 | `NitroFX`, `flameMaterial` | Thruster flames, 140 sparks |

## 7.2 Client: world and visuals

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/city.js` | 313 | `buildCity`, `facadeBox`, `HERO_ASPECT` | Procedural city (seed 1337, extent 2,700 m, base −260 m), hero towers |
| `src/env.js` | 228 | `buildSky`, `buildClouds`, `MOON_DIR` | Sky shader, cloud sea / mist (900 puffs by default) |
| `src/skyprops.js` | 246 | `buildSkyProps`, `loadModel`, `dressScenery` | Flying props (drones, taxis, blimps, islands, koi) on looped paths clear of the track (seed 4242) |
| `src/props.js` | 295 | prefabs, `RING` | Gates, rings, deck, towers, signs, trees |
| `src/ads.js` | 408 | `buildAds` | Animated billboard screens (canvas atlas 768×384 slides, 7 s each, max 48 screens) |
| `src/materials.js` | 344 | `makeMaterials`, `rng`, colors, textures | Shared materials, seeded RNG, canvas textures, environment maps |
| `src/geo.js` | 97 | `Prefab`, geometry helpers | Geometry normalisation and merging |

## 7.3 Client: UI and input

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/hud.js` | 396 | `HUD` | Speed, boost, heat, missiles, lap times, sub-titles (`sub()` is plain text since v0.2.7, C2), kill feed, health bars |
| `src/chat.js` | 76 | `Chat`, `CHAT_MAX` | FREE ROAM chat log and input (120 chars, own pace 1.2 s) |
| `src/shop.js` | 251 | `Shop` | 3D showcase, buy, equip (`neon.car`), NEW badge |
| `src/cockpit.js` | 194 | `Cockpit` | Cockpit view overlays (art 1672×941) |
| `src/touch.js` | 176 | `TouchControls` | Phone controls: tilt steering (24°), climb (18°), dead zone 2.5° |
| `src/input.js` | 132 | `Input` | Keyboard, mouse, gamepad (deadzone 0.15); mouse buttons re-read from `e.buttons` on every captured movement, so none can stay stuck (v0.2.12) |
| `src/online/mutes.js` | 41 | `isMuted`, `mutedNames`, `muteCommand` | FREE ROAM `/mute`, `/unmute`, `/muted` on this device (localStorage `neon.muted`, v0.2.9) |
| `src/audio.js` | 245 | `Sound` | Procedural Web Audio (no sound files), 18 voices |
| `src/sysreq.js` | 89 | `showRequirements` | Minimum requirements and device check |

## 7.4 Client: online layer

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/account.js` | ~470 | `Account` | Single source of account state for the UI: sign-in, nickname dialog, races (`startRace`, `recordRace`, `flushPending`), unsaved-kills display (saving only with the mock backend), `setKiller` (shows; the relay saves), buying, ranks; 15 s timeouts; friendly errors |
| `src/online/index.js` | ~45 | `loadBackend`, `NICK_RE` (re-exported from protocol.js), `RACE_COIN_CAP`, `KILL_CAP` / `KILL_SAVE_GAP_MS` (mock only), `SHOP_CARS`, `timeField` | Backend choice and shared limits |
| `src/online/firebase.js` | 207 | `createFirebase` | Auth, visit record, profile snapshot, nickname transactions, callables (15 s), kill award transaction, killer update, leaderboards, ranks, App Check (optional) |
| `src/online/mock.js` | 112 | `createMock` | `?mock` backend (dev only): `localStorage neon.mockdb`, 10,000 test coins, test profile starts as a Killer |
| `src/online/live.js` | 242 | `LiveRoom`, `onlineCount` | WebSocket client: send at 15/s, interpolation (0.15 s delay), ping, reconnect with backoff, protocol mismatch reload |
| `src/online/protocol.js` | ~20 | `LIVE_PROTOCOL` (7), `KILLER_KILLS` (3), `VOID_KILLS` (10), `NICK_RE`, `CHAT_MAX`, `CAR_IDS` | Shared constants (client and relay) |
| `src/online/analytics.js` | 47 | `setConsent`, `track`, `identify` | Consent-gated GA events (queue of 50 before consent) |

## 7.5 Server: relay (`server/live.js`)

| Function | Lines | Responsibility |
|---|---|---|
| `killerOf(x, kc)` | 67-71 | Sanitise a saved killer status |
| `liveRelay({projectId, allowMock, log, perIp, perIpMinute, maxPending, mod})` | ~100-460 | Creates the `WebSocketServer` (`noServer`, `maxPayload` 4 KB); `identify(token)` (JWKS verify, REST profile read); message handlers `s`, `f`, `die`, `ping`, `hit`, `chat` (+ `command`: `/pkclear`, `/report`, `/help`), `hi`; hit, shot and wreck checks through `server/hits.js` (S3), chat/name filter and bans through `mod` (S6); 15/s snapshot tick; 15 s heartbeat with idle drop |
| `livePlugin(env)` | end of file | Vite plugin for the **dev** server: hooks `upgrade` for `/live` only (leaves HMR alone), serves `GET /live/count`; mock logins only with `LIVE_MOCK=1`. `killSaver()` builds the `relayKills` saver from `RELAY_SECRET`. `liveRelay` queues and saves kills / KILLER STATUS (`queueSave`, `save`, `flush`) |
| `server/serve.mjs` | ~150 | **Production server** (v0.2.4, R5/R6): static `dist/` with cache headers, `/__` proxy, `/live` + `/live/count` passed to the relay; forks and restarts `relay.mjs` |
| `server/relay.mjs` | ~45 | The relay as its own process on `127.0.0.1:LIVE_RELAY_PORT`; reads `.env.local`; exits when `serve.mjs` does |
| `server/hits.js` | 96 | `remember`, `firedFrom`, `shot`, `hitFrom`, `empReaches`, `prune`: FREE ROAM hit checks (S3, v0.2.10): a hit only from a real shot, from where the shooter was, within reach and in the line of fire |
| `server/moderation.js` | 70 | `moderation`, `wordFilter`, `normal`, `noModeration`: chat and nickname word filter, ban list (S6, v0.2.9), files re-read within 10 s |
| `functions/tickets.js` | ~20 | `cleanTickets(db, now)`: deletes race tickets older than 45 min (daily `cleanRaceTickets`, D12) |

## 7.6 Cloud Functions (`functions/index.js`)

| Function | Line | Input | Output | Errors |
|---|---|---|---|---|
| `startRace` | 32 | `{mode: 'race'\|'attack', road: 1..9}` (bad values default to `attack` / 1) | `{ticket}` | `unauthenticated`, `resource-exhausted 'too-fast'` |
| `finishRace` | 48 | `{ticket, coins, bestLap, raceTime, place}` | `{gain, coins, totalCoins}` | `invalid-argument 'no-ticket'`, `failed-precondition 'no-profile' \| 'no-ticket' \| 'expired'`, `resource-exhausted 'too-soon'` |
| `buyCar` | 86 | `{id: 'viper'\|'titan'}` | `{coins}` | `invalid-argument 'unknown'`, `already-exists 'owned'`, `failed-precondition 'no-profile' \| 'funds'` |

Global options: region us-central1, `maxInstances` 10, memory 256 MiB, `cors: true`, `enforceAppCheck` from `ENFORCE_APP_CHECK` (`functions/index.js:22-25`).

## 7.7 Tools

| Tool | Purpose |
|---|---|
| `tools/roadgen/` (`gen.mjs`, `emit.mjs`, `check.mjs`, `metrics.mjs`, `roads.json`) | Offline generation and checking of the 9 roads (writes into `src/roads.js`) [inferred from file names; not run while writing this] |
| `docs/tools/capture-screens.mjs` | Screenshots for the docs (Playwright + Chrome) |
| `tools/fx/voidaura.mjs` | Builds the VOID CROWN textures `public/fx/voidaura_*.webp` from `neon_c/Assets/06_Effects` (recolour + crown cut-out, Playwright Chromium) |
