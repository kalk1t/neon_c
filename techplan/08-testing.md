# 11. Testing

[← Index](TECHNICAL_DOCUMENTATION.md)

## 11.1 Automated suites in the repository

| Suite | Command | Runner | Needs | Covers |
|---|---|---|---|---|
| Relay, shared values, production server | `npm run test:live` | `node:test` | nothing (relay in-process, `allowMock`; `serve.mjs` on free ports with a made-up `dist/`) | `live.test.mjs`: KILLER STATUS counting, `killer` broadcast once, `/pkclear`, unknown commands, protocol mismatch → `err old`, persistence via `hi.x/kc`, shop cars (`hi.car`), saving kills (batched, retried, rejoin before the save). `constants.test.mjs` (D2): values that must agree across game, relay, functions and rules, and each road's minimum times recomputed from `src/roads.js`. `serve.test.mjs` (R5/R6): caching, `/privacy`, path traversal, unknown hosts, `/live` through to the relay process, relay crash → restart, relay stops with the server. `launcher.test.mjs` (R12): log rotation without gaps. Since v0.2.7-v0.2.12 also `race`, `roam`, `controls` (game modules in Node), `hits` and `moderation` (see the result tables below) |
| Firestore rules | `npm run test:rules` | `node:test` + `@firebase/rules-unit-testing` | Firestore emulator (port 8085), Java 21+ | Profiles and nicknames, renaming, server-only fields (M8), **no browser writes to kills or KILLER STATUS** (v0.2.4), private player info, the game's own transactions |
| Cloud Functions | `npm run test:functions` | `node:test` | Emulators: auth 9099, firestore 8085, functions 5001; `tests/functions-secret.mjs` writes the emulator's `RELAY_SECRET` | `races` (15 tests: tickets, timing, caps, per-road minimum times, best lap vs race, daily cap, unlocks), `shop` (6), `relayKills` (4: secret, adding and replacing, `/pkclear`, bad entries and missing profiles), old tickets (1) |
| Browser | `npm run test:browser` | Playwright (`playwright-core`) driven by `tests/browser/run.mjs` | Chromium (`npx playwright-core install chromium`); starts its own Vite dev server with `LIVE_MOCK=1` | `killer.test.mjs` (KILLER STATUS, two players), `void.test.mjs` (VOID CROWN), `save.test.mjs` (race saving, fake backend), `rivals.test.mjs` (v0.2.3: rival speed = player × `road.ai`, `RIVAL_POWER` 1.5 damage and fire rate, roads 1 and 9); a page error fails the test |

The emulator suites use the demo project `demo-roadsters`, so they never touch production [confirmed: `package.json`].

## 11.2 Results

| Suite | Result | When | Evidence |
|---|---|---|---|
| `test:live` | **7 / 7 pass** | Run while writing this document (2026-11-01) | `ℹ tests 7 … pass 7 fail 0` |
| `test:rules` | **40 / 40 pass** | Run while writing this document (2026-11-01), in the emulator | `ℹ tests 40 … pass 40 fail 0` |
| `test:functions` | **18 / 18 pass** | Run while writing this document (2026-11-01), in the emulators | `ℹ tests 18 … pass 18 fail 0` |

These runs test the **repository's** rules and functions in emulators. They do not prove what is deployed in production matches them. The deploy history (functions, then rules on 2026-11-01) suggests it does [inferred].

Earlier the same day, before the v0.2 launch: rules 40/40, functions 18/18, live 7/7, and a two-player Playwright browser test of KILLER STATUS (29/29 checks).

For v0.2.1 (all run on 2026-11-01 against the final code):

| Suite | Result | Covers |
|---|---|---|
| `test:live` | 7 / 7 | now also `join.kc` and protocol 6 |
| `test:functions` | 18 / 18 | race gap test changed to 60 s |
| Two-player KILLER STATUS browser test | 29 / 29 | regression after the aura and save changes |
| VOID CROWN browser test (two players) | 9 / 9 | 9 kills → CRIMSON FLAME, 10th kill → VOID CROWN on both screens, kill feed, `/pkclear`, `?mock&killer=void` |
| Race-save browser test (fake one-ticket backend) | 5 / 5 | RACE AGAIN during a slow save, too-soon countdown, ticket retry; the old `account.js` fails 4 of 5 |

