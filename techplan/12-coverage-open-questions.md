# 14. Coverage and open questions

[← Index](TECHNICAL_DOCUMENTATION.md)

## 14.1 What was examined

| Area | Coverage | How |
|---|---|---|
| Manifests, scripts, Vite / Firebase config, `.gitignore`, env **names** | Full | read |
| `firestore.rules` | Full | read |
| `functions/index.js`, `functions/rules.js` | Full | read |
| `server/live.js` | Full | read |
| `server/*.ps1`, `start-live.cmd`, `release.mjs` (`start-server.cmd` deleted 2026-11-02) | Full | read; `release.mjs` written and rehearsed 2026-11-02 |
| `src/online/*` | Full | read |
| `src/account.js` | Most (save, kill, killer, errors, ranks) | read |
| `src/main.js` | Start-up, state machine, menus, `window.NEON` (dev only since v0.2.4), main loop | read (~700 lines since the v0.2.6 split) |
| `src/race.js`, `aim.js`, `roam.js`, `camera.js`, `render.js`, `world.js` | Race rules, weapons, FREE ROAM, cameras, renderer, world (split from `main.js` in v0.2.6) | read; `race.js` unit-tested (`tests/race.test.mjs`), `roam.js` hits (`roam.test.mjs`), `aim.js` + `camera` follow + `car.js` steering (`controls.test.mjs`, v0.2.12) |
| `car.js`, `ai.js`, `physics.js`, `track.js`, `roads.js`, `maps.js`, `killer.js`, `chat.js`, `shop.js`, `touch.js`, `input.js`, `audio.js` | Constants and public API | grepped headers, exports, constants |
| `city.js`, `env.js`, `skyprops.js`, `props.js`, `ads.js`, `materials.js`, `geo.js`, `fx.js`, `nitro.js`, `interior.js`, `coins.js`, `sysreq.js`, `hud.js` | Headers, exports, constants | skimmed |
| `index.html`, `style.css` | External resources (Google Fonts); the CSP is a header from `server/serve.mjs` (v0.2.7) | skimmed; DOM structure not documented in detail |
| `tools/roadgen` | File list only | not read or run |
| `tests/*` | Suites and counts | **run**: live 7/7, rules 40/40, functions 18/18 (2026-11-01; live and functions re-run for v0.2.1) |
| Production | Live bundle version 0.2.0 and `/live/count` responding (checked at the v0.2 launch, same day) | observed earlier in the session; not re-checked for this document |
| `.env.local`, `functions/.env` | Not opened / does not exist | by policy |

Not examined: GLB/texture assets, `docs/media`, the Cloudflare dashboard, the Firebase / GCP console (indexes, backups, alerts, quotas, App Check), Windows Task Scheduler state on the PC.

## 14.2 Open questions

| # | Question | Why it matters | Where to answer |
|---|---|---|---|
| Q1 | ~~Was a Firebase Hosting site ever deployed?~~ **Answered 2026-11-02:** no. `roadsters-race.web.app` and `.firebaseapp.com` both answer 404 "Site Not Found"; the `hosting` section was then removed from `firebase.json` | — | — |
| Q2 | Was **M6** (two-real-account go-live test) ever done? | Milestone status shows ⬜ | owner |
| Q3 | **App Check:** ~~is a key planned?~~ Yes, rollout started 2026-11-02 (R9); the earliest enforce date (2026-11-05) has passed: open is the owner's go after checking the metrics | Bot and cost protection | owner / GCP console |
| Q4 | **Cloudflare config:** WAF, rate limiting, bot fight mode, cache rules, security headers, WebSocket timeouts? | DoS posture of the relay; CSP (C3) | Cloudflare dashboard |
| Q5 | Which **Firestore indexes** exist in production (the repo file is empty)? | Rank queries rely on automatic single-field indexes | Firebase console → Firestore → Indexes |
| Q6 | ~~Are there **Firestore backups**?~~ **Answered 2026-11-03:** yes, daily (7 days) and weekly (14 weeks), R11. Still open: PITR and budget alerts? | Data loss and cost | GCP console |
| Q7 | Value of `VITE_FIREBASE_AUTH_DOMAIN` in production: `roadstersrace.com` (via the `/__` proxy) or `roadsters-race.firebaseapp.com`? | Sign-in behaviour in Safari/Firefox | `.env.local` (owner) |
| Q8 | ~~Why `raceTime >= 90` for PB rank display (D4), and why +10 per killer write?~~ Both gone in v0.2.4 (D4 replaced by the server's `newTime`; the killer rule was removed with R7) | — | answered |
| Q9 | ~~Meaning of the `KNOCK` / `ROAM_KNOCK` tuple elements~~ **Answered:** `KNOCK` = [speed kept (factor), stun s], `ROAM_KNOCK` = [speed kept, stun s, push m/s, damage hp] ([11-constants](11-constants.md)) | — | — |
| Q10 | ~~When is `multi` true?~~ Answered: `multi = g.racers.length > 1`, i.e. a RACE with AI rivals (`src/race.js` `showResults`); TIME ATTACK is alone | — | — |
| Q11 | Is there a process for privacy requests (deleting `players` / `users` docs)? | Rules forbid client deletion | owner; `public/privacy.html` |
| Q12 | PC uptime expectations: sleep settings, UPS, Windows Update restart windows | R1 | owner |

## 14.3 Summary

**Documented:**
- overview, stack and repository layout
- system and client architecture, with Mermaid diagrams
- startup sequence and configuration inventory (variable names only)
- six workflows as sequence diagrams: sign-in, race save, purchase, FREE ROAM with KILLER STATUS, leaderboards, release
- per-module components and the three Cloud Functions
- Firestore model (ER diagram), relay state and `localStorage`
- the WebSocket protocol and external services
- a security threat table with findings and recommendations
- tests and their gaps
- build, deploy and operations, including documentation conflicts
- 13 risks and 14 debt items
- a table of about 120 constants with `file:line`, units, configurability and dependencies

**Verified by running:** the three test suites passed while writing this (relay 7/7, rules 40/40, functions 18/18), against the repository's code in emulators; re-run for v0.2.1 along with three browser tests (see [08](08-testing.md)).

**Kept current:** updated for v0.2.7 on 2026-11-03 (line numbers of the modules split from `main.js` were replaced by file and symbol names). Earlier: v0.2.1 on 2026-11-01. Every `file:line` citation was re-checked against the code by script (189 references; the remaining mismatches flagged by the script cite a neighbouring line on purpose, e.g. the `NICK_RE` copy in `server/live.js` is a regex literal).

**Remaining gaps:** everything in 14.2. Production cloud and Cloudflare settings can't be seen from the repository. Some rendering modules and `tools/roadgen` are covered only at the API and constant level.
