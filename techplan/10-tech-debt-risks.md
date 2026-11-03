# 13. Technical debt and risks

[← Index](TECHNICAL_DOCUMENTATION.md)

Severity: **H** high, **M** medium, **L** low. The "known issue" column refers to `docs/technical/11-known-issues-roadmap.md`.

## 13.1 Risks

| # | Risk | Severity | Evidence | Known issue | Mitigation idea |
|---|---|---|---|---|---|
| R1 | **Single point of failure:** one home PC serves the site and FREE ROAM; power, network, Windows Update reboot or sleep take everything down | H | `server/*`, `start-live.cmd` | | Move static files to Firebase Hosting or Cloudflare Pages; keep or move the relay to a small VPS |
| R2 | ~~**Restart deploys the working tree**~~ **Fixed 2026-11-02:** production serves per-commit release worktrees (`npm run release`, `server/release.mjs`, launcher `server/start-live.cmd`); `serve` no longer builds | ~~H~~ | `package.json` `serve`, `server/release.mjs` | T2 (fixed) | Done; switched over 2026-11-02 (release `1843f2c` live) |
| R3 | ~~`npm run build` in `game/` changes production at once~~ **Fixed 2026-11-02** (as R2) | ~~M~~ | release folders under `neon_c/live/releases` | T2 (fixed) | Done |
| R4 | ~~`npm run deploy` bypasses the release order and deploys unused Hosting~~ **Fixed 2026-11-02:** `npm run deploy -- <functions\|client\|rules>` (`server/deploy.mjs`) runs the named parts in the order functions → client → rules, each behind its tests, refuses uncommitted functions/rules, and prints help with no parts; `hosting` removed from `firebase.json` | ~~M~~ | `server/deploy.mjs`, `firebase.json` | | Done |
| R5 | ~~`vite preview` is not designed as a production server~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** `npm run serve` = `server/serve.mjs`, a small Node server (static `dist/` with cache headers, `/__` proxy, host check), tested by `tests/serve.test.mjs` | ~~M~~ | `server/serve.mjs` | | Done |
| R6 | ~~Relay shares the web process~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** the relay is its own process (`server/relay.mjs`), started and restarted by `serve.mjs`, 384 MB memory cap; the site keeps serving while it restarts | ~~M~~ | `server/serve.mjs`, `server/relay.mjs` | | Done |
| R7 | ~~Kills and killer status are client-saved (forgeable)~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** the relay saves them through `relayKills` (shared secret `RELAY_SECRET`); the rules refuse browser writes. Remaining: a victim's game reports its own wreck, so two accounts can still feed each other kills (S2) | ~~M~~ L | `functions/index.js` `relayKills`, `server/live.js`, `firestore.rules` | S8 (fixed) | Needs `RELAY_SECRET` set before the release (see 10-operations) |
| R8 | Race results client-claimed. **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** per-road fastest times (`ROAD_MIN_RACE_S` / `ROAD_MIN_LAP_S`), the best lap must fit the race, coins need the road's minimum real time, **2,000 coins a day**, cut claims logged. Remaining: a cheat can still claim up to the cap with plausible times (the server doesn't replay the race) | ~~M~~ L | `functions/index.js` `finishRace`, `functions/rules.js` | S1 | Replay-checking would close it fully (big job); App Check enforcement (R9) blocks scripts that don't run the game |
| R9 | No App Check. **In progress (2026-11-02):** code ready (dev debug token, `appCheckToken()`, relay forwards `hi.ac` as `X-Firebase-AppCheck` so Firestore enforcement won't break FREE ROAM); **tokens live since v0.2.2** (2026-11-02, exchange verified 200, TTL 1 h); next: watch metrics, enforce no earlier than 2026-11-05 | M | `src/online/firebase.js`, `server/live.js` identify | yes (open) | Rollout steps in `docs/technical/10-operations.md#app-check` |
| R10 | ~~No CI; tests only run by hand~~ **Fixed 2026-11-02:** GitHub Actions (`.github/workflows/ci.yml`) runs build, live, rules, functions and browser tests on every push to `main` | ~~M~~ | `.github/workflows/ci.yml` | | Done |
| R11 | No Firestore backups configured in repo | M | [unknown] | | Scheduled exports |
| R12 | ~~`server.log` grows forever and holds chat text~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** the launcher rotates it at 5 MB (3 old files kept); chat text isn't logged | ~~L~~ | `server/launcher.mjs`, `server/live.js` | | Install the new launcher once: `install-autostart.ps1` (admin) |
| R13 | `neon_c/` (assets, milestones, techplan) is not in git | L | | | Separate repo or backup |

## 13.2 Code-level debt

| # | Item | Evidence | Effect |
|---|---|---|---|
| D1 | `main.js` is 1,817 lines mixing state machine, race, roam, menus and settings | `src/main.js` | Hard to test; most game logic has no tests |
| D2 | ~~Constants duplicated~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** one definition where code can share it (`DECK_Y` in `course.js`; `NICK_RE`, `CHAT_MAX`, `CAR_IDS` in `src/online/protocol.js`); the copies that must stay separate (functions, rules, relay limits, road minimum times) are compared by `tests/constants.test.mjs`, so drift fails the tests | `tests/constants.test.mjs` | — |
| D3 | ~~`finishRace` 'too-soon' fell into `RETRYABLE`: a generic error with TRY AGAIN~~ **Fixed in v0.2.1:** the race is kept and saved automatically when the gap passes, with a countdown | `src/account.js` `flushPending` | — |
| D4 | ~~Magic `raceTime >= 90` for the PB rank display~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** TIME RANK shows when `finishRace` says the time became the new best (`newTime`) | `src/account.js` `recordRace` | — |
| D5 | ~~Stale data-model comment in `firebase.js`~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** | `src/online/firebase.js` | — |
| D6 | ~~Stale `WEAPON` comment~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** (seekers: Space / MMB; F is the spin) | `src/ai.js` | — |
| D7 | ~~`DEPLOY.md` describes Firebase Hosting~~ **Fixed 2026-11-02:** describes the PC set-up and `npm run deploy` | `DEPLOY.md` | — |
| D8 | ~~`milestones/README.md` status column stale~~ **Fixed 2026-11-03:** M6 and M7 in progress, M8 done | `neon_c/milestones/README.md` | — |
| D9 | ~~`window.NEON` in production~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** defined only in dev builds (`import.meta.env.DEV`) | `src/main.js` | — |
| D10 | ~~888 KB main chunk, no code splitting~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** three.js is its own chunk, cached across releases: an update downloads ~80 KB gzipped of game code instead of ~250 KB | `vite.config.js` | — |
| D11 | ~~FREE ROAM shows every remote player as a ROADSTER~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** `hi.car` / `join.car` (owned cars only, checked against the profile), models load on first sight | `server/live.js`, `src/main.js` `takeAvatar` | — |
| D12 | ~~Abandoned `raceTickets` docs are never cleaned up~~ **Fixed in v0.2.4 (2026-11-03, code; goes live with the release):** the daily `cleanRaceTickets` function deletes tickets older than 45 min | `functions/tickets.js` | — |
| D13 | ~~Browser tests live outside the repo~~ **Fixed 2026-11-02:** `game/tests/browser/` + `npm run test:browser` (starts its own dev server), run in CI | `game/tests/browser/` | — |
| D15 | **Fixed in v0.2.1:** a finished race was refused (`no-ticket`) when RACE AGAIN was pressed while `finishRace` was still on its way: the new `startRace` replaced the player's only ticket. Seen in the production function logs (2026-11-01 16:59:34 UTC). The client now waits for the save before asking for the next ticket | `src/account.js` `startRace` | Server-side alternative: accept the previous ticket too |
| D14 | ~~`RACE_GAP_S` (120 s) longer than a fast race~~ **Fixed in v0.2.1:** `RACE_GAP_S` = `MIN_RACE_S` = 60 s, so a real back-to-back race never waits (trade-off: R8 cheat rate doubles) | `functions/rules.js` | — |

## 13.3 Status 2026-11-03 (v0.2.4)
Fixed in code (live once v0.2.4 is deployed): R5, R6, R7, R8, R12, D2, D4, D5, D6, D8, D9, D10, D11, D12 (earlier: R2,
R3, R4, R10, D3, D7, D13, D14, D15). Still open:
- **R9** App Check enforcement: date-gated, not before 2026-11-05.
- **D1** `main.js` split: planned as its own step after v0.2.4 (a pure refactor, kept apart from the security fixes).
- **R1** (single home PC), **R11** (Firestore backups), **R13** (`neon_c/` not in git): need the owner's decisions or
  accounts (hosting, a backup schedule in Google Cloud, a backup repository).
