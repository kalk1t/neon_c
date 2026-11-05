# 5. Startup and configuration

[← Index](TECHNICAL_DOCUMENTATION.md)

## 5.1 npm scripts (`package.json`) [confirmed]

| Script | Does | Notes |
|---|---|---|
| `dev` | `vite` dev server (HMR) + relay | `LIVE_MOCK=1` enables `mock:Name` test logins on the relay (`server/live.js:296`) |
| `build` | `vite build` → `dist/` | Only the working folder's own `dist/`; production doesn't serve it (since 2026-11-02, R3 fixed) |
| `preview` | `vite preview` | |
| `serve` | `node server/serve.mjs --port 8080` | **Production entry point**, run by the launcher inside a release folder. No build. `vite preview` until v0.2.4 (R5) |
| `release` | `node server/release.mjs` | Builds a commit into `neon_c/live/releases/<sha>` and switches production to it; `-- --rollback` |
| `deploy` | `node server/deploy.mjs` | `npm run deploy -- functions client rules` (any subset; always in that order, each behind its tests; `--dry-run`); no parts = help only. Before 2026-11-02 it was `vite build && firebase deploy` (everything at once, R4) |
| `test:live` | `node --test` on 9 files: relay (`live`), hit checks (`hits`), moderation, constants, production server (`serve`), launcher, race rules, FREE ROAM hits (`roam`), mouse controls (`controls`) | no emulator; the release gate |
| `test:game` | `node --test` on `race`, `roam`, `controls` | game modules in Node (Vite SSR) |
| `test:rules` | `firebase emulators:exec --only firestore --project demo-roadsters …` | needs Java 21+ |
| `test:functions` | emulators (auth, firestore, functions), project `demo-roadsters` | |
| `docs:shots` | `docs/tools/capture-screens.mjs` | uses `CHROME_PATH` |

## 5.2 Production start sequence [confirmed from scripts; rehearsed on port 8099 on 2026-11-02]

Since 2026-11-02 production serves **releases**: git worktrees of single commits under `neon_c/live/releases/<sha>`, each with its own `node_modules`, copied `.env.local` and built `dist/`. `neon_c/live/current.txt` names the served one; `history.txt` lists releases in the order they went live.

```mermaid
sequenceDiagram
  participant TS as Task Scheduler
  participant L as live\launcher.mjs (via live\start.cmd)
  participant NPM as npm run serve (in releases\<sha>)
  participant V as serve.mjs :8080
  participant R as relay.mjs (child process, :8091)
  participant CF as cloudflared (task "Roadsters Race tunnel", SYSTEM)
  TS->>L: boot / restart (S4U, restart every 1 min, 999×)
  loop until the task stops
    L->>L: read current.txt; wait while releases\<sha>\dist\index.html is missing
    L->>NPM: "starting release <sha>" >> live\server.log
    NPM->>V: node server/serve.mjs --port 8080 (no build)
    V->>R: fork relay.mjs (restarted if it stops)
    V->>R: configurePreviewServer → liveRelay({projectId: VITE_FIREBASE_PROJECT_ID, allowMock: false})
    R->>V: handle upgrade /live, middleware /live/count
    Note over L,V: every 2 s: current.txt changed → the launcher taskkills its own child → loop; crash → 2 s → loop
  end
  CF-->>V: https://roadstersrace.com → http://localhost:8080
```

- The launcher (`server/launcher.mjs`, Node) loops itself. A stopped server comes back in about 2 s, and a changed `current.txt` switches the server within about 2 s. Task Scheduler's restart policy only covers the launcher itself dying (`start.cmd` exits 1). It replaced a batch-only loop on 2026-11-02, because a normal (non-admin) shell gets *Access denied* when it runs `taskkill` on processes the S4U task started. So the switch must happen inside the launcher.
- `npm run release` (`server/release.mjs`): `git worktree add --detach` → copy `.env.local` → `npm ci` → `npx vite build` → `npm run test:live` in the release (the gate) → write `current.txt` and history → append to `live/release.log` (not `server.log`, which the launcher holds open: EBUSY) → (if `live/launcher.mjs` is missing, stop here and point to `install-autostart.ps1`) → wait for the launcher's `starting release <sha>` line, then check that the served bundle carries the version and exists in that release's `dist/` **and** that the launcher logged `starting release <sha>` (two releases can build identical bundles). It keeps the last 3 releases. `--rollback` switches to the release live before the current one (an undo; a second rollback goes forward again).
- `restart-server.ps1` (admin) stops the task, kills whatever listens on 8080 and starts the task: same release, no build.
- **R2/R3 fixed:** a reboot or crash restarts the same built release; the working tree and `game/dist` are never served.
- Before 2026-11-02 the task ran `server/start-server.cmd` in `game/`, whose `serve` was `vite build && vite preview`: every start deployed the working tree. The switch-over (`install-autostart.ps1` as admin) was done on 2026-11-02 at 8:45 local time; `start-server.cmd` was then deleted. The first release served is `1843f2c` [observed: `live/server.log`, task action].

