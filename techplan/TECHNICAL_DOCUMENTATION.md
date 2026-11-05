# Roadsters Race: Technical Documentation

> **Scope.** This documents the system **as it is** on 2026-11-01. Written at game version **0.2.0** (git `6d125bc`, tag `v0.2`; no code was changed while writing it), updated for **0.2.1** (VOID CROWN rank, race-save fixes, live protocol 6), 0.2.2, 0.2.3, **0.2.4** (tech-debt fixes, live protocol 7) and every release since, through **0.2.26** (2026-11-05, no pause in FREE ROAM; live protocol 16; the status line below lists them).
> **Audience.** Engineers who will maintain, extend or operate the game.
> **Repository.** `C:\Users\User\Projects\neon_c\game` (git remote `kalk1t/Roadsters-Race`, private, branch `main`). All paths below are relative to `game/` unless stated otherwise.

## Evidence labels

Every claim that is not obvious from a file citation carries one of these labels:

| Label | Meaning |
|---|---|
| **[confirmed]** | Read in the code or config, or observed by running it (the cited file is the source of truth) |
| **[inferred]** | Follows from the code, comments or docs, but wasn't directly observed |
| **[unknown]** | Not answerable from the repository; listed in [Coverage and open questions](12-coverage-open-questions.md) |

"Repository config" means what the files say. "Production" means what roadstersrace.com actually runs. Where the two differ, both are stated.

## Document map

The material is too large for one file, so this page is the index plus sections 1 to 3. Each remaining section has its own file.

| # | Section | File |
|---|---|---|
| 1 | Overview | this page |
| 2 | Technology stack | this page |
| 3 | Repository structure | this page |
| 4 | Architecture (Mermaid) | [01-architecture.md](01-architecture.md) |
| 5 | Startup and configuration | [02-startup-config.md](02-startup-config.md) |
| 6 | Core workflows (sequence diagrams) | [03-workflows.md](03-workflows.md) |
| 7 | Components and functions | [04-components.md](04-components.md) |
| 8 | Data architecture | [05-data.md](05-data.md) |
| 9 | APIs and integrations | [06-apis-integrations.md](06-apis-integrations.md) |
| 10 | Security | [07-security.md](07-security.md) |
| 11 | Testing | [08-testing.md](08-testing.md) |
| 12 | Build, deploy and operations | [09-build-deploy-ops.md](09-build-deploy-ops.md) |
| 13 | Technical debt and risks | [10-tech-debt-risks.md](10-tech-debt-risks.md) |
| — | Constants and important values (table) | [11-constants.md](11-constants.md) |
| 14 | Coverage and open questions | [12-coverage-open-questions.md](12-coverage-open-questions.md) |
| — | Browser tests (moved into `game/tests/browser/` 2026-11-02) | [browser-tests/](browser-tests/README.md) |

The repository also has its own docs: `docs/technical/01…11` (written for maintainers), `docs/manual/01…10` (player manual), `docs/changelog.md`, `docs/feature-inventory.md` and `docs/glossary.md`. This tech plan is an architect's cross-section of the system. For detail on gameplay subsystems, those docs remain the reference.

---

## 1. Overview

**Roadsters Race** is a browser game: an anti-gravity neon car racer set in one 3D city map, **SKYLINE CITY**. It runs entirely in the browser (three.js, WebGL) and has these online parts:

