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
| `RULES.MIN_LAP_S` | 30 | s | Fastest lap on any road (= road 1's) | `functions/rules.js` | — | code | `constants.test.mjs` checks it = `min(ROAD_MIN_LAP_S)` |
| `PACE_MAX` | 170 | m/s | Faster than any driver can average (nitro 138 + a 1.5× spin surge 2 s in every 6 s ≈ 161) | `functions/rules.js` | `ROAD_MIN_*` | code | `constants.test.mjs` checks it against `CAR` / `SPIN` |
| `ROAD_MIN_RACE_S` / `ROAD_MIN_LAP_S` | 61, 66, 72, 79, 84, 94, 86, 103, 106 / 30, 33, 36, 39, 42, 47, 43, 51, 53 | s | Fastest possible 2-lap race / lap per road: gate-to-gate distance ÷ `PACE_MAX` (R8, v0.2.4) | `functions/rules.js` | `finishRace` | code | Recomputed from `src/roads.js` by `constants.test.mjs`: change them with the roads |
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
| `MAX_HI` | 4,096 | bytes | Max sign-in frame (ID token ~1 KB) | `:50` | code | Also `maxPayload` |
| `MAX_RATE` | 45 | msgs / s | Per client (state 15 + shots) | `:51` | code | Excess dropped |
| `MAX_SHOTS` | 14 | / s | Fire events and hits per client | `:52` | code | |
| `IDLE_MS` | 30,000 | ms | Silent car removed | `:53` | code | Client idle keep-alive 500 ms |
| `LIMIT` | 20,000 | m | Coordinate sanity bound | `:54` | code | `ROAM_RADIUS` 9,000 sits well inside |
| `CHAT_MAX` | 120 | chars | Chat length | `src/online/protocol.js` (v0.2.4) | relay and `src/chat.js` import it | code | One definition |
| `CHAT_BURST` / `CHAT_REFILL_MS` | 5 / 3,000 | msgs / ms | Chat token bucket | `:57` | code | Client pace `GAP_MS` 1,200 (`src/chat.js:7`) |
| `HIT_AFTER_FIRE_MS` | 5,000 | ms | A hit must follow the shooter's own shot | `:61` | code | Missile flight ≤ 3.6 s (comment) |
| `DIE_MS` | 2,000 | ms | One wreck per victim per 2 s | `:62` | code | Below `ROAM_WRECK` 3 s |
| `KC_MAX` | 1,000,000 | count | Saved count above this is reset | `:63` | code | |
| Auth timer | 15,000 | ms | Close if no sign-in | `:138` | code | Client welcome timeout 12 s |
| Profile REST timeout | 8,000 | ms | Firestore read | `:94` | code | |
| Heartbeat | 15,000 | ms | WebSocket ping, idle check | `:264` | code | Keeps Cloudflare from closing quiet sockets |
| State clamps | pitch/roll ±1.6, speed −50..400, boost 0..1, hp 0..100 | rad, m/s, ratio | Sanitise | `:156-161` | code | |
| `LIVE_PROTOCOL` | 7 | version | Client/relay compatibility (7: relay-saved kills, shop cars) | `src/online/protocol.js` | code | Bump on any message change; old clients reload |
| `VOID_KILLS` | 10 | Killer kills | CRIMSON FLAME → VOID CROWN | `src/online/protocol.js:9` | `src/killer.js` `auraStyleFor`, `src/main.js` | code | > `KILLER_KILLS`; rules allow +10 per save, so the count can reach it in one save |

## C. FREE ROAM client (`src/online/live.js`, `src/main.js`)

| Name | Value | Units | Purpose | Def | Config? | Limits / dependencies |
|---|---|---|---|---|---|---|
| `SEND_MS` | 66.7 | ms | Send rate 15/s | `src/online/live.js:7` | code | Must keep under `MAX_RATE` with shots |
| `IDLE_SEND_MS` | 500 | ms | Keep-alive while paused | `src/online/live.js:8` | code | ≪ `IDLE_MS` |
| `PING_MS` | 2,000 | ms | RTT measurement | `src/online/live.js:9` | code | |
| `DELAY` | 0.15 | s | Interpolation delay | `src/online/live.js:10` | code | Covers jitter and 66 ms ticks |
| Welcome timeout | 12,000 | ms | Give up joining | `src/online/live.js:71` | code | |
| Reconnect backoff | min(15,000, 500·2ⁿ) | ms | Retry | `src/online/live.js:93` | code | |
| Reload-on-old guard | 60,000 | ms | At most one reload per minute | `src/online/live.js:171` | code | |
| `ROAM_HUB` | (0, 175, 70) | m | Spawn hub | `src/main.js:745` | code | |
| `ROAM_SLOTS` | 24 | slots | Spawn slots | `src/main.js:746` | code | ≥ `MAX_PLAYERS` |
| `ROAM_KNOCK` | bolt [0.9, 0.25, 6, 6], missile [0.45, 1.0, 26, 48], emp [0.6, 1.2, 34, 25] | mixed: [speed keep factor, ?, impulse m/s?, damage hp] | Hit reaction | `src/main.js:932-934` | code | **Meaning of the first three per-element values: unknown** (no comment); last = `WEAPON.*_DMG` |
| `ROAM_WRECK` | 3 | s | Down time after wreck | `src/main.js:935` | code | > relay `DIE_MS` |
| `ROAM_SAFE` | 3 | s | Spawn protection | `src/main.js:936` | code | |
| `ROAM_REGEN` | [5, 8] | [s, hp/s] | Regen delay and rate | `src/main.js:937` | code | |
| `ROAM_CEIL` / `ROAM_RADIUS` | 1,600 / 9,000 | m | Flight bounds | `src/main.js:1007` | code | < relay `LIMIT` |
| Car contact radius | 4.6 | m | Car-to-car collision | `src/main.js:1050,1265` | code | ≈ 2 × `CAR.RADIUS` (2.1) + margin [inferred] |
| Online count refresh | 10,000 | ms | Menu count | `src/main.js:1087` | code | |

## D. Race, cars, weapons

| Name | Value | Units | Purpose | Def | Config? | Notes |
|---|---|---|---|---|---|---|
| `CAR.MAX` / `BOOST_MAX` | 95 / 138 | m/s | Top speed / with boost | `src/car.js:10` | code | Server `MIN_*_S` depend on these |
| `CAR.ACCEL` / `BOOST_ACCEL` | 36 / 75 | m/s² | Acceleration | `src/car.js:10` | code | |
| `CAR.RADIUS` | 2.1 | m | Collision radius | `src/car.js:10` | code | |
| `SPIN` | ROLL 0.55, SURGE 2, MUL 1.5, JINK 22, COOLDOWN 6 | s, s, ×, m/s, s | Spin dodge | `src/car.js:12` | code | Units [inferred] from comments |
| Boost drain / regen | 0.32 / 0.06 | fraction / s | Boost tank | `src/car.js:666` | code | |
| `LIVERIES` | 10 rivals, skill 1.0–0.92 | factor | Rival identity, boost eagerness (top speed no longer, since v0.2.3) | `src/ai.js:3` | code | |
| `RIVAL_POWER` | 1.5 | factor | Rival damage and fire rate multiplier (v0.2.3) | `src/ai.js:18` | code | |
| `DAMAGE` | RIVAL_SHOT 51, RIVAL_SHOT_ON_PLAYER 30 (34 / 20 × `RIVAL_POWER`) | hp | AI damage | `src/ai.js:19` | code | |
| `WEAPON` | BOLT_DMG 6, BOLT_RATE 0.09 s, BOLT_SPEED 430 m/s, HEAT_PER_BOLT 0.06, COOL_RATE 0.45 /s, MISSILE_DMG 48, MISSILES 3, MISSILE_RECHARGE 4.5 s, LOCK_TIME 0.5 s, EMP_DMG 25, EMP_RADIUS 95 m, EMP_COOLDOWN 11 s | | Player arsenal | `src/ai.js:18-22` | code | Comment says seekers on "Space / F / MMB"; F is spin dodge (stale comment) |
| `COIN` | PER_GATE_GAP 6, VALUE 1, RESPAWN 6 s, RADIUS 6.5 m | | Track coins | `src/ai.js:23` | code | |
| `AI_FIRE_INTERVAL` | [4, 8] ([6, 12] ÷ `RIVAL_POWER`) | s | Rival shot interval | `src/ai.js:27` | code | × road `fire` factor |
| Race wreck | 2.4 | s | Down time in races | `src/main.js:1140` | code | |
| Loot on wreck | 25 % | of coins | Coins dropped | `src/main.js:1153` | code | |
| Podium coin bonus | [0, 15, 8, 4] | coins | Bonus for 1st/2nd/3rd in the coin race (`multi` = a RACE with AI rivals) | `src/main.js:1333` | code | Counts toward the 200 cap |
| Rubber band | clamp(gap·0.05, −0.05, 0.07) | speed factor | AI catch-up | `src/main.js:1302` | code | |
| Lock-on | acquire < 0.3 rad within 400 m, keep < 0.5 rad within 460 m, min 8 m | rad, m | Seeker lock | `src/main.js:1386-1395` | code | |
| `KNOCK` | bolt [0.985, 0.12], missile [0.5, 1.0], emp [0.65, 1.6], shot [0.6, 0.7] | **unknown** units | Race hit reaction | `src/main.js:1116` | code | No comment; likely [speed factor, kick] [inferred] |
| `CAMS` | {d 9, h 2.7}, {d 15, h 4.8}, cockpit | m | Camera presets | `src/main.js:1523` | device (`neon.view`) | |
| `MOUSE_RAD_PER_PX` | 0.0021 | rad / px | Mouse look | `src/main.js:465` | × device `neon.sens` | |
| `PLAYER_SLOT` | 7 | grid slot | Player's start slot | `src/main.js:410` | code | `GRID` has 12 (`src/track.js:8`) |
| `MAPS` laps / floor / roamFloor | 2 / 34 / 40 | laps, m, m | Race length, fall-out height | `src/maps.js:13-15` | code | `MIN_RACE_S` assumes 2 laps |
| `ROADS` | 9 entries, `ai` ≈ 1–1.0x, `fire`, rings 9–19 | | Road set | `src/roads.js:11` | code (tools/roadgen) | Must match functions `ROADS` |
| `RING` | R 15.5, tube 2.3, H 22 | m | Ring geometry | `src/props.js:5` | code | |

## E. Rendering and world

| Name | Value | Units | Purpose | Def | Config? |
|---|---|---|---|---|---|
| `FOG_DENSITY` | 0.00062 | 1/m (exp²) | Fog | `src/main.js:35` | code |
| `DECK_Y` | 111 | m | Start deck height | `src/main.js:36`, duplicate `src/course.js:10` | code (two copies) |
| `GATE` | pos (0,126,0), halfW 23, y 109..142 | m | Start/finish gate | `src/main.js:37` | code |
| `GFX.high` | ratio min(dpr, 1.5), 4 samples, blur | | High quality | `src/main.js:129` | device (`neon.gfx`) |
| `GFX.low` | ratio 0.75, 0 samples, no blur | | Low quality | `src/main.js:130` | device |
| Auto-gfx | skip 40 frames, 3 s sample, < 40 fps → low | frames, s, fps | Auto quality (menu, no saved choice, not touch) | `src/main.js:150-156` | code |
| `sculptReady` timeout | 20,000 | ms | Stop waiting for the car GLB | `src/main.js:409` | code |
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