## 5.3 Client boot [confirmed: `src/main.js`, `src/online/index.js:31-42`]

1. `index.html` loads `src/main.js` (Vite module) and Google Fonts.
2. `main.js` builds the renderer and scene, starts loading the car GLB (`sculptReady`, which gives up waiting after 20 s, `src/main.js`), and builds the map (`maps.js` → city, environment, roads, course, props).
3. `loadBackend()` picks `mock.js` when `import.meta.env.DEV && ?mock`, otherwise `firebase.js` with `FIREBASE_CONFIG` from the `VITE_FIREBASE_*` variables (`src/online/index.js:31-42`).
4. `account` subscribes to auth state; on sign-in it records the visit (`players/{uid}`) and listens to `users/{uid}` (`onSnapshot`).
5. The menu shows `v${__APP_VERSION__}` and the FREE ROAM online count (`GET /live/count`, refreshed every 10 s, `refreshOnlineCount` in `src/roam.js`).

## 5.4 Configuration inventory

Values are **never** shown here; only names. `.env.local` exists on the production PC and was not read.

| Name | Where set | Read by | Required | Effect |
|---|---|---|---|---|
| `VITE_FIREBASE_API_KEY` | `.env.local` | `src/online/index.js:31` | yes (prod) | Firebase web config (public by design, but kept out of git) |
| `VITE_FIREBASE_AUTH_DOMAIN` | `.env.local` | `src/online/index.js:32` | yes | Comment in `vite.config.js` says it may be `roadstersrace.com` thanks to the `/__` proxy; the actual value is [unknown] |
| `VITE_FIREBASE_PROJECT_ID` | `.env.local` | client + relay (`server/live.js:282`) | yes | Relay: JWT issuer/audience, REST profile URL |
| `VITE_FIREBASE_APP_ID` | `.env.local` | `src/online/index.js:34` | yes | |
| `VITE_FIREBASE_MEASUREMENT_ID` | `.env.local` | `src/online/index.js:35` | optional | Analytics |
| `VITE_APPCHECK_SITE_KEY` | `.env.local` | `src/online/firebase.js:32` | optional | App Check (reCAPTCHA Enterprise); **not set up yet** (rollout R9 started 2026-11-02). Dev builds also read `VITE_APPCHECK_DEBUG_TOKEN` (`src/online/firebase.js:37`) |
| `VITE_FUNCTIONS_EMULATOR` | dev env | `src/online/firebase.js:39` | dev only | Use the local functions emulator |
| `RELAY_SECRET` | `.env.local` of the release (relay only, never bundled) + Firebase Secret Manager (`relayKills`) | Shared secret the relay signs kill saves with (v0.2.4). Unset: kills aren't saved (logged at start) |
| `RELAY_KILLS_URL` | `.env.local` (optional) | Overrides the `relayKills` address |
| `LIVE_MOCK` | process env | `server/live.js:296` | dev only | `1` = accept `mock:Name` tokens. Ignored by `preview` (always `false`, `server/live.js:297`) |
| `ENFORCE_APP_CHECK` | `functions/.env` | `functions/index.js:25` | optional | `true` = callables reject requests without App Check. **`functions/.env` does not exist** in the repo working tree [confirmed], so the deployed functions were built without it [inferred] |
| `CHROME_PATH` | shell | `docs/tools/capture-screens.mjs` | tool only | |

Other configuration files:

| File | Purpose | Notes |
|---|---|---|
| `vite.config.js` | `__APP_VERSION__`, `livePlugin` (dev only since v0.2.4), `allowedHosts`, `/__` proxy (dev; `serve.mjs` in production), the `three` chunk (D10) | Loads `VITE_*` and `RELAY_*` from `.env.local`; only `VITE_*` reach the browser |
| `firebase.json` | rules, indexes, functions (`nodejs22`), emulators firestore 8085 / functions 5001 / auth 9099 | No `hosting` section since 2026-11-02 (it was never deployed, Q1) |
| `.firebaserc` | project alias | git-ignored; `.firebaserc.example` committed |
| `firestore.indexes.json` | composite indexes | **empty**; current queries need only single-field indexes [inferred from `src/online/firebase.js:182-193`] |
| `functions/rules.js` | economy limits | must stay in step with `src/online/index.js` (comment, `functions/rules.js:1-4`) |
| `src/online/protocol.js` | `LIVE_PROTOCOL` (16), `KILLER_KILLS`, `VOID_KILLS`, `NICK_RE`, `CHAT_MAX`, `CAR_IDS` | shared by client and relay |

**Runtime settings stored per device** (`localStorage`) are listed in [05-data.md](05-data.md#localstorage).