- **Race mode.** You race against 10 AI rivals over 2 laps on one of 9 roads ("routes" through the same city) [confirmed: `src/roads.js:11`, `src/maps.js:13`, `src/ai.js:3`]. Coins collected on the track are banked to your account by a Cloud Function.
- **FREE ROAM.** A shared city of up to 24 players, connected over WebSocket to a relay running on the owner's PC [confirmed: `server/live.js:47`]. Players fight each other (bolts and seekers; spin dodge and backflip; the EMP was removed in v0.2.21), and kills count towards a lifetime kill stat and **KILLER STATUS** (3 kills give the CRIMSON FLAME aura, 10 the VOID CROWN) [confirmed: `src/online/protocol.js:7`].
- **Accounts.** Google sign-in through Firebase Auth. A public profile `users/{uid}` holds nickname, coins, best times, kills and killer status; a private record `players/{uid}` holds email and visits [confirmed: `firestore.rules:38,65`].
- **Shop.** Cars with their own stats since v0.2.17 (`CAR_STATS`: speed, handling, health, cooldowns, seekers): ROADSTER (free), VIPER (2,500 coins), TITAN (7,500 coins), bought through the `buyCar` function [confirmed: `functions/rules.js:20`].
- **Leaderboards.** Top 10 by coins, by best time per road, and by kills, plus your own rank [confirmed: `src/account.js:14`, `src/online/firebase.js:182-193`].

**Production topology** [confirmed from `server/*.ps1`, `server/start-live.cmd`, `server/release.mjs`, `vite.config.js`; and observed: the live bundle served 0.2.0 at the v0.2 launch, 0.2.1 after it]:
the owner's Windows PC runs `server/serve.mjs` (`vite preview` before v0.2.4) on port 8080 inside a **release folder** (`neon_c/live/releases/<commit>`, a git worktree made by `npm run release`), which serves that release's built `dist/` **and** passes the FREE ROAM WebSocket `/live` to the relay, its own process (`server/relay.mjs`). Cloudflare Tunnel publishes it as `roadstersrace.com`. Firebase (project `roadsters-race`) provides Auth, Firestore and five Cloud Functions (`startRace`, `finishRace`, `buyCar`, `relayKills`, `cleanRaceTickets`). Firebase Hosting is not used (never deployed; no `hosting` section since 2026-11-02, Q1).

