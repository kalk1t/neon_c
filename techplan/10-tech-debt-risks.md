# 13. Technical debt and risks

[← Index](TECHNICAL_DOCUMENTATION.md)

Severity: **H** high, **M** medium, **L** low. The "known issue" column refers to `docs/technical/11-known-issues-roadmap.md`.

## 13.1 Risks

| # | Risk | Severity | Evidence | Known issue | Mitigation idea |
|---|---|---|---|---|---|
| R1 | **Single point of failure:** one home PC serves the site and FREE ROAM; power, network, Windows Update reboot or sleep take everything down | H | `server/*`, `start-live.cmd` | | **Postponed by the owner (2026-11-03)** until the Windows app (win_app) shows what servers both games need; then: website to Firebase Hosting (free), relay to a small VPS if FREE ROAM grows. Player data is in Firebase, so an outage loses no data |
| R2 | ~~**Restart deploys the working tree**~~ **Fixed 2026-11-02:** production serves per-commit release worktrees (`npm run release`, `server/release.mjs`, launcher `server/start-live.cmd`); `serve` no longer builds | ~~H~~ | `package.json` `serve`, `server/release.mjs` | T2 (fixed) | Done; switched over 2026-11-02 (release `1843f2c` live) |
| R3 | ~~`npm run build` in `game/` changes production at once~~ **Fixed 2026-11-02** (as R2) | ~~M~~ | release folders under `neon_c/live/releases` | T2 (fixed) | Done |
| R4 | ~~`npm run deploy` bypasses the release order and deploys unused Hosting~~ **Fixed 2026-11-02:** `npm run deploy -- <functions\|client\|rules>` (`server/deploy.mjs`) runs the named parts in the order functions → client → rules, each behind its tests, refuses uncommitted functions/rules, and prints help with no parts; `hosting` removed from `firebase.json` | ~~M~~ | `server/deploy.mjs`, `firebase.json` | | Done |
| R5 | ~~`vite preview` is not designed as a production server~~ **Fixed in v0.2.4 (2026-11-03; live):** `npm run serve` = `server/serve.mjs`, a small Node server (static `dist/` with cache headers, `/__` proxy, host check), tested by `tests/serve.test.mjs` | ~~M~~ | `server/serve.mjs` | | Done |
| R6 | ~~Relay shares the web process~~ **Fixed in v0.2.4 (2026-11-03; live):** the relay is its own process (`server/relay.mjs`), started and restarted by `serve.mjs`, 384 MB memory cap; the site keeps serving while it restarts | ~~M~~ | `server/serve.mjs`, `server/relay.mjs` | | Done |
| R7 | ~~Kills and killer status are client-saved (forgeable)~~ **Fixed in v0.2.4 (2026-11-03; live):** the relay saves them through `relayKills` (shared secret `RELAY_SECRET`); the rules refuse browser writes. Remaining, **accepted by the owner 2026-11-03**: two accounts can kill each other as often as they like; kills earn no reward (S2) | ~~M~~ L | `functions/index.js` `relayKills`, `server/live.js`, `firestore.rules` | S8 (fixed) | Needs `RELAY_SECRET` set before the release (see 10-operations) |
| R8 | Race results client-claimed. **Fixed in v0.2.4 (2026-11-03; live):** per-road fastest times (`ROAD_MIN_RACE_S` / `ROAD_MIN_LAP_S`), the best lap must fit the race, coins need the road's minimum real time, **2,000 coins a day**, cut claims logged. Remaining, **accepted by the owner 2026-11-03**: up to the daily cap with plausible times, which real play can also earn | ~~M~~ L | `functions/index.js` `finishRace`, `functions/rules.js` | S1 | Replay-checking would close it fully (big job); App Check enforcement (R9) blocks scripts that don't run the game |
| R9 | No App Check. **In progress (2026-11-02):** code ready (dev debug token, `appCheckToken()`, relay forwards `hi.ac` as `X-Firebase-AppCheck` so Firestore enforcement won't break FREE ROAM); **tokens live since v0.2.2** (2026-11-02, exchange verified 200, TTL 1 h); next: watch metrics, enforce no earlier than 2026-11-05 | M | `src/online/firebase.js`, `server/live.js` identify | yes (open) | Rollout steps in `docs/technical/10-operations.md#app-check` |
| R10 | ~~No CI; tests only run by hand~~ **Fixed 2026-11-02:** GitHub Actions (`.github/workflows/ci.yml`) runs build, live, rules, functions and browser tests on every push to `main` | ~~M~~ | `.github/workflows/ci.yml` | | Done |
| R11 | ~~No Firestore backups~~ **Fixed 2026-11-03:** Firestore scheduled backups on `(default)`: daily (kept 7 days) and weekly on Sunday (kept 14 weeks). Restore = `firebase firestore:databases:restore` into a new database (see `docs/technical/10-operations.md#backups`) | ~~M~~ | `firebase firestore:backups:schedules:list` | | Done |
| R12 | ~~`server.log` grows forever and holds chat text~~ **Fixed in v0.2.4 (2026-11-03; live):** the launcher rotates it at 5 MB (3 old files kept); chat text isn't logged | ~~L~~ | `server/launcher.mjs`, `server/live.js` | | Install the new launcher once: `install-autostart.ps1` (admin) |
| R13 | ~~`neon_c/` not in git~~ **Fixed 2026-11-03:** github.com/kalk1t/neon_c (private): Assets, techplan, milestones, archive, meshy scripts, win_app; left out (root `.gitignore`): `game/` (own repo), `live/`, raw Meshy downloads, videos | ~~L~~ | `neon_c/.gitignore` | | Claude commits and pushes it after changes (win_app committed as-is, never edited) |

## 13.2 Code-level debt

| # | Item | Evidence | Effect |
|---|---|---|---|
| D1 | ~~`main.js` is 1,850 lines mixing state machine, race, roam, menus and settings~~ **Fixed in v0.2.6 (2026-11-03):** split into `render.js`, `world.js`, `race.js`, `aim.js`, `camera.js`, `roam.js`, `clock.js`; `main.js` keeps start-up, the state machine, menus, settings and the loop (~700 lines). Pure move (no behaviour change), checked by the browser tests | `src/*.js` | `race.js` unit tests since v0.2.7 (`tests/race.test.mjs`) |
| D2 | ~~Constants duplicated~~ **Fixed in v0.2.4 (2026-11-03; live):** one definition where code can share it (`DECK_Y` in `course.js`; `NICK_RE`, `CHAT_MAX`, `CAR_IDS` in `src/online/protocol.js`); the copies that must stay separate (functions, rules, relay limits, road minimum times) are compared by `tests/constants.test.mjs`, so drift fails the tests | `tests/constants.test.mjs` | — |
| D3 | ~~`finishRace` 'too-soon' fell into `RETRYABLE`: a generic error with TRY AGAIN~~ **Fixed in v0.2.1:** the race is kept and saved automatically when the gap passes, with a countdown | `src/account.js` `flushPending` | — |
| D4 | ~~Magic `raceTime >= 90` for the PB rank display~~ **Fixed in v0.2.4 (2026-11-03; live):** TIME RANK shows when `finishRace` says the time became the new best (`newTime`) | `src/account.js` `recordRace` | — |
| D5 | ~~Stale data-model comment in `firebase.js`~~ **Fixed in v0.2.4 (2026-11-03; live):** | `src/online/firebase.js` | — |
| D6 | ~~Stale `WEAPON` comment~~ **Fixed in v0.2.4 (2026-11-03; live):** (seekers: Space / MMB; F is the spin) | `src/ai.js` | — |
| D7 | ~~`DEPLOY.md` describes Firebase Hosting~~ **Fixed 2026-11-02:** describes the PC set-up and `npm run deploy` | `DEPLOY.md` | — |
| D8 | ~~`milestones/README.md` status column stale~~ **Fixed 2026-11-03:** M6 and M7 in progress, M8 done | `neon_c/milestones/README.md` | — |
| D9 | ~~`window.NEON` in production~~ **Fixed in v0.2.4 (2026-11-03; live):** defined only in dev builds (`import.meta.env.DEV`) | `src/main.js` | — |
| D10 | ~~888 KB main chunk, no code splitting~~ **Fixed in v0.2.4 (2026-11-03; live):** three.js is its own chunk, cached across releases: an update downloads ~80 KB gzipped of game code instead of ~250 KB | `vite.config.js` | — |
| D11 | ~~FREE ROAM shows every remote player as a ROADSTER~~ **Fixed in v0.2.4 (2026-11-03; live):** `hi.car` / `join.car` (owned cars only, checked against the profile), models load on first sight | `server/live.js`, `src/roam.js` `takeAvatar` | — |
| D12 | ~~Abandoned `raceTickets` docs are never cleaned up~~ **Fixed in v0.2.4 (2026-11-03; live):** the daily `cleanRaceTickets` function deletes tickets older than 45 min | `functions/tickets.js` | — |
| D13 | ~~Browser tests live outside the repo~~ **Fixed 2026-11-02:** `game/tests/browser/` + `npm run test:browser` (starts its own dev server), run in CI | `game/tests/browser/` | — |
| D15 | **Fixed in v0.2.1:** a finished race was refused (`no-ticket`) when RACE AGAIN was pressed while `finishRace` was still on its way: the new `startRace` replaced the player's only ticket. Seen in the production function logs (2026-11-01 16:59:34 UTC). The client now waits for the save before asking for the next ticket | `src/account.js` `startRace` | Server-side alternative: accept the previous ticket too |
| D14 | ~~`RACE_GAP_S` (120 s) longer than a fast race~~ **Fixed in v0.2.1:** `RACE_GAP_S` = `MIN_RACE_S` = 60 s, so a real back-to-back race never waits (trade-off: R8 cheat rate doubles) | `functions/rules.js` | — |

## 13.3 Status 2026-11-05 (v0.2.20)
v0.2.4 went live 2026-11-03; its build carried `RELAY_SECRET` inside `firebase-*.js` (a missing newline in `.env.local`
glued it onto `VITE_APPCHECK_SITE_KEY`). v0.2.5 rotated the secret, disabled the leaked version and added a build guard
(`vite.config.js` `secretGuard`). v0.2.6 split `main.js` (D1). v0.2.7 fixed the security findings C2 (plain-text HUD
lines) and C3 (CSP, report-only first), limited relay connections per address (07-security, DoS row) and added unit
tests for the race rules. v0.2.8 fixed S5 (pausing no longer protects a car in FREE ROAM), v0.2.9 S6 (chat moderation:
word filter, `/mute`, `/report`, bans), v0.2.10 S3 (the relay checks every hit; wrecks need a checked hit). v0.2.11
made the CSP ready to enforce (findings from the first real reports). v0.2.12 (not debt, a player-facing fix): one
centred mouse-aim ring with the camera following it; mouse reversing no longer spins the car; stuck mouse buttons fixed. v0.2.13: A / D slide sideways with the mouse;
a stopped car no longer shakes when turning (coasting overshot zero). v0.2.14 (gameplay): PULSE blaster rework (21 damage,
750 m/s × 1.5 s, ~22 bolts to overheat, lock-on 2 km), bolts in the car's paint colour from each model's front bumper,
42° slide lean; `LIVE_PROTOCOL` 9. v0.2.15 (gameplay): rivals 120 m/s + 2% a road, brake for bends, fire the
player's bolts 3x slower; the rubber band never slows a rival; player car 135 m/s; `PACE_MAX` 240 (functions deploy);
km/h / mph setting; `LIVE_PROTOCOL` 10. v0.2.16 (gameplay): rivals also fire seekers, spin-dodge and set off EMPs
with the player's values; target cone 35°. v0.2.17 (gameplay): per-car stats (`CAR_STATS`); `LIVE_PROTOCOL` 11; CI
killer suite fixed (test helper). Accepted: `PACE_MAX` 240 stays below the VIPER's theoretical 247 m/s (real times are
far slower; the owner chose not to touch the leaderboard). v0.2.18 (visual): thruster flames and damage smoke from each
car's marked nozzles; no new debt. v0.2.19 (visual + gameplay): each car's own 3D bolt, muzzle flash and impact from
the owner's art, fired from the marked shooting spots; bolts 300 m/s × 3.5 s (`LIVE_PROTOCOL` 12, the relay's hit check
too); fixed painted cars counting as ROADSTERs (`car.modelId`). No new debt; `WEAPON.BOLT_LEN` / `BOLT_WID` are now
unused by bolts (kept, still in the tests). v0.2.20 (balance): health 200 / 150 / 250 (rivals 200), rival seekers
slower (1.5 s lock, 2 charges, 9 s recharge, 6 s apart), 2 s protection after a wreck in races for every car (no hits, no attacks); exact positions (fractional, held at a
missed gate, wrecked cars at their respawn point, every frame); `LIVE_PROTOCOL` 13. v0.2.21: the EMP removed
(player, rivals, relay), no RESTART in FREE ROAM's pause menu; `LIVE_PROTOCOL` 14. v0.2.22: loops by hand (free mouse pitch,
camera turns over), backflip (2), air brake (Space), seeker on 1; `LIVE_PROTOCOL` 15;
the Killer R block is FREE ROAM only (it wrongly applied in races). No
new debt.

Fixed: R2-R8, R10-R13, D1-D15. Still open:
- **R9** App Check enforcement: date-gated, not before 2026-11-05.
- **C3 step 2:** switch the CSP from report-only to enforced (`CSP_ENFORCE=1`) once `[csp]` lines stay away. The first
  real reports (2026-11-03) found an inline `onclick` and the Cloudflare Web Analytics beacon: both fixed in v0.2.11;
  watch the log again after it goes live.
- **R1** single home PC: deferred by the owner until the Windows app's server needs are known.
- **S4** server-authoritative health (hp and dodges decided by the server; no EMP since v0.2.21): deferred by the owner, to be
  planned with the Windows app's game server (same decision as R1).

**Accepted by the owner (2026-11-03), not debt:**
- Two accounts killing each other as often as they like (R7 remainder): kills earn no reward, so there is nothing to farm.
- Up to 2,000 coins a day with plausible times (R8 remainder): real play can earn that much too, so the cap is the
  limit by design.
