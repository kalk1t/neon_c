# 11. Testing

[← Index](TECHNICAL_DOCUMENTATION.md)

## 11.1 Automated suites in the repository

| Suite | Command | Runner | Needs | Covers |
|---|---|---|---|---|
| Relay, shared values, production server | `npm run test:live` | `node:test` | nothing (relay in-process, `allowMock`; `serve.mjs` on free ports with a made-up `dist/`) | `live.test.mjs`: KILLER STATUS counting, `killer` broadcast once, `/pkclear`, unknown commands, protocol mismatch → `err old`, persistence via `hi.x/kc`, shop cars (`hi.car`), saving kills (batched, retried, rejoin before the save). `constants.test.mjs` (D2): values that must agree across game, relay, functions and rules, and each road's minimum times recomputed from `src/roads.js`. `serve.test.mjs` (R5/R6): caching, `/privacy`, path traversal, unknown hosts, `/live` through to the relay process, relay crash → restart, relay stops with the server. `launcher.test.mjs` (R12): log rotation without gaps |
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

## 11.3 Gaps

| Area | Automated test? |
|---|---|
| Game simulation (physics, AI, laps, weapons, coins) | **Partly**: the browser tests exercise races, FREE ROAM kills and respawns end to end; no unit tests |
| `account.js` flows (pending races, retry, unsaved-kills display) | Partly (`save.test.mjs`: pending races, retry) |
| `src/online/live.js` client (reconnect, interpolation) | Indirectly, by `killer.test.mjs` (reconnect) |
| Relay: rate limits, size limits, idle drop, `die` / `hit` validation, `err full`, `err again` | Partly (KILLER STATUS, kill saving, shop cars, invalid kills) |
| Rendering / visual regressions | No (`docs:shots` makes screenshots for docs, no comparison) |
| Build smoke test | No script; builds were checked manually into a scratch `outDir` |
| CI | **GitHub Actions** since 2026-11-02 (`.github/workflows/ci.yml`): on push to `main`, pull requests and by hand; Node 22, Java 21; build + live + rules + functions + browser. Never deploys |
