# 12. Build, deploy and operations

[← Index](TECHNICAL_DOCUMENTATION.md)

## 12.1 Build

- `vite build` produces `dist/` (hashed `assets/index-*.js`, CSS, and `public/` copied as-is). `__APP_VERSION__` comes from `package.json` and is shown on the menu as `v0.2.0` [confirmed: `vite.config.js`; observed in the live bundle].
- Since v0.2.4 (D10) three.js is its own chunk (`assets/three-*.js`, ~676 KB, ~171 KB gzipped), cached across releases; the game's own chunk is ~216 KB (~80 KB gzipped); the Firebase SDK is a lazily loaded chunk (~568 KB). `chunkSizeWarningLimit` is 700 KB because the two libraries are single files [confirmed: build output 2026-11-03].
- `npm run build` inside `game/` only writes `game/dist`, which production no longer serves (since 2026-11-02; before that it changed the live site at once, R3). Production builds happen inside each release folder (`server/release.mjs`). Builds are reproducible: the release of `d854d41` produced the same `index-BAcZtIrA.js` the old set-up served [observed in the rehearsal].

## 12.2 Environments

| Environment | How | Backend | Relay |
|---|---|---|---|
| Local dev | `npm run dev` (port 5173 default) | real Firebase (`.env.local`), or `?mock` for the mock backend | in-process; mock logins only with `LIVE_MOCK=1` |
| Emulator tests | `npm run test:rules` / `test:functions` | Firebase emulators, project `demo-roadsters` | — |
| Production | PC: the release named in `neon_c/live/current.txt`, `npm run serve` (`server/serve.mjs`, since v0.2.4) on 8080, Cloudflare Tunnel → roadstersrace.com | Firebase project `roadsters-race` | `server/relay.mjs`, its own process (port 8091), from that release |

There is no staging environment [confirmed: no second project or config in the repo].

## 12.3 Deploy (what is actually done)

| Part | Command | Who |
|---|---|---|
| Cloud Functions | `npm run deploy -- functions` (`test:functions`, then `firebase deploy --only functions`) | owner (or Claude with permission) |
| Firestore rules | `npm run deploy -- rules` (`test:rules`, then `firebase deploy --only firestore:rules`) | owner (or Claude with permission) |
| Web client + relay | commit/tag/push → `npm run release [-- <tag>]` (builds the commit in its own folder, `test:live` gate, switches, verifies; `-- --rollback` undoes). Until 2026-11-02: admin `restart-server.ps1`, which rebuilt the working folder | owner or Claude with permission (no admin needed) |

**Order for releases that change rules and client together:** functions → client → rules [confirmed: owner's release note for v0.2]. v0.2.1 had no rules change: functions → client. Since 2026-11-02 `npm run deploy -- functions client rules` (`server/deploy.mjs`) enforces this order whatever order the parts are given in, stops at the first failed test or deploy, and refuses uncommitted `functions/` or rules; dry run checked (functions 18/18 + Firebase validation, rules 40/40 + validation).

**Version:** bump `package.json` (and the lock file) every release; the menu label must always show [confirmed: owner rule]. Don't edit JSON with PowerShell `Set-Content` (adds a BOM).

**Git:** `game/` is the repo; tags `v0.2`, `v0.2.1` (`d854d41` on `main` came after the tag), `v0.2.2`, `v0.2.3` [confirmed]. GitHub Actions CI runs on every push to `main` (see [08](08-testing.md)). `neon_c/` (assets, milestones, archive, this techplan) is **not** under version control [confirmed].

### Conflicting documentation

| Document | Says | Reality |
|---|---|---|
| ~~`DEPLOY.md`, `firebase.json` `hosting`~~ | ~~Firebase Hosting~~ | Fixed 2026-11-02 (D7, R4, Q1): DEPLOY.md describes the PC set-up; no hosting section; web.app was never deployed |
| `milestones/README.md` | M6, M7, M8 ⬜ | M8 is done (v0.2 server coins); M7 is ongoing; M6 (two-account go-live test) not confirmed |

## 12.4 Operations

| Task | How | Evidence |
|---|---|---|
| Autostart and crash restart | Task "Roadsters Race server" (S4U, restart every 1 min up to 999×) runs `neon_c/live/start.cmd` → `launcher.mjs`, which switches releases when `current.txt` changes and restarts the server itself within 2 s; since v0.3 it also installs a newer launcher from a release (`LAUNCHER` number, exit 75, `start.cmd` loops; R12); task "Roadsters Race tunnel" (SYSTEM) | `server/install-autostart.ps1`, `server/start-live.cmd`, `server/install-tunnel.ps1` |
| Logs | `neon_c/live/server.log` (release switches and starts, `[serve]` relay starts / restarts, `[live]` relay lines: joins, kills, kill saves, sign-in failures; no chat text since v0.2.4). Rotated at 5 MB into `server.1-3.log` by the launcher (R12; **not yet installed on the PC as of 2026-11-05**: run `install-autostart.ps1` as administrator once; later launchers then arrive with releases). Older log: `game/server/server.log` | `server/launcher.mjs`, `server/release.mjs`, `server/live.js` |
| Rollback | `npm run release -- --rollback`; the last 3 releases stay built | `server/release.mjs` |
| Relay secret | `RELAY_SECRET` in Secret Manager (`firebase functions:secrets:set`) and in `game/.env.local` (copied into releases); see `docs/technical/10-operations.md#relay-secret` | `functions/index.js`, `server/relay.mjs` |
| Health check | `GET https://roadstersrace.com/live/count`; menu online count | `server/live.js:287` |
| Firebase monitoring | Firebase console (functions logs, Firestore usage); no alerts configured in the repo | [unknown] |
| Backups | Firestore scheduled backups since 2026-11-03 (R11): daily, kept 7 days, and weekly on Sunday, kept 14 weeks; restore into a new database | `docs/technical/10-operations.md#backups` |
| Downtime | PC off, asleep, rebooting or offline means the site **and** FREE ROAM are down; Firebase parts keep working for open tabs | [inferred] |
| Restart effect | All FREE ROAM sessions drop and reconnect (backoff up to 15 s); clients on an older protocol reload (≤ once a minute) | `src/online/live.js:93,171` |

Day-to-day running is described for the owner in `milestones/M7-running-it.md` and `docs/technical/10-operations.md`.
