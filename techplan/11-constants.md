# Constants and important values

[← Index](TECHNICAL_DOCUMENTATION.md)

**How to read this table**

- **Def** is where the value is defined, as `file:line`. **Used** gives the main usage sites.
- **Config?** says whether the value can change without a code edit:
  - "code" means it is a constant in source, so changing it needs a commit plus a rebuild, deploy or restart.
  - "env" means it comes from an environment variable.
  - "device" means a per-player setting.
- **Purpose** is taken from code comments where they exist. When no comment or doc explains a value, the purpose is marked **unknown**.
- All values are [confirmed] by reading the cited line on 2026-11-01.

## A. Economy and anti-cheat (server is the authority)

| Name | Value | Units | Purpose | Def | Used | Config? | Limits / dependencies |
|---|---|---|---|---|---|---|---|
| `RULES.RACE_COIN_CAP` | 200 | coins | Most coins one race can bank | `functions/rules.js:6` | `functions/index.js:66` | code (functions deploy) | Must equal client `RACE_COIN_CAP` (`src/online/index.js:6`) |
| `RACE_COIN_CAP` (client) | 200 | coins | Client mirror of the cap | `src/online/index.js:6` | race/HUD code | code | Display only; the server decides |
| `RULES.RACE_GAP_S` | 60 (120 before v0.2.1) | s | Minimum time between two saved races | `functions/rules.js:8` | `functions/index.js:61` | code | Error `resource-exhausted 'too-soon'`; mirrored by client `RACE_GAP_MS` |
| `RACE_GAP_MS` (client) | 60,000 | ms | When a too-soon race is saved again (`lastRaceAt` + gap + 2 s, clamped 3–65 s) | `src/online/index.js` | `src/account.js` `flushPending` | code | Must equal `RACE_GAP_S` × 1000 |
| Ticket retry | 3,500 | ms | Second ticket request after a failure | `src/account.js` `startRace` | — | code | > `TICKET_MIN_GAP_S` (3 s) |
| `RULES.MIN_RACE_S` | 60 | s | Equals `RACE_GAP_S` (back-to-back real races never wait). Since v0.2.4 coins, times and unlocks use the per-road minimums below | `functions/rules.js` | — | code | ≤ `min(ROAD_MIN_RACE_S)` |
| `RULES.MIN_LAP_S` | 21 (30 before v0.2.15) | s | Fastest lap on any road (= road 1's) | `functions/rules.js` | — | code | `constants.test.mjs` checks it = `min(ROAD_MIN_LAP_S)` |
| `PACE_MAX` | 240 (170 before v0.2.15) | m/s | Faster than any driver can average (nitro 196 + a 1.5× spin surge 2 s in every 6 s ≈ 229) | `functions/rules.js` | `ROAD_MIN_*` | code | `constants.test.mjs` checks it against `CAR` / `SPIN` |
| `ROAD_MIN_RACE_S` / `ROAD_MIN_LAP_S` | 43, 47, 51, 56, 59, 66, 61, 73, 75 / 21, 23, 25, 28, 29, 33, 30, 36, 37 (v0.2.15) | s | Fastest possible 2-lap race / lap per road: gate-to-gate distance ÷ `PACE_MAX` (R8, v0.2.4) | `functions/rules.js` | `finishRace` | code | Recomputed from `src/roads.js` by `constants.test.mjs`: change them with the roads |
| `RULES.DAILY_COIN_CAP` | 2,000 | coins / UTC day | Most coins banked per day (R8) | `functions/rules.js` | `finishRace` (`coinsDay`) | code | Real players make ~10–50 a race |
| `RULES.RELAY_BATCH` / `RELAY_KILLS_MAX` / `KILLER_COUNT_MAX` | 50 / 60 / 1e6 | players / kills / count | What one `relayKills` save may hold | `functions/rules.js` | `relayKills` | code | Relay copies `SAVE_BATCH`, `SAVE_KILLS`, `KC_MAX` (`server/live.js`), checked by `constants.test.mjs` |
| `RULES.TICKET_MAX_AGE_S` | 2,700 (45 min) | s | A race left open longer isn't saved | `functions/rules.js:10` | `functions/index.js:58` | code | |
| `RULES.TICKET_MIN_GAP_S` | 3 | s | Restarting faster than this is refused | `functions/rules.js:11` | `functions/index.js:41` | code | `resource-exhausted 'too-fast'` |
| `ROADS` (functions) | 9 | roads | Valid road ids 1..9 | `functions/rules.js:15` | `functions/index.js:35` | code | Must match `src/roads.js` length |
| `SHOP_PRICES.viper` | 2,500 | coins | Price | `functions/rules.js:21` | `functions/index.js:89` | code | Must match `SHOP_CARS` (`src/online/index.js:21`) |
| `SHOP_PRICES.titan` | 7,500 | coins | Price | `functions/rules.js:22` | as above | code | as above |
| `KILL_CAP` | 60 | kills / save | `?mock` backend only since v0.2.4 (the relay saves real kills) | `src/online/index.js` | `src/online/mock.js` | code | — |
| `KILL_SAVE_GAP_MS` | 60,000 | ms | `?mock` backend only since v0.2.4 | `src/online/index.js` | `src/account.js` `flushKills` | code | — |
| ~~Killer count step~~ | — | — | Removed with the browser's KILLER STATUS writes (v0.2.4) | — | — | — | — |
| `KILLER_KILLS` | 3 | kills | Kills to become a Killer | `src/online/protocol.js` | `server/live.js`; `functions/rules.js` copy (`relayKills`) | code | `constants.test.mjs` checks the copy |
| `NICK_RE` | `^[A-Za-z0-9_-]{3,16}$` | chars | Nickname format | `src/online/protocol.js` (since v0.2.4; re-exported by `src/online/index.js`) | client; relay (imports it); rules `validNick` | code | `constants.test.mjs` checks the rules copy; also the XSS guard for C2 |
| ~~PB rank threshold~~ | — | — | Replaced in v0.2.4 (D4): TIME RANK shows when `finishRace` returns `newTime` | — | — | — | — |
| `TEST_COINS` | 10,000 | coins | `?mock` test wallet | `src/online/mock.js:16` | mock only | code | dev only |

## B. FREE ROAM relay (`server/live.js`)

| Name | Value | Units | Purpose | Def | Config? | Limits / dependencies |
|---|---|---|---|---|---|---|
| `MAX_PLAYERS` | 24 | players | One shared city; limits PC upload | `:47` | code (restart) | Must be ≤ client `ROAM_SLOTS` (24) |
| `TICK_MS` | 66.7 (1000/15) | ms | Snapshot rate 15/s | `:48` | code | Client `SEND_MS` and `DELAY` assume it |
| `MAX_MSG` | 600 | bytes | Max frame once joined | `:49` | code | |
| `MAX_HI` | 8,192 | bytes | Max sign-in frame (ID token ~1 KB + App Check token ~1 KB; 4,096 before v0.2.2) | `server/live.js` | code | Also `maxPayload` |
| `MAX_RATE` | 45 | msgs / s | Per client (state 15 + shots) | `:51` | code | Excess dropped |
| `MAX_SHOTS` | 14 | / s | Fire events and hits per client | `:52` | code | |
| `IDLE_MS` | 30,000 | ms | Silent car removed | `:53` | code | Client idle keep-alive 500 ms |
| `LIMIT` | 20,000 | m | Coordinate sanity bound | `:54` | code | `ROAM_RADIUS` 9,000 sits well inside |
| `CHAT_MAX` | 120 | chars | Chat length | `src/online/protocol.js` (v0.2.4) | relay and `src/chat.js` import it | code | One definition |
| `CHAT_BURST` / `CHAT_REFILL_MS` | 5 / 3,000 | msgs / ms | Chat token bucket | `:57` | code | Client pace `GAP_MS` 1,200 (`src/chat.js:7`) |
| `KILL_AFTER_HIT_MS` | 30,000 | ms | A wreck counts only after the killer's checked hit on the victim this recently (EMPs too until v0.2.20) (S3, v0.2.10; replaced `HIT_AFTER_FIRE_MS` 5,000) | `server/live.js` | code | Generous: a slow victim's game may report late |
| `DIE_MS` | 2,000 | ms | One wreck per victim per 2 s | `:62` | code | Below `ROAM_WRECK` 3 s |
| `KC_MAX` | 1,000,000 | count | Saved count above this is reset | `:63` | code | |
| Auth timer | 15,000 | ms | Close if no sign-in | `:138` | code | Client welcome timeout 12 s |
| Profile REST timeout | 8,000 | ms | Firestore read | `:94` | code | |
| Heartbeat | 15,000 | ms | WebSocket ping, idle check | `:264` | code | Keeps Cloudflare from closing quiet sockets |
| State clamps | pitch ±3.15 (v0.2.22; ±1.6 before), roll ±1.6, speed −50..400, boost 0..1, hp 0..100 | rad, m/s, ratio | Sanitise | `:156-161` | code | |
| `LIVE_PROTOCOL` | 15 | version | Client/relay compatibility (7: relay-saved kills, shop cars; 8: moderation, `err banned`, v0.2.9; 9: bolts 21 damage, 750 m/s × 1.5 s, v0.2.14; 10: cars 135 / 196 m/s, v0.2.15; 11: shop cars differ, hp sent in %, v0.2.17; 12: bolts 300 m/s × 3.5 s, v0.2.19; 13: health 200 / 150 / 250, v0.2.20; 14: no EMP, v0.2.21; 15: backflip, v0.2.22) | `src/online/protocol.js` | code | Bump on any message change; old clients reload |
| `VOID_KILLS` | 10 | Killer kills | CRIMSON FLAME → VOID CROWN | `src/online/protocol.js:9` | `src/killer.js` `auraStyleFor`, `src/main.js` | code | > `KILLER_KILLS`; rules allow +10 per save, so the count can reach it in one save |

## C. FREE ROAM client (`src/online/live.js`, `src/roam.js`)

| Name | Value | Units | Purpose | Def | Config? | Limits / dependencies |
|---|---|---|---|---|---|---|
| `SEND_MS` | 66.7 | ms | Send rate 15/s | `src/online/live.js:7` | code | Must keep under `MAX_RATE` with shots |
| `IDLE_SEND_MS` | 500 | ms | Keep-alive while paused | `src/online/live.js:8` | code | ≪ `IDLE_MS` |
| `PING_MS` | 2,000 | ms | RTT measurement | `src/online/live.js:9` | code | |
| `DELAY` | 0.15 | s | Interpolation delay | `src/online/live.js:10` | code | Covers jitter and 66 ms ticks |
| Welcome timeout | 12,000 | ms | Give up joining | `src/online/live.js:71` | code | |
| Reconnect backoff | min(15,000, 500·2ⁿ) | ms | Retry | `src/online/live.js:93` | code | |
| Reload-on-old guard | 60,000 | ms | At most one reload per minute | `src/online/live.js:171` | code | |
| `ROAM_HUB` | (0, 175, 70) | m | Spawn hub | `src/roam.js` | code | |
| `ROAM_SLOTS` | 24 | slots | Spawn slots | `src/roam.js` | code | ≥ `MAX_PLAYERS` |
| `ROAM_KNOCK` | bolt [0.9, 0.25, 6, 21], missile [0.45, 1.0, 26, 48] (emp removed in v0.2.21) | [speed kept (factor), stun s, push m/s, damage hp] | FREE ROAM hit reaction | `src/roam.js` | code | Meaning from the code comment above it; damage = `WEAPON.*_DMG` |
| `ROAM_WRECK` | 3 | s | Down time after wreck | `src/roam.js` | code | > relay `DIE_MS` |
| `ROAM_SAFE` | 3 | s | Spawn protection | `src/roam.js` | code | |
| `RESPAWN_SAFE` | 2 | s | RACE: every car's protection after a wreck (no damage, shots pass through, not targeted, can't attack, blinking; v0.2.20) | `src/race.js` | code | `race.test.mjs` |
| `ROAM_REGEN` | [5, 8] | [s, hp/s] | Regen delay and rate | `src/roam.js` | code | |
| `ROAM_CEIL` / `ROAM_RADIUS` | 1,600 / 9,000 | m | Flight bounds | `src/roam.js` | code | < relay `LIMIT` |
| Car contact radius | 4.6 | m | Car-to-car collision | `src/roam.js` `updateRoam`, `src/race.js` `carContacts` | code | ≈ 2 × `CAR.RADIUS` (2.1) + margin [inferred] |
| Online count refresh | 10,000 | ms | Menu count | `src/roam.js` `refreshOnlineCount` | code | |