**Status.** v0.2 has been live since 2026-11-01 (release order: functions, then server restart, then rules). v0.2.1 (VOID CROWN rank, race-save fixes, live protocol 6) is committed and pushed (`4363a4c`, tag `v0.2.1`, plus `d854d41`: race gap 60 s); its Cloud Functions were deployed on 2026-11-01, and the client and relay are live (bundle checked) [confirmed: git, deploy output, live bundle]. On 2026-11-02 releases replaced rebuild-on-restart (R2/R3, see [02 §5.2](02-startup-config.md)); v0.2.2 (releases, `npm run deploy`, App Check tokens) and v0.2.3 (TRACK FLYOVER removed from the menu; rivals at the player's top speed × `road.ai`; rival damage and fire rate × 1.5 via `RIVAL_POWER`; browser tests in the repo incl. `rivals.test.mjs`; GitHub Actions CI on `ubuntu-24.04`) were released the same day with `npm run release` (v0.2.3 = `28a4e30`, tag `v0.2.3`, live bundle checked) [confirmed]. **v0.2.4** (2026-11-03, committed; deploy pending): the relay saves kills and KILLER STATUS (R7), harder race checks and a daily coin cap (R8), `server/serve.mjs` + the relay as its own process (R5/R6), log rotation (R12), shop cars in FREE ROAM (D11), and D2, D4-D6, D8-D10, D12; see [13](10-tech-debt-risks.md#133-status-2026-11-03-v024). **v0.2.5-v0.2.22** (2026-11-03 to 2026-11-05, each tagged and pushed; the owner deploys with `npm run deploy`): S3/S5/S6 security fixes (v0.2.10), moderation, the PULSE blaster rework and per-car bolts, faster cars, rivals with the player's arsenal, per-car stats (`CAR_STATS`), thruster flames, 3D bolts per car (v0.2.19), double health and slower rival seekers with 2 s protection after a wreck and exact positions (v0.2.20), the EMP removed and no RESTART in FREE ROAM (v0.2.21), and loops by hand with the mouse, the backflip (2), the air brake (Space) and the seeker on 1 (v0.2.22, `LIVE_PROTOCOL` 15), then a full project review (v0.2.23: bug fixes, dependency audit clean, docs caught up; see [13.4](10-tech-debt-risks.md#134-project-review-2026-11-05-v0223)), then v0.2.24 (a hidden tab leaves FREE ROAM, B7; see-through pause; `LIVE_PROTOCOL` 16) and v0.2.25 (backflip on G, 2 free), v0.2.26 (no pause in FREE ROAM: the car flies on behind the Esc menu, a wreck respawns). Per-version detail: `game/docs/changelog.md`.

## 2. Technology stack

| Layer | Technology | Version (declared) | Evidence |
|---|---|---|---|
| Language | JavaScript (ES modules), GLSL shaders inline | — | `package.json` `"type": "module"` |
| 3D engine | three.js | `^0.186.1` | `package.json` |
| Build / dev server / prod server | Vite | `^8.3.5` | `package.json`, `vite.config.js` |
| Backend SDK (client) | Firebase JS SDK (Auth, Firestore, Functions, Analytics, App Check) | `^13.1.0` | `package.json`, `src/online/firebase.js:4-7` |
| Realtime relay | Node `ws` WebSocket server, its own process in production (`server/relay.mjs`, v0.2.4); inside Vite on the dev server | `ws ^8.22.0` | `server/live.js`, `vite.config.js` |
| Token verification | `jose` (Google JWKS) | `^6.2.14` | `server/live.js:65` |
| Cloud Functions | `firebase-functions` v2 (`onCall`), `firebase-admin` | `^7.4.0`, `^14.5.0` | `functions/package.json`, `functions/index.js:14` |
| Functions runtime | Node.js 22, region us-central1 | — | `firebase.json`, `functions/index.js:22` |
| Database | Cloud Firestore (location nam5) | — | `firestore.rules`; location [confirmed in Firebase console earlier this session, not in the repo] |
| Auth | Firebase Auth, Google provider (popup, redirect fallback) | — | `src/online/firebase.js` |
| Hosting (production) | Windows PC + `server/serve.mjs` and `server/relay.mjs` (per-commit release folders) + Cloudflare Tunnel (`cloudflared`) | — | `server/start-live.cmd`, `server/release.mjs`, `server/install-tunnel.ps1` |
| Process supervision | Windows Task Scheduler (2 tasks) | — | `server/install-autostart.ps1`, `server/install-tunnel.ps1` |
| Testing | `@firebase/rules-unit-testing ^6`, Firebase emulators (`firebase-tools ^15.34.2`), `node:test`, `playwright-core ^1.56.1` (screenshots / ad-hoc browser tests) | — | `package.json`, `tests/*.mjs` |
| Fonts | Google Fonts (Orbitron, Rajdhani), loaded at runtime | — | `index.html` |
| 3D assets | GLB models (Meshy-generated), WebP/PNG textures | — | `public/models`, `public/tex`, `public/fx` |

There is no framework (no React or Vue). The UI is DOM plus CSS (`src/style.css`) driven from `src/main.js`, `src/race.js`, `src/roam.js` and `src/hud.js`. There is no TypeScript and no linter config [confirmed: no `tsconfig`, `.eslintrc` or `prettier` files in the repo root].

## 3. Repository structure

```text
neon_c/
├─ game/                      ← the git repository (everything below is relative to here)
│  ├─ index.html              single page: menu, HUD, shop, chat, dialogs (DOM)
│  ├─ vite.config.js          app version define, live relay plugin, allowed hosts, /__ auth proxy
│  ├─ package.json            scripts, deps (version 0.2.1 = label in the menu)
│  ├─ firebase.json           firestore rules/indexes, functions, hosting (unused in prod), emulators
│  ├─ firestore.rules         security rules (the only guard for client writes)
│  ├─ firestore.indexes.json  empty (no composite indexes declared)
│  ├─ .env.example            names of the VITE_FIREBASE_* variables (real values: .env.local, git-ignored)
│  ├─ .firebaserc(.example)   Firebase project alias (real file git-ignored)
│  ├─ DEPLOY.md               set-up: Firebase project, npm run deploy, serving from the PC
│  ├─ src/
│  │  ├─ main.js              ~700 lines: boot, game states, loop, menus, settings, window.NEON (dev)
│  │  ├─ render.js, world.js  renderer + post-processing; map, roads, coins (D1, v0.2.6)
│  │  ├─ race.js, aim.js      race rules, results; aim, lock-on, player weapons
│  │  ├─ camera.js, roam.js   cameras; FREE ROAM
│  │  ├─ clock.js             the game clock
│  │  ├─ car.js  ai.js  physics.js  track.js  roads.js  maps.js  course.js  coins.js
│  │  ├─ city.js  env.js  skyprops.js  props.js  ads.js  materials.js  geo.js  fx.js  nitro.js
│  │  ├─ killer.js            Killer auras (CRIMSON FLAME, VOID CROWN)
│  │  ├─ hud.js  chat.js  shop.js  cockpit.js  touch.js  input.js  audio.js  sysreq.js
│  │  ├─ account.js           account state, saving races/kills/killer, ranks
│  │  ├─ style.css
│  │  └─ online/
│  │     ├─ index.js          backend loader, shared limits (NICK_RE, caps, SHOP_CARS)
│  │     ├─ firebase.js       real backend (Auth, Firestore, Functions, Analytics, App Check)
│  │     ├─ mock.js           ?mock backend for dev (localStorage "database")
│  │     ├─ live.js           LiveRoom: FREE ROAM WebSocket client
│  │     ├─ protocol.js       LIVE_PROTOCOL, KILLER_KILLS, VOID_KILLS, NICK_RE, CHAT_MAX, CAR_IDS (shared with the relay)
│  │     └─ analytics.js      consent-gated analytics queue
│  ├─ server/
│  │  ├─ live.js              FREE ROAM relay (ws, jose, Firestore REST; saves kills via relayKills; Vite plugin for dev)
│  │  ├─ serve.mjs            production web server (npm run serve, v0.2.4); starts relay.mjs
│  │  ├─ relay.mjs            the relay as its own process (v0.2.4)
│  │  ├─ deploy.mjs           npm run deploy -- functions client rules (safe order, tests first)
│  │  ├─ release.mjs          npm run release: commit → live/releases/<sha>, test, switch, verify; --rollback
│  │  ├─ launcher.mjs         launcher (installed as live/launcher.mjs): serves current.txt, switches, restarts
│  │  ├─ start-live.cmd       task entry: node launcher.mjs; restarts it (installed as live/start.cmd)
│  │  ├─ restart-server.ps1   admin: stop task, free 8080, start (same release, no build)
│  │  ├─ install-autostart.ps1  Task Scheduler "Roadsters Race server" → live/start.cmd
│  │  └─ install-tunnel.ps1   Task Scheduler "Roadsters Race tunnel" (cloudflared, token param)
│  ├─ functions/
│  │  ├─ index.js             startRace, finishRace, buyCar (onCall)
│  │  └─ rules.js             RULES (economy limits), ROADS, timeField, SHOP_PRICES
│  ├─ tests/
│  │  ├─ rules.test.mjs       Firestore rules (emulator)
│  │  ├─ functions.test.mjs   Cloud Functions (emulators)
│  │  └─ live.test.mjs        relay (in-process, no emulator)
│  ├─ tools/roadgen/          offline road generator (gen/emit/check/metrics, roads.json)
│  ├─ tools/fx/voidaura.mjs   builds the VOID CROWN textures (public/fx/voidaura_*.webp)
│  ├─ public/                 static: models, tex, fx, ui, cockpit art, brand, privacy/terms pages
│  ├─ docs/                   manual, technical docs, changelog, glossary, feature inventory, tools
│  └─ dist/                   build output (git-ignored), served in production
├─ Assets/  meshy/            source art (outside git)
├─ archive/maps-r1-r3/        shelved maps (outside git)
├─ milestones/                M1–M8 launch plan (outside git; ⚠ status column stale)
└─ techplan/                  this documentation
```

Size: about 9,500 lines of JavaScript plus rules across `src`, `server` and `functions` [confirmed: `wc -l`].
