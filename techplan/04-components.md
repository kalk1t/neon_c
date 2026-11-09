# 7. Components and functions

[← Index](TECHNICAL_DOCUMENTATION.md)

Line counts and exports are [confirmed] (`wc -l`, `grep export`). Each file's role comes from its header comment and its exports.

## 7.1 Client: game core

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/main.js` | ~700 | (entry) `window.NEON` (dev builds only since v0.2.4) | Start-up; game state machine (menu, roads, countdown, race, pause, results, shop); menus and settings; car paint and loading; KILLER STATUS sync; main loop. Split in v0.2.6 (D1) |
| `src/render.js` | ~190 | `initRender`, `renderer`, `scene`, `camera`, `lens`, `GFX`, `setQuality`, `measureGfx`, `adaptResolution`, `applyAtmosphere`, `volAt`, `panAt` | Renderer, post-processing, graphics quality, dynamic resolution (v0.3.4), map atmosphere |
| `src/world.js` | ~95 | `initWorld`, `buildWorld`, `setRoad`, `bestKey`, live bindings `map`, `road`, `track`, `course`, `coins`… | The map and its nine roads |
| `src/race.js` | ~390 | `initRace`, `GATE`, `raceRules`, `respawn`, `damage`, `wreck`, `rivalCombat`, `RIVAL_AIM`, `RUBBER_BAND`, `GATE_R`, `carContacts`, `updateRanking`, `showResults` | Race rules and results; rivals' weapons (bolts v0.2.15; seekers, spin dodge v0.2.16; their EMP v0.2.16-0.2.20; slower rival seekers v0.2.20); backflip, air brake and free mouse pitch (`FLIP`, `AIRBRAKE`, `PITCH` in `car.js`, `FREE_PITCH` in `aim.js`, v0.2.22), `RESPAWN_SAFE` protection after a wreck, exact positions (`updateRanking` every frame; `Racer.progress` in `ai.js`) (v0.2.20) |
| `src/aim.js` | ~220 | `initAim`, `centreAim`, `updateAim`, `updateLock`, `playerWeapons`, `autopilot`, `FREE_PITCH`, `AIM_PICK` | Mouse aim (the car flies to the aim, forwards and backwards: `inp.aimSteer`, v0.2.12; free pitch, loops by hand with an auto-level, v0.2.22), the target under the aim ring (`updateLock`; v0.3.8, a sticky lock-on before), the player's weapons (`playerWeapons(dt, inp, keys)`: keys = false runs only the timers, FREE ROAM's Esc menu, v0.2.26; blaster, seeker on Q / middle mouse since v0.3.9 (1 before), spin dodge on F, backflip on G since v0.2.25), `driveHold` (v0.3.9, hold E in FREE ROAM: land / take off) |
| `src/camera.js` | ~160 | `initCamera`, `cam`, `CAMS`, `show`, `setInterior`, `chaseCamera`, `showcaseCamera`, `cinematicCamera` | Cameras; with the mouse captured the chase camera follows the aim (`g.camAim`, v0.2.12), at any pitch with its up vector turned over (loops, v0.2.22) |
| `src/roam.js` | ~410 | `initRoam`, `live`, `avatars`, `startRoam`, `endRoam`, `updateRoam`, `roamHit`, `roamSpawn`… | FREE ROAM (avatars wear each player's shop car since v0.2.4) |
| `src/clock.js` | 7 | `time`, `advance` | The game clock |
| `src/car.js` | ~990 | `Car`, `CAR`, `SPIN`, `FLIP`, `AIRBRAKE`, `PITCH`, `DRIVE`, `CAR_STATS`, `CAR_MODELS`, `loadCarModel`, `liveryModel`, `sculptShader`, `isAuthored`, `LIGHT` | Player and rival car: flight model (speed, boost, climb, roll; free pitch toward `inp.pitchTo`, v0.2.22), spin dodge, backflip (`flip` / `flipStep` / `flipClear`, v0.2.22), air brake (v0.2.22), drive mode (v0.3.9: `mode` fly / landing / drive / rising, `driveHeld`, `grounded`, `groundK`, `surfaceBelow`, `toggleDrive`, `groundStep`, `groundDrop`; wheels: each pod's `_wheel` child rolls and steers), sideways slide, per-model muzzle at the owner's marked shooting spots (`car.muzzle(side)`, v0.2.14, spots v0.2.19), `car.modelId` found by the model file (painted clones too, v0.2.19), GLB model loading (authored materials from the v0.2.27 car files: emissive light mask, region maps, baked normals, clear coat on the normal map; the v0.2.26 material for files without an emissive texture), livery shader (per-copy materials, freed by `releaseModel`, v0.2.27), nitro (light ribbons removed v0.3.2), thruster flames at each model's marked nozzles (`turbineFlame`, `flames`, pod-mounted ROADSTER side flames, v0.2.18), killer aura attachment |
| `src/ai.js` | ~195 | `Racer`, `aiInput`, `LIVERIES`, `WEAPON`, `COIN`, `CORNER`, `cornerSpeeds` | 10 rival liveries with skill (boost eagerness only since v0.2.3; top speed = player × `road.ai`), `RIVAL_POWER` 1.5 on rival damage and fire rate; AI steering along the track, rubber band (in `race.js`), weapon and coin constants |
| `src/physics.js` | 253 | `OBB`, `Cyl`, `Torus`, `Compound`, `Colliders` | Static collision world: uniform grid (`CELL` 120 m), swept segment tests (3 m steps). v0.3.6: `Compound` (many boxes in one placed frame), custom moving colliders (`{ bounds, resolve }`), `ground(x, z)` |
| `src/track.js` | 155 | `Track`, `Corridor`, `SAMPLE_STEP`, `GRID` | Road spline (sampled every 4 m), start grid (12 slots), corridor grid (`CELL` 160) used to keep scenery off the road |
| `src/roads.js` | 128 | `ROADS`, `RIVAL_SPEED`, `rivalTop` | 9 roads: id, name, control points, rival aim factor `fire`, ring count; rival top speed per road (v0.2.15) |
| `src/maps.js` | 24 | `MAPS` | The single map: laps 2, floor 34, roamFloor 40, atmosphere options |
| `src/course.js` | 251 | `buildCourse` | Race furniture: gates, path dots (7 m), chevrons (84 m), beacons (125 m), barriers |
| `src/coins.js` | 128 | `Coins` | Track coins (instanced), pickup, respawn |
| `src/fx.js` | 434 | `Weapons`, `tex`, `additive`, `axisBillboard`, `BOLT_LOOKS` | Projectiles (bolts, seekers); each car's bolt: a 3D model (`public/models/bolts/<id>.glb`, all loaded at start), muzzle flash and impact in its art (`BOLT_LOOKS`, v0.2.19), explosions, trails, engine smoke (`exhaustSmoke`, v0.2.18) |
| `src/killer.js` | 366 | `KillerAura`, `KILLER_AURA` | Killer auras: `AURA_STYLES` CRIMSON FLAME and VOID CROWN (adds a crown sprite and orbiting crystal shards); `auraStyleFor(kc)`: VOID CROWN from 10 Killer kills; tiers high / menu / remote / low |
| `src/nitro.js` | 199 | `NitroFX`, `flameMaterial` | Thruster flames, 140 sparks |

## 7.2 Client: world and visuals

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/cityplan.js` | 145 | `planCity`, `KINDS`, `GROUND`, `C`, `PITCH`, `T`, `SPORT`, `FARM`, `STADIUM`, `GOLF`, `HILLS`, `hillH`, `groundHeight` | v0.3.5: the new city's plan (grid, lot types, building placement clear of the roads); pure data, node-tested. v0.3.6: arenas (`SPORT.ARENA`), the ground's height for collisions |
| `src/newcity.js` | 1,263 | `NEWCITY`, `buildNewCity`, `GROUND`, `groundHeight` | v0.3.5: the Living Megacity: ground shader, horizon, lamps, trees, props, traffic, BatchedMesh mid/far forms, near forms with interiors, shader warm-up, dev warps. v0.3.6: simulated traffic with models, traffic lights, balloons and drones, park / pool / stadium props, all solid |
| `src/trafficsim.js` | 184 | `TRAFFIC`, `DT`, `Q`, `CYCLE`, `signal`, `VEH`, `TYPES`, `RESPAWN_MS`, `trafficSim` | v0.3.7: the city traffic as a deterministic simulation (0.1 s ticks, 1/256 grid, integer-hash light offsets): `advance`, `where`, `lerpU`, `snapshot`/`restore`, `checksum`. Shared by the game (`newcity.js` `buildTraffic` draws it) and the relay |
| `server/traffic.js` | 88 | `cityTraffic`, `CHECK_TICKS`, `SNAP_GAP_MS`, `WRECKS_BURST`, `WRECK_GAP_MS` | v0.3.7: the relay's city: runs while anyone is online, checksums every 50 ticks, checks hits on city cars (`hits.js` against the relay's own car track), health, wrecks (hidden 60 s), coins with the rate bucket |
| `src/stadiumgame.js` | 83 | `MATCH`, `TEAMS`, `TEAM_COLOR`, `STAD`, `inStadium`, `ballSpot`, `teamSpawn`, `stepBall` | v0.3.7 STADIUM 3V3 rules and geometry (v0.3.8: `stepBall`, the moving ball), shared by the game and the relay (no three.js) |
| `server/stadium.js` | 103 | `stadiumMatch` | v0.3.7: the match on the relay (pure, clock injected): teams (`toggle`, `remove`, `friendly`), the ball's health and goals (`hit`), states idle → countdown → play → results (`update`), `view` for `sm` |
| `src/stadium.js` | 167 | `initStadium`, `onMatch`, `onJoin`, `onGoal`, `stadiumKey`, `ballHit`, `updateStadium`, `teamOf`, `myStadiumTeam` | v0.3.7: the ball (mesh, beam, sphere collider), the score panel, B to join, team spawns (`g.matchSpawn`) |
| `src/megacity.js` | 283 | `MEGACITY`, `SITES`, `ROAM_DECK`, `nearLevel`, `dress`, `stageColliders`, `WARPS` | The 12 buildings' models, colliders, BLD-12 and the FREE ROAM roof hub. Since v0.3.5 on in every build (the old city is kept as a backup: dev server with `neon.megacity` = `off` only) |
| `src/facades.js`, `src/interiors.js`, `src/trees.js` | 300 / 393 / 165 | `facadeMaterial`, `releaseMaterial`, `buildInteriors`, `pauseInteriors`, `Scatter`, `forest`, `modelParts` | See-through facades with per-building tints, real interiors (3 ms/frame build queue), instanced trees and props by distance |
| `src/city.js` | 313 | `buildCity`, `facadeBox`, `HERO_ASPECT` | The old procedural city (seed 1337, extent 2,700 m, base −260 m), hero towers; kept in the files as a backup (dev server, megacity off) |
| `src/env.js` | 228 | `buildSky`, `buildClouds`, `MOON_DIR` | Sky shader, cloud sea / mist (900 puffs by default) |
| `src/skyprops.js` | ~280 | `buildSkyProps`, `loadModel`, `dressScenery` | Flying props (drones, taxis, blimps, islands, koi) on looped paths clear of the track (seed 4242); Meshy tower crowns, control tower and trees; crowns and trees in LOD cells (`lodInstances`, v0.3.4) |
| `src/props.js` | 295 | prefabs, `RING` | Gates, rings, deck, towers, signs, trees |
| `src/ads.js` | 408 | `buildAds` | Animated billboard screens (canvas atlas 768×384 slides, 7 s each, max 48 screens) |
| `src/materials.js` | 344 | `makeMaterials`, `rng`, colors, textures | Shared materials, seeded RNG, canvas textures, environment maps |
| `src/geo.js` | 97 | `Prefab`, geometry helpers | Geometry normalisation and merging |