Since 2026-11-02 the browser tests are in the repository (`game/tests/browser/`, `npm run test:browser`; D13 fixed), and GitHub Actions runs every suite on each push to `main` (R10 fixed, see below). The CI runner is pinned to `ubuntu-24.04`: Playwright 1.56 has no Chromium for `ubuntu-latest` (Ubuntu 26.04) yet. The runner has 2 cores (private repository), so software WebGL draws ~0.4 fps there: the browser tests wait for frames and states instead of fixed sleeps, skip the race countdown, pause the other player's game loop while a second page compiles its shaders (`hold`; ~11 min instead of ~25 s otherwise) (`tests/browser/browser.mjs` helpers), the test relay keeps silent players for 15 min (`LIVE_IDLE_MS`, only with `LIVE_MOCK=1`; one frame can block a page for 30+ s there), and use the full Chromium rather than the headless shell (3x faster in a race, measured 2026-11-02).

For v0.2.4 (R7, R8, R5/R6, R12, D2, D4, D9-D12; all run on 2026-11-03 against the final code):

| Suite | Result | Covers |
|---|---|---|
| `test:live` | 32 / 32 | relay 13 (incl. kill saving, shop cars, protocol 7), constants 10, production server 8, launcher 1 |
| `test:rules` | 33 / 33 | kill and KILLER STATUS writes refused (replacing 11 tests of the old client limits) |
| `test:functions` | 26 / 26 | `relayKills`, per-road minimums, daily cap, ticket clean-up |
| `test:browser` | rivals 12/12, save 5/5, void 9/9, killer 29/29 | on the dev server. The first full run had killer 27/29: once, the R key press in the RACE Killer check wasn't registered (the 10-minute wait then also let the automatic LOW graphics switch fail the menu-tier check); a re-run passed 29/29, and the check now prints the game state when it fails |

For v0.2.7 (C2, C3, relay connection limits, race unit tests; 2026-11-03):

| Suite | Result | Covers |
|---|---|---|
| `test:live` | 57 / 57 | relay 17 (new: connections per address, per minute, still signing in, this PC exempt), constants 10, production server 11 (new: CSP header, report-only vs enforced, `/csp-report`), launcher 1, race rules 18 |
| `test:game` | 18 / 18 | `tests/race.test.mjs`: gates (counted, missed beside, MISSED GATE), laps and best lap, finish and place, FREE ROAM without gates, SIGNAL LOST respawn, damage by kind, dodging, takedown loot and streaks, player wreck and respawn, ranking (finishers first), rubber band limits, rival fire (6 s hold, target ahead), results banking (coin bonus, ranked time only in RACE). Loads the real game modules in Node (Vite SSR + a stub page, `tests/game-env.mjs`); 1.4 s. Checked by breaking two rules on purpose (loot share, gate radius): both caught |
| `test:browser` | killer 29/29, rivals 12/12, save 5/5, void 9/9 | on the dev server |
| CSP on a real build | no violations | `serve.mjs` + Chromium: reCAPTCHA, Analytics, Firebase, Google Fonts all loaded |

### v0.2.12 (2026-11-03)
| Suite | Result | Covers |
|---|---|---|
| `test:live` | 94 / 94 | relay 24 (+ moderation: filtered chat and names, `/report`, bans; + hit checks: a shot per hit, far shots and EMPs, wrecks need a hit), hit checks 11 (`hits.test.mjs`), moderation 6, constants 11 (+ the relay's weapon values = the game's), production server 12 (+ Cloudflare beacon allowed, no inline event handlers), launcher 1, race rules 18, FREE ROAM hits 4 (`roam.test.mjs`), mouse controls 7 (`controls.test.mjs`) |
| `test:game` | 29 / 29 | race 18, roam 4, controls 7 (the car flies to the aim forwards and backwards, keyboard reversing, nose height, camera follows the aim, stuck buttons) |
| `test:browser` | killer 29/29, rivals 12/12, save 5/5, void 9/9 | wrecks now follow a real hit (`shootAt`), as the relay requires since v0.2.10 |

Each new test was first run against the old code and failed there (or was checked by breaking the rule on purpose).

### v0.2.13 (2026-11-03)
`test:live` 100 / 100, `test:game` 35 / 35: `controls.test.mjs` 13 (+6: coasting stops at zero, smooth turn from a
stop, slide standing and at speed, slide decay and reset, A / D vs arrows; all fail on v0.2.12). Browser suites as
below.