## D. Race, cars, weapons

| Name | Value | Units | Purpose | Def | Config? | Notes |
|---|---|---|---|---|---|---|
| `CAR.MAX` / `BOOST_MAX` / `ACCEL` / `BOOST_ACCEL` | `TOP` 135 / 196 / 51 / 107 (95 / 138 / 36 / 75 × `K = TOP / 95`; v0.2.15) | m/s, m/s² | Top speed / with boost / accelerations | `src/car.js` | code | Server `PACE_MAX` and `ROAD_MIN_*` depend on these (`constants.test.mjs` evaluates `CAR`) |
| `CAR.ACCEL` / `BOOST_ACCEL` | 36 / 75 | m/s² | Acceleration | `src/car.js:10` | code | |
| `CAR.RADIUS` | 2.1 | m | Collision radius | `src/car.js:10` | code | |
| `FREE_PITCH` | LEAD 0.62, LEVEL 0.9, LEVEL_AFTER 0.5 | rad, rad/s, s | Mouse loops: aim lead, auto-level (v0.2.22) | `src/aim.js` | code | Tested in `tests/controls.test.mjs` |
| `PITCH` | RATE 2.4, FOLLOW 4 | rad/s, 1/s | The nose following the mouse's pitch (v0.2.22) | `src/car.js` | code | |
| `FLIP` | TIME 1.2, R 15 | s, m | Backflip: the loop (v0.2.22) | `src/car.js` | code | Tested in `tests/controls.test.mjs` |
| `AIRBRAKE` | DECEL 220, MIN 25, TURN 1.6, GRIP 6 | m/s², m/s, ×, 1/s | Air brake (Space, v0.2.22) | `src/car.js` | code | Tested in `tests/controls.test.mjs` |
| `SPIN` | ROLL 0.55, SURGE 2, MUL 1.5, JINK 22, COOLDOWN 6 | s, s, ×, m/s, s | Spin dodge | `src/car.js:12` | code | Units [inferred] from comments |
| Boost drain / regen | 0.32 / 0.06 | fraction / s | Boost tank | `src/car.js:666` | code | |
| `LIVERIES` | 10 rivals, skill 1.0–0.92 | factor | Rival identity, boost eagerness (top speed no longer, since v0.2.3) | `src/ai.js:3` | code | |
| `CAR_MODELS[id].flames` | roadster side [0.85, 0.25] middle [0.85, 0.4] · viper [0.85, 0.34] / [0.85, 0.6] · titan [0.85, 0.25] / [0.85, 0.34] | [length, width] scale | Thruster flame size per car (v0.2.18), spots in `exhaust` / `turbineFlame` (marked by the owner) | `src/car.js` | code | Same length, middle thicker (`controls.test.mjs`) |
| `BOLT_LOOKS` | roadster len 8, flash [2.6, 1.07, 0.34], impact 5.5 · viper 8, [2.6, 0.91, 0.41], 5.5 · titan 7, [3, 1.26, 0.32], 6; glow (flash / impact added) roadster, viper | m, [height m, width / height, gun across 0..1], m | Each car's bolt (v0.2.19): 3D model scaled to `len` (×1.6 at most with distance), muzzle flash 0.09 s, impact | `src/fx.js` | code | Rivals: roadster. Art and GLBs checked by `controls.test.mjs` |
| Engine smoke | every 0.012 s + health / 20; 93% of the car's speed, drag e^(−1.6 t); 1.3 + 2.2 k m | s, m | Damage smoke from the nozzles under 60% health (v0.2.18) | `src/main.js` `stepRacer`, `src/fx.js` `exhaustSmoke` | code | |
| `CAR_STATS` | roadster: all 1, hp 200, spinCD 6, flipCD 8, missiles 3 · viper: top 1.08, accel 1.276, turn 1.12, hp 150, knock 1.3, mass 0.8, spinCD 4, flipCD 6 · titan: top 0.94, accel 0.797, turn 0.9, hp 250, knock 0.4, stun 0.6, mass 2, missiles 4 | factors, hp, s | Per shop car (v0.2.17; hp 100 / 75 / 140 until v0.2.19): speed, acceleration, turning, health, knock-back, stun, bump weight, spin and backflip cooldowns, seekers (EMP radius until v0.2.20) | `src/car.js` | code | Rivals use roadster; the relay's `EMP_CAR_MAX` = the biggest `emp` (`constants.test.mjs`) |
| `RIVAL_AIM` | SPREAD 0.03 rad, CONE 35° (0.3 rad before v0.2.16), SLOWER 3, SEEKER_RANGE 600 m, LOCK_TIME 1.5 s, SEEKERS 2, SEEKER_RECHARGE 9 s, SEEKER_GAP 6 s (v0.2.20; the player's 0.5 s / 3 / 4.5 s, gap 1.5 s before), DODGE 0.5 | | Rivals use the player's arsenal: bolt aim error at road 1 (× road `fire`), target cone, × the player's bolt interval (0.63 s); seekers on a full lock within range, at most one per gap (v0.2.16); spin-dodge chance per seeker homing on them; (EMP_CARS removed in v0.2.21) | `src/race.js` | code | Replaced `RIVAL_POWER`, `DAMAGE`, `AI_FIRE_INTERVAL` |
| `RIVAL_SPEED` | ROAD1 120, STEP 1.02 | m/s, factor | Rival top speed `rivalTop(id) = 120 × 1.02^(id − 1)`: 120 … 141 m/s (v0.2.15) | `src/roads.js` | code | Replaced road `ai` factors |
| `CORNER` | TURN 1.15 rad/s, GRIP 0.8, BRAKE 45 m/s², SPAN 3 samples | | Rival corner speeds and braking (v0.2.15) | `src/ai.js` | code | |
| `GATE_R` / `GATE_R_RIVAL` | 15 / 45 | m | Gate ring radius that counts a pass: player / rivals (v0.2.15) | `src/race.js` | code | |
| `WEAPON` | BOLT_DMG 21, BOLT_RATE 0.21 s, BOLT_SPEED 300 m/s, BOLT_LIFE 3.5 s (750 / 1.5 in v0.2.14-0.2.18), BOLT_LEN 45 m, BOLT_WID 7 m (unused by bolts since v0.2.19: `BOLT_LOOKS`), HEAT_PER_BOLT 0.08 (~22 bolts to overheat), COOL_RATE 0.45 /s, LOCK_RANGE 2000 m, LOCK_KEEP 2100 m, MISSILE_DMG 48, MISSILES 3, MISSILE_RECHARGE 4.5 s, LOCK_TIME 0.5 s | | Player arsenal | `src/ai.js:18-22` | code | Comment says seekers on "Space / F / MMB"; F is spin dodge (stale comment; since v0.2.22: "1 / MMB; F is the spin dodge, 2 the backflip, Space the air brake") |
| `COIN` | PER_GATE_GAP 6, VALUE 1, RESPAWN 6 s, RADIUS 6.5 m | | Track coins | `src/ai.js:23` | code | |
| Race wreck | 2.4 | s | Down time in races | `src/race.js` `wreck` | code | |
| Loot on wreck | 25 % (rounded up) | of coins | Coins the attacker takes on a takedown | `src/race.js` `wreck` | code | Tested in `tests/race.test.mjs` |
| Podium coin bonus | [0, 15, 8, 4] | coins | Bonus for 1st/2nd/3rd in the coin race (`multi` = a RACE with AI rivals) | `src/race.js` `showResults` | code | Counts toward the 200 cap |
| Rubber band (`RUBBER_BAND`) | clamp(gap·0.05, 0, 0.07) (−0.05 before v0.2.15) | speed factor | AI catch-up; a rival ahead is never slowed | `src/race.js` `updateRanking` | code | |
| Lock-on | acquire < 0.3 rad within 2000 m (`LOCK_RANGE`), keep < 0.5 rad within 2100 m (`LOCK_KEEP`), min 8 m (400 / 460 before v0.2.14) | rad, m | Seeker lock | `src/aim.js` `updateLock` | code | |
| `KNOCK` | bolt [0.985, 0.12], missile [0.5, 1.0], shot [0.6, 0.7] (emp removed in v0.2.21) | [speed kept (factor), stun s] | Race hit reaction | `src/race.js` | code | Tested in `tests/race.test.mjs` (missile halves the speed) |
| `CAMS` | {d 9, h 2.7}, {d 15, h 4.8}, cockpit | m | Camera presets | `src/camera.js` | device (`neon.view`) | |
| `MOUSE_RAD_PER_PX` | 0.0021 | rad / px | Mouse look | `src/aim.js` | × device `neon.sens` | |
| `PLAYER_SLOT` | 7 | grid slot | Player's start slot | `src/main.js` | code | `GRID` has 12 (`src/track.js:8`) |
| `MAPS` laps / floor / roamFloor | 2 / 34 / 40 | laps, m, m | Race length, fall-out height | `src/maps.js:13-15` | code | `MIN_RACE_S` assumes 2 laps |
| `ROADS` | 9 entries, `fire` (rival aim), rings 9–19 | | Road set | `src/roads.js:11` | code (tools/roadgen) | Must match functions `ROADS` |
| `RING` | R 15.5, tube 2.3, H 22 | m | Ring geometry | `src/props.js:5` | code | |

## E. Rendering and world

| Name | Value | Units | Purpose | Def | Config? |
|---|---|---|---|---|---|
| `FOG_DENSITY` | 0.00062 | 1/m (exp²) | Fog | `src/render.js` | code |
| `DECK_Y` | 111 | m | Start deck height | `src/course.js` | code (one definition since v0.2.4) |
| `GATE` | pos (0,126,0), halfW 23, y 109..142 | m | Start/finish gate | `src/race.js` | code |
| `GFX.high` | ratio min(dpr, 1.5), 4 samples, blur | | High quality | `src/render.js` | device (`neon.gfx`) |
| `GFX.low` | ratio 0.75, 0 samples, no blur | | Low quality | `src/render.js` | device |
| Auto-gfx | skip 40 frames, 3 s sample, < 40 fps → low | frames, s, fps | Auto quality (menu, no saved choice, not touch) | `src/render.js` `measureGfx` | code |
| `sculptReady` timeout | 20,000 | ms | Stop waiting for the car GLB | `src/main.js` `sculptReady` | code |
| `physics CELL` | 120 | m | Collider grid | `src/physics.js:3` | code |
| `SAMPLE_STEP` | 4 | m | Spline sampling | `src/track.js:6` | code |
| `Corridor CELL` | 160 | m | Road corridor grid | `src/track.js:116` | code |
| City | base −260, extent 2,700, seed 1337 | m | Procedural city | `src/city.js:7,38-41` | code (map opts) |
| Sky props seed | 4242 | | Flying props layout | `src/skyprops.js:81` | code |
| Clouds | 900 puffs default | count | Mist | `src/env.js:161` | map opts |
| `SLIDE_W/H`, `SLIDE_SECONDS`, max screens | 768×384, 7, 48 | px, s, count | Billboards | `src/ads.js:12-13,344` | code |
| `SPARKS` | 140 | particles | Nitro sparks | `src/nitro.js:46` | code |
| `TIERS` (killer) | embers / shards (shards: VOID CROWN only): high 28 / 7 + trail, menu 14 / 7, remote 10 / 5 + trail, low 0 / 3 | particles, meshes | Aura cost | `src/killer.js` `TIERS` | code |
| `AURA_STYLES` | `crimson` (CRIMSON FLAME), `void` (VOID CROWN) | style | The two Killer auras | `src/killer.js` | code | `auraStyleFor(kc)`: `void` from `VOID_KILLS`, else `crimson` |
| `KILLER_AURA.*.crown` / `.shards` | crown [1.35 × body width, lift −0.25 m roadster / 0.1 m others, +0.05 × length]; shards [0.55 m beyond sides, 0.9 × half length] | ×, m | VOID CROWN placement per car | `src/killer.js` `KILLER_AURA` | code |
| `ART` | 1672×941 | px | Cockpit art size | `src/cockpit.js:7` | code |
| `DROP` | 0.2 | fraction | Cockpit layout shift | `src/cockpit.js:10` | code |

## F. Input, audio, UI, analytics

| Name | Value | Units | Purpose | Def | Config? |
|---|---|---|---|---|---|
| Gamepad deadzone | 0.15 | axis | | `src/input.js:104` | code |
| `STEER_RANGE` / `CLIMB_RANGE` / `DEAD` | 24° / 18° / 2.5° | deg | Touch tilt | `src/touch.js:10-12` | code |
| `MASTER` | 0.28 | gain | Volume at 100 % | `src/audio.js:4` | × device `neon.vol` |
| `MAX_VOICES` | 18 | voices | Audio polyphony | `src/audio.js:5` | code |
| `FADE_S` / `KEEP` / `GAP_MS` | 12 / 30 / 1,200 | s / lines / ms | Chat UI | `src/chat.js:4-7` | code |
| `RANKING_SIZE` | 10 | rows | Leaderboard size | `src/account.js:14` | code |
| Online call timeout | 15,000 | ms | Every backend call | `src/online/firebase.js:41`, `src/account.js:72,224,248,361,401` | code |
| `RETRYABLE` | unavailable, deadline-exceeded, internal, aborted, resource-exhausted | error codes | Show TRY AGAIN | `src/account.js:32` | code |
| Analytics queue | 50 | events | Events kept before consent | `src/online/analytics.js:38` | code |
| `SHOP_NEWS` | 'titan' | car id | NEW badge | `src/shop.js:11` | code |
| `maxInstances` / memory | 10 / 256 MiB | instances / RAM | Functions scaling | `functions/index.js:22` | code (deploy) |
| Emulator ports | firestore 8085, functions 5001, auth 9099 | port | Tests | `firebase.json` | config |
| Production port | 8080 | port | `server/serve.mjs` | `package.json` `serve` | config (+ tunnel) |
| Relay port | 8091 (production port + 11) | port | relay process, `127.0.0.1` only | `server/serve.mjs` (`LIVE_RELAY_PORT`) | code / env | — |

## Relay connection limits and page security (v0.2.7)
| Name | Value | Unit | Purpose | Source | Where it applies | Notes |
|---|---|---|---|---|---|---|
| `PER_IP` | 6 | open connections | Per player address (Cloudflare `CF-Connecting-IP`) | `server/live.js` | relay upgrade | Over the limit: HTTP 429 before the WebSocket opens |
| `PER_IP_MINUTE` | 20 | connections / minute | New connections per address | `server/live.js` | relay upgrade | The game reconnects with a 1-15 s back-off |
| `MAX_PENDING` | 64 | connections | Still signing in, all addresses together | `server/live.js` | relay upgrade | Each closes after 15 s without a sign-in |
| CSP | see `server/serve.mjs` `CSP` | | Content-Security-Policy on every page | `server/serve.mjs` | pages | Report-only until `CSP_ENFORCE=1` (`.env.local`); reports logged as `[csp]` |

## FREE ROAM hit checks (S3, v0.2.10, `server/hits.js`)
| Name | Value | Unit | Purpose | Notes |
|---|---|---|---|---|
| `BOLT_SPEED` / `BOLT_LIFE` | 300 / 3.5 (v0.2.19; 750 / 1.5 from v0.2.14, 430 / 1.3 before) | m/s, s | Bolt reach (+ the shooter's speed) | = `WEAPON.BOLT_SPEED`, `src/aim.js`; `constants.test.mjs` checks |
| `MISSILE_TOP` / `MISSILE_LIFE` | 330 / 3.6 | m/s, s | Seeker reach (+ the shooter's speed) | = `maxSpeed: speed + 330`, `life: 3.6` |
| `SEEN_MS` | 1,500 | ms | How far back a victim's / shooter's positions count | Covers 150 ms playback + lag |
| `TRACK_MS` | 3,000 | ms | Positions kept per player | |
| `LAG_S` / `LATE_S` | 0.6 / 1 | s | Extra flight time; a hit may arrive this long after the shot's life | |
| `REACH_SLACK` | 40 | m | Car size, muzzle, 15/s positions | |
| `NEAR` / `AIM_COS` | 80 / 0.6 | m, cos | Any direction this close; otherwise within ~53° | |
| Shot origin | 20 + 0.3 × speed | m | `f` must come from near the shooter | |
| `DROP_LOG_MS` | 60,000 | ms | `[s3]` log at most once a minute per player | `server/live.js` |

## Chat moderation (S6, v0.2.9)
| Name | Value | Purpose | Source |
|---|---|---|---|
| Reload check | 10,000 ms | `blocked-words.txt` / `bans.txt` re-read when changed | `server/moderation.js` `every` |
| `REPORT_GAP_MS` | 30,000 ms | One `/report` per player | `server/live.js` |
| `RECENT_LINES` | 8 | Chat lines kept per player (memory) for `/report` | `server/live.js` |
| Mutes | 200 names | `neon.muted` (localStorage) | `src/online/mutes.js` |

## Mouse aim, camera and slide (v0.2.12-v0.2.13)
| Name | Value | Unit | Purpose | Source |
|---|---|---|---|---|
| Aim limits | ±1.1 yaw from the nose; pitch ±0.62 (keys, no mouse) or with the mouse any angle, ≤ 0.62 ahead of the nose (`FREE_PITCH.LEAD`, v0.2.22) | rad | How far the aim may lead the car | `src/aim.js` `updateAim` |
| Steering gain | `steer = −Δyaw × 2.8`, `climb = aimPitch / 0.62` | | The car flies to the aim (× 1.25 on climb before v0.2.12) | `src/aim.js` |
| `STRAFE` | SPEED 25, RESPONSE 6, LEAN 42° | m/s, 1/s, rad | Sideways slide (A / D with the mouse, v0.2.13); body roll at a full slide (v0.2.14, was 0.3 rad) | `src/car.js` |
| `CAR_MODELS[id].muzzle` | roadster [0, −0.61, −2.77], viper [±0.17, −1.15, −2.15], titan [0, −1.03, −2.45] | m, car space | Where shots leave each model: the owner's marked shooting spots (v0.2.19, `Assets/01_Vehicle/*_shooting_spot*.png`, measured on the GLBs; x 0 = one spot, VIPER fires from its two in turn). Front bumper in v0.2.14-0.2.18 | `src/car.js` |
| Aim camera follow | yaw 14 /s, pitch 10 /s toward aim − 0.15 × clamp(aim, ±0.62) (0.85 × aim before v0.2.22), up vector turned with the pitch | 1/s | Keeps the aim ring near the centre (car camera: 7 and 5, 0.6 × car pitch) | `src/camera.js` `chaseCamera` |