## 7.3 Client: UI and input

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/hud.js` | 396 | `HUD` | Speed, boost, heat, missiles, lap times, sub-titles (`sub()` is plain text since v0.2.7, C2), kill feed, health bars |
| `src/chat.js` | 76 | `Chat`, `CHAT_MAX` | FREE ROAM chat log and input (120 chars, own pace 1.2 s) |
| `src/shop.js` | ~340 | `Shop` | 3D showcase, buy, equip (`neon.car`), NEW badge; per-car stat bars (`statRows`, v0.2.17); skins: chips, preview, buy, wear (`neon.skin.<car>`, v0.2.28); nitros: SKINS / NITROS tabs, fit (`neon.nitro.<car>`, v0.3.2) |
| `src/nitrostyle.js` | ~230 | `NitroStyle`, `NitroEnvelope`, `loadNitroPack`, `nitroManifest` | v0.3.2: shop nitro effect packs (`public/vfx/<car>/`) on a car model: mounts on body / rear pods (ROADSTER) or body only (VIPER, TITAN), mixer, off / flame / boost (ROADSTER: the four clips; VIPER, TITAN: filtered NitroLoop + envelope controller), distance LOD |
| `src/interior.js` | ~330 | `CockpitInterior`, `loadInteriorPack`, `interiorManifest`, `routeOf`, `COCKPIT` | v0.3.3: shop interiors (`public/interiors/<car>/`) in the player's car (ROADSTER, VIPER, TITAN; one master GLB per car + colour finish + own steering wheel `wheels/iXX.glb` on `CTRL_Steer`): glass cutaway of `hi_body` (`glass.json` per car), CTRL_* poses from `src/interiorkinematics.js` (pack copy), 10 Hz canvas screens; the cockpit view (I) and the shop's INTERIORS tab. Replaces `src/cockpit.js` (removed) |
| `src/touch.js` | 176 | `TouchControls` | Phone controls: tilt steering (24°), climb (18°), dead zone 2.5° |
| `src/input.js` | ~135 | `Input` | Keyboard, mouse, gamepad (deadzone 0.15); A / D give `strafe` while the mouse is captured (v0.2.13); mouse buttons re-read from `e.buttons` on every captured movement, so none can stay stuck (v0.2.12) |
| `src/online/mutes.js` | 41 | `isMuted`, `mutedNames`, `muteCommand` | FREE ROAM `/mute`, `/unmute`, `/muted` on this device (localStorage `neon.muted`, v0.2.9) |
| `src/audio.js` | 245 | `Sound` | Procedural Web Audio (no sound files), 18 voices |
| `src/sysreq.js` | ~95 | `showRequirements` | Minimum requirements and device check; integrated GPUs: LOW recommended (v0.3.4) |

## 7.4 Client: online layer

| Module | Lines | Key exports | Responsibility |
|---|---|---|---|
| `src/account.js` | ~470 | `Account` | Single source of account state for the UI: sign-in, nickname dialog, races (`startRace`, `recordRace`, `flushPending`), unsaved-kills display (saving only with the mock backend), `setKiller` (shows; the relay saves), buying, ranks; 15 s timeouts; friendly errors |
| `src/online/index.js` | ~45 | `loadBackend`, `NICK_RE` (re-exported from protocol.js), `RACE_COIN_CAP`, `KILL_CAP` / `KILL_SAVE_GAP_MS` (mock only), `SHOP_CARS`, `SHOP_SKINS` / `SKIN_PRICE` / `shopItem` / `ownedSkins` (v0.2.28), `timeField` | Backend choice and shared limits |
| `src/online/firebase.js` | 207 | `createFirebase` | Auth, visit record, profile snapshot, nickname transactions, callables (15 s), kill award transaction, killer update, leaderboards, ranks, App Check (optional) |
| `src/online/mock.js` | 112 | `createMock` | `?mock` backend (dev only): `localStorage neon.mockdb`, 10,000 test coins, test profile starts as a Killer |
| `src/online/live.js` | 242 | `LiveRoom`, `onlineCount` | WebSocket client: send at 15/s, interpolation (0.15 s delay), ping, reconnect with backoff, away while the tab is hidden (v0.2.24), protocol mismatch reload |
| `src/online/protocol.js` | ~20 | `LIVE_PROTOCOL` (17), `KILLER_KILLS` (3), `VOID_KILLS` (10), `NICK_RE`, `CHAT_MAX`, `CAR_IDS` | Shared constants (client and relay) |
| `src/online/analytics.js` | 47 | `setConsent`, `track`, `identify` | Consent-gated GA events (queue of 50 before consent) |

## 7.5 Server: relay (`server/live.js`)

| Function | Lines | Responsibility |
|---|---|---|
| `killerOf(x, kc)` | 67-71 | Sanitise a saved killer status |
| `liveRelay({projectId, allowMock, log, perIp, perIpMinute, maxPending, mod})` | ~100-460 | Creates the `WebSocketServer` (`noServer`, `maxPayload` 8 KB, `MAX_HI`); `identify(token)` (JWKS verify, REST profile read); message handlers `s`, `f`, `ping`, `hit`, `chat` (+ `command`: `/pkclear`, `/report`, `/help`), `hi`; hit and shot checks through `server/hits.js` (S3), health, dodges and wrecks through `server/health.js` (S4, v0.3), chat/name filter and bans through `mod` (S6); 15/s snapshot tick; 15 s heartbeat with idle drop |
| `livePlugin(env)` | end of file | Vite plugin for the **dev** server: hooks `upgrade` for `/live` only (leaves HMR alone), serves `GET /live/count`; mock logins only with `LIVE_MOCK=1`. `killSaver()` builds the `relayKills` saver from `RELAY_SECRET`. `liveRelay` queues and saves kills / KILLER STATUS (`queueSave`, `save`, `flush`) |
| `server/serve.mjs` | ~210 | **Production server** (v0.2.4, R5/R6): static `dist/` with cache headers, `/__` proxy, `/live` + `/live/count` passed to the relay; forks and restarts `relay.mjs` |
| `server/relay.mjs` | ~45 | The relay as its own process on `127.0.0.1:LIVE_RELAY_PORT`; reads `.env.local`; exits when `serve.mjs` does |
| `server/hits.js` | 96 | `remember`, `firedFrom`, `shot`, `hitFrom`, `prune`: FREE ROAM hit checks (S3, v0.2.10): a hit only from a real shot, from where the shooter was, within reach and in the line of fire |
| `server/moderation.js` | 70 | `moderation`, `wordFilter`, `normal`, `noModeration`: chat and nickname word filter, ban list (S6, v0.2.9), files re-read within 10 s |
| `functions/tickets.js` | ~20 | `cleanTickets(db, now)`: deletes race tickets older than 45 min (daily `cleanRaceTickets`, D12) |

## 7.6 Cloud Functions (`functions/index.js`)

| Function | Line | Input | Output | Errors |
|---|---|---|---|---|
| `startRace` | 32 | `{mode: 'race'\|'attack', road: 1..9}` (bad values default to `attack` / 1) | `{ticket}` | `unauthenticated`, `resource-exhausted 'too-fast'` |
| `finishRace` | 48 | `{ticket, coins, bestLap, raceTime, place}` | `{gain, coins, totalCoins}` | `invalid-argument 'no-ticket'`, `failed-precondition 'no-profile' \| 'no-ticket' \| 'expired'`, `resource-exhausted 'too-soon'` |
| `buyCar` | 86 | `{id: 'viper'\|'titan'\|<skin id>}` (skins v0.2.28: `roadster-r01`…`titan-t10`; nitros v0.3.2: `roadster-f01`…`roadster-f10`, `viper-f01`…`viper-f10`, `titan-f01`…`titan-f10`; interiors v0.3.3: `roadster-i01`…`roadster-i10`, `viper-i01`…, `titan-i01`…`titan-i10`) | `{coins}` | `invalid-argument 'unknown'`, `already-exists 'owned'`, `failed-precondition 'no-profile' \| 'funds'` |

Global options: region us-central1, `maxInstances` 10, memory 256 MiB, `cors: true`, `enforceAppCheck` from `ENFORCE_APP_CHECK` (`functions/index.js:22-25`).

## 7.7 Tools

| Tool | Purpose |
|---|---|
| `tools/roadgen/` (`gen.mjs`, `emit.mjs`, `check.mjs`, `metrics.mjs`, `roads.json`) | Offline generation and checking of the 9 roads (writes into `src/roads.js`) [inferred from file names; not run while writing this] |
| `tools/split-wheels.mjs` (v0.3.9) | Splits each shop car pod into frame + `<pod>_wheel` child in `neon_c/meshy/refined/` (from `refined/v038/`); then `tools/compress-car.mjs` |
| `tools/split-wheels.mjs` (v0.3.9) | Splits each shop car pod into frame + `<pod>_wheel` child in `neon_c/meshy/refined/` (from `refined/v038/`); then `tools/compress-car.mjs` |
| `tools/stadium-bots.mjs` (v0.3.8, `npm run bots`) | 5 STADIUM 3V3 bots for local testing: ordinary relay clients with test logins (dev server with `LIVE_MOCK=1` only); they join after a human presses B, orbit and shoot the ball or hunt the other team; `--skill easy\|normal\|hard`, `--bots n`, `--url` |
| `docs/tools/capture-screens.mjs` | Screenshots for the docs (Playwright + Chrome) |
| `tools/fx/voidaura.mjs` | Builds the VOID CROWN textures `public/fx/voidaura_*.webp` from `neon_c/Assets/06_Effects` (recolour + crown cut-out, Playwright Chromium) |
| `inspect.html` + `src/inspect.js` (v0.2.27) | Dev-only car inspection scene on the game renderer (not built: Vite builds `index.html` only): one car, fixed views, diagnostic modes (clay, wire, normals, base, rough, metal, emissive, nocoat, ...), `pipeline=legacy` (frozen v0.2.26 materials), any GLB under `public/`, the 11 colours |
| `tools/capture-cars.mjs`, `tools/inspect-glb.mjs`, `tools/perf-cars.mjs` (v0.2.27) | Matched captures of the inspection scene; GLB report (nodes, pivots, triangles, attributes, materials, textures, GPU estimate, hash); in-game performance before/after (the v0.2.26 car files served in place) |
| `meshy/refine_car.py` (+ `clay_render.py`, `glb_textures.py`, `contact_sheet.py`) (v0.2.27) | Blender 5.2 build of the three shop car files from the raw Meshy sources: production parts/pivots/budget kept, normals from the source, baked normal map, tangents, region maps, emissive light mask, metal pods; saves `meshy/refined/<car>.blend` |