### v0.2.14 (2026-11-03)
`test:live` 104 / 104, `test:game` 39 / 39: `controls.test.mjs` 16 (+3: holding fire gives ~22 bolts before it
overheats; bolt damage 21, reach ≥ 1.1 km, lock-on 2 km; each car model's bolts leave from its front bumper, left and
right in turn), `roam.test.mjs` 5 (+1: 4 bolts leave a car alive, the 5th wrecks it). The new tests fail on v0.2.13's
values (3 fail with HEAT_PER_BOLT 0.06 and BOLT_DMG 6). `hits.test.mjs` updated for 750 m/s × 1.5 s; `live.test.mjs`
for protocol 9. Browser suites as below.

### v0.2.15 (2026-11-03)
`test:live` 106 / 106, `test:game` 41 / 41: `race.test.mjs` 20 (+2: rival top speed 120 m/s × 1.02 a road; rivals
hold fire 6 s, then fire the player's bolts at a car ahead; one bolt per 0.63 s, never overheating, nothing at
1,200 m, firing at 1,000 m, an EMP pause; the rubber band test now checks that a rival ahead keeps its full speed).
`constants.test.mjs` evaluates `CAR` (it read the literal 138 of `138 * K` and missed the faster nitro) against
`PACE_MAX` 240. `test:functions` 26 / 26 (R8 and impossible-times tests on the new road limits). Rival driving was
checked with a Node simulation of 2-lap races on all 9 roads (every rival finished, no respawns). Browser suites as
below (`rivals` updated for the new speed and fire rules).

### v0.2.16 (2026-11-03)
`test:live` 110 / 110, `test:game` 45 / 45: `race.test.mjs` 24 (+4: the 35° target cone; rival seekers after a full
0.5 s lock, the player's missile values, 1.5 s apart, 3 charges and the 4.5 s recharge, none at 700 m; the spin dodge
on a low random draw and not on a high one, 6 s cooldown; the EMP with 2 cars within 95 m: the player's damage, rivals
hold fire 3 s, 11 s cooldown, none with one car near). Browser suites as below.

### v0.2.18 (2026-11-04)
`test:live` 117 / 117: `controls.test.mjs` 20 (+1: every shop car has its flame spots, three flames of the same length,
the middle one thicker, at the tail). Flame and smoke placement was checked on screenshots of each car (no browser test).

### v0.2.17 (2026-11-04)
`test:live` 116 / 116, `test:game` 51 / 51: `controls.test.mjs` 19 (+3: each car's top speed and time to 90%, turning,
health), `race.test.mjs` 27 (+3: a seeker's speed loss and stun per car, bumps split by mass, rivals on ROADSTER stats
and a TITAN refilling to 4 seekers), `constants.test.mjs` (the relay's `EMP_CAR_MAX` = the biggest `CAR_STATS.emp`).
CI fix: `shootAt` (`tests/browser/browser.mjs`) sends the shooter's position before the shot; on the ~0.4 fps runner the
relay's 1.5 s origin check could miss it and one kill was ignored (run 41760860639: killer suite 21 / 29).

## 11.3 Gaps

| Area | Automated test? |
|---|---|
| Game simulation (physics, AI, laps, weapons, coins) | **Partly**: race rules (`race.test.mjs`, v0.2.7), FREE ROAM hits on our car (`roam.test.mjs`, v0.2.8) and mouse aim / car steering (`controls.test.mjs`, v0.2.12) unit-tested; AI steering, coins and firing only through the browser tests |
| `account.js` flows (pending races, retry, unsaved-kills display) | Partly (`save.test.mjs`: pending races, retry) |
| `src/online/live.js` client (reconnect, interpolation) | Indirectly, by `killer.test.mjs` (reconnect) |
| Relay: rate limits, size limits, idle drop, `die` / `hit` validation, `err full`, `err again` | Partly (KILLER STATUS, kill saving, shop cars, invalid kills, connection limits, moderation, hit and wreck checks) |
| Rendering / visual regressions | No (`docs:shots` makes screenshots for docs, no comparison) |
| Build smoke test | No script; builds were checked manually into a scratch `outDir` |
| CI | **GitHub Actions** since 2026-11-02 (`.github/workflows/ci.yml`): on push to `main`, pull requests and by hand; Node 22, Java 21; build + live + rules + functions + browser. Never deploys |
