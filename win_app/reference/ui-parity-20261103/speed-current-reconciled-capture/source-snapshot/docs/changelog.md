# Changelog

[Documentation home](README.md)

Newest first. Each version is a git tag (`git tag v0.1`). Player-facing highlights are in the manual's
[What's new](manual/10-whats-new.md).

## v0.2.16 (2026-11-03): rivals get seekers, the spin dodge and the EMP
- Rivals fight with the player's whole arsenal, with the player's values (`WEAPON`); when they use it is `RIVAL_AIM`
  in `src/race.js`:
  - **target:** the nearest car within **35°** of the nose (was 17°), out to 2 km; bolts only within their reach;
  - **lock-on:** the player's 0.5 s on the same target;
  - **seekers:** 48 damage, homing, 3 charges (one back every 4.5 s), on a full lock within 600 m, at most one every
    1.5 s; *SEEKER INCOMING · NAME* when one is fired at the player;
  - **spin dodge:** a 50% chance to roll away from a seeker homing on it within 120 m, 6 s cooldown;
  - **EMP:** when 2 or more cars are within 95 m: the player's damage, push and shot wipe, rivals hit hold fire 3 s,
    11 s cooldown; *EMP · NAME* on the player.
- Unchanged: no fire in the first 6 s, 100 health, the player's EMP holds rival fire 3 s.
- Tests: `race.test.mjs` 24 (+4: the 35° cone, seekers after a full lock / range / gap / charges / recharge, the spin
  dodge, the EMP).

## v0.2.15 (2026-11-03): faster, fiercer rivals; a faster car; km/h or mph
- **Rival speed:** 120 m/s on road 1, +2% a road (`RIVAL_SPEED` in `src/roads.js`, `rivalTop`); before, the player's
  95 m/s × 1.000 → 1.072. Nitro × 1.45 on top.
- **Rubber band:** a rival ahead of the player is never slowed down (it lost up to 5%); one behind still gets up to +7%.
- **Rivals drive the bends:** they brake for corners (`CORNER` in `src/ai.js`: each sample's corner speed, braking
  planned at 45 m/s²), aim at the gate for the last ~1.5 s, and correct their climb for their flight path. Their gates
  count within 45 m (`GATE_R_RIVAL`; the player's 15 m). Without this they flew off the road from 120 m/s; even at the
  old speeds they could get stuck on road 9. Rivals accelerate in proportion to their top speed.
- **Rivals fire the player's PULSE bolts:** 21 damage, 750 m/s for 1.5 s (~1,125 m), from their bumper, one every
  0.63 s (3x slower than the player, `RIVAL_AIM.SLOWER`), leading the target with an aim error of up to 0.03 rad
  (x the road's `fire`, 0.6 on road 9). Before: one homing 240 m/s shot every 4-8 s, 30 damage on the player and 51
  on a rival (`DAMAGE`, `AI_FIRE_INTERVAL` and `RIVAL_POWER` are gone).
- **The player's car:** top speed **135 m/s** (486 km/h, was 95), nitro 196 m/s (706 km/h, was 138); both
  accelerations scale with it (`K = TOP / 95`).
- **Leaderboard limits** (`functions/rules.js`): `PACE_MAX` 240 m/s (was 170), so `ROAD_MIN_RACE_S` 43-75 s and
  `ROAD_MIN_LAP_S` 21-37 s, `MIN_LAP_S` 21. Needs the functions deployed with the client. `tests/constants.test.mjs`
  now evaluates `CAR` (it read the literal `138` and would have missed the faster nitro).
- **Settings: SPEED KM/H / MPH** for the speed panel and the cockpit (`neon.units`).
- **Test backend (`?mock`, dev only):** every road is open.
- `LIVE_PROTOCOL` 10: FREE ROAM cars are faster, so old tabs reload.
- Tests: `race.test.mjs` 20 (+3: rival speed table, rival bolts, their rate / range / EMP pause; the rubber band test
  now checks that a rival ahead keeps its speed), `rivals` browser test updated, `functions.test.mjs` R8 on the new
  road 9 limits.

## v0.2.14 (2026-11-03): PULSE blaster rework, bolts in your paint colour
- **Bolts hit harder and fly further:** 21 damage (was 6; 5 bolts wreck a car), one every 0.21 s (was 0.09 s),
  750 m/s (was 430) for 1.5 s (was 1.3): ~1,125 m standing still, more at speed. Heat +0.08 a bolt (was 0.06):
  ~22 bolts (4.6 s) of non-stop fire before it overheats (was ~17 / 1.5 s).
- **Lock-on reaches 2 km** (`LOCK_RANGE` 2000, kept to `LOCK_KEEP` 2100; were 400 / 460).
- **Bolts in the car's paint colour** (and the muzzle flash): the paint's accent, lightened, ×3 bright. Other players'
  and rivals' bolts in theirs. Bigger streaks: 45 m × 7 m (were 24 × 5), drawn growing out of the muzzle instead of
  centred on the shot (half of it used to stick out behind the car).
- **Shots leave from each car's front bumper** (measured on the models; `CAR_MODELS[id].muzzle`), not from 1.3 m
  either side of a point under the nose, which was outside or above the bodies.
- The slide leans **42°** at full slide (`STRAFE.LEAN`, was 17°).
- `LIVE_PROTOCOL` 9: the victim's game applies bolt damage, so old tabs are told to reload. The relay's hit checks
  (`server/hits.js`) use 750 m/s × 1.5 s.
- Tests: `tests/controls.test.mjs` 16 (+3: ~22 bolts to overheat, bolt values, muzzle per car), `tests/roam.test.mjs`
  +1 (the 5th bolt wrecks); the new ones fail on v0.2.13. `tests/hits.test.mjs` updated for the new reach.

## v0.2.13 (2026-11-03): sideways slide; a stopped car turns smoothly
- **A / D slide the car sideways** while the mouse is captured (the mouse turns it): up to **25 m/s** (~90 km/h),
  at any speed, the nose keeps pointing where it was. The car leans into the slide and its thruster pods tilt toward
  it. The arrows still turn; without the mouse (keyboard only) A / D turn as before. Rivals don't slide.
- **A stopped car no longer shakes when turning.** Coasting overshot zero, so the speed flipped sign every frame and
  with it the steering (reversing steers the other way): the pods showed the turn, the car barely moved and the camera
  vibrated. Coasting now stops at exactly zero.
- The speed readout (panel and cockpit) counts a slide too.
- `tests/controls.test.mjs` now 13 (+6: stop at zero, smooth turn from a stop, slides standing and at speed, slide
  decay and reset, A / D vs arrows); all 6 fail on v0.2.12.

## v0.2.12 (2026-11-03): one aim ring, mouse controls fixed
- **Mouse-aim camera:** with the mouse captured, the view follows the aim and the **aim ring stays in the middle of
  the screen**; the car turns and climbs after it, and bolts go where the ring is. Where the nose points is a small dot
  now (the big heading crosshair only shows without the mouse: keys, touch, gamepad). Before, the camera followed the
  car and the aim ring wandered, at hard turns off the screen.
- **Flying backwards with the mouse no longer spins the car.** Reversing used car-style steering, so the nose turned
  *away* from the aim and the car kept turning at full lock. The nose now turns to the aim in both directions
  (keyboard reversing still steers like a car).
- **Right-click free look can't get stuck any more.** A button release the browser never delivered left free look (or
  the blaster) on, so the mouse swung the camera instead of steering. The real button state now comes with every mouse
  movement.
- The nose settles at the aim's height instead of overshooting it (`climb = aimPitch / 0.62`, was × 1.25).
- `tests/controls.test.mjs` (7, part of `test:game` and `test:live`); it fails on v0.2.11.

## v0.2.11 (2026-11-03): ready to enforce the CSP
The first `[csp]` reports from real visitors (report-only, C3) showed two things an enforced policy would break:
- the error screen's **RELOAD** button used an inline `onclick` (now a listener in `main.js`); a test now fails on any
  inline event handler in the pages;
- **Cloudflare Web Analytics** (the beacon Cloudflare adds to every page): `static.cloudflareinsights.com` is now
  allowed for scripts and `cloudflareinsights.com` for connections.

## v0.2.10 (2026-11-03): the relay checks FREE ROAM hits
- **Hit checks** (known issue S3, `server/hits.js`): the shooter's game still decides its hits (so they land where the
  car is drawn), but the relay passes one on only if it could have happened:
  - it comes from a real shot of that kind, and each shot hits once (one seeker used to allow 70 hits);
  - the shot left from where the shooter really was (a fast car gets more room), which also stops EMPs set off on
    someone else's car;
  - the victim was within reach for the time the shot flew (bolt 430 m/s for 1.3 s, seeker up to 330 m/s for 3.6 s,
    plus the shooter's speed);
  - and roughly in the line of fire (within ~53°, any direction within 80 m; a seeker homing on that victim from any
    side).
  Where the victim was in the last 1.5 s counts, so lag never costs a fair hit.
- **Wrecks count only after a hit** (helps S2): a reported wreck credits the killer only if one of their hits or EMPs
  reached the victim in the last 30 s.
- Ignored hits, shots and wrecks are logged as `[s3] Name: N implausible ignored (reasons) uid …`, once a minute per
  player at most.
- Tests: `tests/hits.test.mjs` (11), 3 relay tests, a constants check (the relay's weapon values = the game's); the
  browser tests now land a real hit before a wreck (`shootAt`). `test:live` runs 85.
- No protocol change: tabs on v0.2.9 keep working.

## v0.2.9 (2026-11-03): FREE ROAM chat moderation
- **Word filter** (known issue S6): the relay replaces blocked words with stars in chat lines and nicknames, for
  everyone. Look-alikes (`sh1t`, `$hit`) and stretched letters are caught; words of 4+ letters also inside longer
  words, shorter ones only on their own (so *class* and *grass* stay). A line that is only blocked words is dropped.
  Starter list: `server/blocked-words.txt`; the owner adds words in `neon_c/live/blocked-words.txt`.
- **/report NAME**: logs the player's last 8 chat lines (as written) and account id in `server.log` as `[report]`.
  One report every 30 s.
- **/mute NAME, /unmute NAME, /muted**: hide a player's chat on this device (kept in the browser, never sent).
- **Bans**: nicknames or account ids in `neon_c/live/bans.txt` are refused FREE ROAM (*YOU ARE BANNED FROM FREE
  ROAM*; races still work). The file is re-read within 10 s; a player banned while driving is removed at once.
- **/help** lists the commands; an unknown command points to it.
- `LIVE_PROTOCOL` 8: open tabs reload once.
- Tests: `tests/moderation.test.mjs` (6: filter, bans, reload, mutes) and 4 relay tests; `test:live` runs 71.

## v0.2.8 (2026-11-03): no hiding in the pause menu in FREE ROAM
- **Pausing in FREE ROAM no longer protects your car** (known issue S5). Other players could still see the paused car
  but every hit on it was ignored, so a lost fight could be escaped with Esc. FREE ROAM keeps going during a pause:
  hits and EMPs count and the car can be wrecked; the pause screen says *FREE ROAM KEEPS GOING · YOUR CAR CAN STILL BE
  HIT*. Spawn protection and the spin dodge work as before. Races are unchanged.
- `tests/roam.test.mjs` (4, part of `test:game` and `test:live`): hits while driving, while paused, not in the menu or
  shop, spawn protection and dodge. It fails on v0.2.7.

## v0.2.7 (2026-11-03): security headers, relay connection limits, race rule tests
### Security
- **HUD lines are plain text** (C2): `hud.sub()` used `innerHTML`, and FREE ROAM lines carry other players'
  nicknames. Nicknames can't hold HTML (`NICK_RE`), but the HUD no longer relies on that.
- **Content-Security-Policy** (C3) from `server/serve.mjs` on every page, **report-only** for now: violations are
  posted to `/csp-report` and logged as `[csp]` in `server.log`; `CSP_ENFORCE=1` in `.env.local` turns it on. A real
  build loaded through it reported nothing (reCAPTCHA, Analytics, Firebase and Google Fonts all load). Also
  `Referrer-Policy: strict-origin-when-cross-origin`.
- **Relay connection limits:** at most 6 open and 20 new connections a minute per player address, and 64 still
  signing in overall; over a limit the connection is refused with HTTP 429 before the WebSocket opens.
### Tests
- `tests/race.test.mjs` (18, also `npm run test:game`): the race rules on the real map in Node: gates, laps, best
  laps, finishing, SIGNAL LOST, damage, dodging, takedowns and loot, ranking, rubber band, rival fire, results
  banking. `tests/game-env.mjs` loads the game's modules through Vite with a stub page. `test:live` now runs 57.
- Relay tests for the connection limits (4), server tests for the CSP (3).
- `volAt` / `panAt` return 0 before the camera exists (only matters for the tests).

## v0.2.6 (2026-11-03): main.js split into modules (tech debt D1)
### Code
- `src/main.js` (1,850 lines) is split by job; `main.js` keeps start-up, the state machine, menus, settings and the
  main loop (~700 lines). New modules: `render.js` (renderer, post-processing, graphics quality, atmosphere),
  `world.js` (map, roads, coins; `map` / `road` / `track` / `coins`… are live ES-module bindings), `race.js` (race
  rules, damage, ranking, results), `aim.js` (aim, lock-on, the player's weapons), `camera.js` (cameras),
  `roam.js` (FREE ROAM) and `clock.js` (the game clock). Each gets the shared objects (`g`, `player`, `hud`, …) from
  `main.js` once through `init…(deps)`.
- The code was moved, not changed: the browser tests (killer, rivals, save, void) pass on it unchanged.

## v0.2.5 (2026-11-03): relay secret rotated, build guard
### Security
- **The v0.2.4 build published `RELAY_SECRET`.** `.env.local` had no newline at its end, so the added
  `RELAY_SECRET=...` line was glued onto `VITE_APPCHECK_SITE_KEY`, and that value is built into the browser code
  (`firebase-*.js`). It also broke the App Check site key in v0.2.4 (App Check is not enforced yet, so nothing failed
  for players). The secret was replaced (Secret Manager version 2), the leaked version disabled, and the functions
  redeployed with the new one. Someone holding the old value could only have changed kill counts and KILLER STATUS.
- **Build guard** (`vite.config.js` `secretGuard`): a build stops if a `VITE_` value contains another `NAME=` or
  whitespace (two lines run together), or if any built file contains `RELAY_SECRET` or a server secret's value.

## v0.2.4 (2026-11-03): server-saved kills, tighter race checks, shop cars in FREE ROAM, a production server
### Security (tech debt R7, R8)
- **Kills and KILLER STATUS are saved by the live relay** (R7, known issue S8). The relay sends what changed, 3 s after
  a kill or `/pkclear`, to the new `relayKills` function, signed with a shared secret (`RELAY_SECRET`: Secret Manager
  for the function, `.env.local` for the relay). Failed saves are kept and retried; a player who rejoins before a save
  lands carries on from the relay's count. `firestore.rules` no longer lets the browser write `kills`,
  `lastKillsAt`, `isKiller` or `killerKillCount`. Live games no longer save them (`awardKills` / `setKiller` are gone
  from the Firebase backend; the `?mock` backend keeps them for tests). `/pkclear` while disconnected now asks to
  reconnect instead of clearing locally.
- **Race results are checked harder** (R8): each road has a fastest possible 2-lap time and lap (`ROAD_MIN_RACE_S`,
  `ROAD_MIN_LAP_S` in `functions/rules.js`: its gate-to-gate distance at 170 m/s, more than nitro plus spin surges can
  average); coins need that much real time; the best lap must be at most half the 2-lap time; at most **2,000 coins a
  day** (`DAILY_COIN_CAP`, `coinsDay` on the profile); cut claims are logged (`finishRace: claim cut`). `finishRace`
  also returns `capped` and `newTime`.
- `LIVE_PROTOCOL` 7.

### Added
- **Shop cars in FREE ROAM** (known issue G8): the game sends its equipped car (`hi.car`); the relay shows a car the
  profile owns (`cars`), otherwise the ROADSTER (`welcome.car`, `join.car`).
- **Daily clean-up of abandoned race tickets** (D12): the `cleanRaceTickets` scheduled function (04:00 UTC) deletes
  tickets older than 45 min.

### Changed
- **Production server:** `npm run serve` is now `server/serve.mjs` instead of `vite preview` (R5). The relay runs as
  its own process (`server/relay.mjs`, R6), restarted on its own if it crashes; the site keeps serving meanwhile.
- **The launcher rotates `server.log`** at 5 MB, keeping 3 old logs (R12), and chat text is no longer logged. Run
  `server/install-autostart.ps1` once as administrator to install the new launcher.
- **TIME RANK after a new best** shows when the server accepted the time (`newTime`), replacing an unexplained
  `raceTime >= 90` check (D4).
- **three.js is its own file** (`assets/three-*.js`, D10), so it stays cached across releases: an update downloads
  ~80 KB (gzipped) of game code instead of ~250 KB.
- **`window.NEON` is dev-only** (D9): production builds leave the debug handle out.
- Shared values have one home where possible (D2): `DECK_Y` in `course.js`; `NICK_RE`, `CHAT_MAX`, `CAR_IDS` in
  `src/online/protocol.js`. `tests/constants.test.mjs` checks the copies that must stay separate (functions, rules,
  relay limits, road minimum times).
- Stale comments fixed: the data model in `firebase.js` (D5), the `WEAPON` keys in `ai.js` (D6).

### Development
- `npm run test:live` now also runs `tests/constants.test.mjs`, `tests/serve.test.mjs` and `tests/launcher.test.mjs`
  (32 tests). `npm run test:functions` writes the emulator's test secret first (`tests/functions-secret.mjs`;
  `functions/.secret.local` is git-ignored and never deployed).

## v0.2.3 (2026-11-02): tougher rivals, TRACK FLYOVER removed, tests in the repository, CI
### Changed
- **Rivals are as fast as you.** Their top speed no longer drops with skill (it was 92–100% of yours on road 1); every
  rival now has the player's top speed × the road's `ai` factor (1.000 → 1.072), still with the gentle rubber band.
  Skill only decides how eagerly they boost.
- **Rivals hit harder and shoot more often: 150% on every road** (`RIVAL_POWER` = 1.5 in `src/ai.js`). A shot now
  does 30 damage to you (was 20) and 51 to another rival (was 34); time between shots is 4–8 s (was 6–12 s), still
  × each road's `fire` factor, so later roads stay harder.

### Removed
- **TRACK FLYOVER** is gone from the main menu (the button and the `tour` state). `cinematicCamera` stays for the
  start-up shader warm-up. The docs screenshot script no longer takes `track-flyover.jpg` (the v0.1 picture stays in
  `docs/media/v0.1/`).

### Development
- **Browser tests in the repo:** `tests/browser/` (`killer`, `void`, `save`, plus `killershot.mjs` for screenshots),
  run by `npm run test:browser`, which starts its own dev server with test logins. A page error now fails a test.
  They used to be loose scripts outside git that needed a hand-started server on port 5199.
- **CI:** GitHub Actions (`.github/workflows/ci.yml`) builds the game and runs the live, rules, functions and browser
  tests on every push to `main`, on pull requests and on demand. It never deploys. Pinned to `ubuntu-24.04`:
  Playwright 1.56 has no Chromium for `ubuntu-latest` (now Ubuntu 26.04).

## v0.2.2 (2026-11-02): releases, safe deploys, App Check on (not enforced)
App Check sends tokens from this version (key `VITE_APPCHECK_SITE_KEY` set); enforcement comes after a few days of
metrics (see [Operations → App Check](technical/10-operations.md#app-check)). No gameplay change.

- **Production serves releases, not the working folder.** `npm run release [-- <tag|commit>]` checks a commit out into
  `neon_c/live/releases/<commit>` (its own `node_modules`, copied `.env.local`, `dist/`), builds it, runs
  `test:live` against its relay, then switches `neon_c/live/current.txt` and restarts the server on it, and checks
  the site serves it. No admin needed. `npm run release -- --rollback` switches back; the last 3 releases are kept.
- The scheduled task runs `neon_c/live/start.cmd` → `launcher.mjs` (from `server/`), which serves the current release, switches when `current.txt` changes
  and restarts it if it stops. A reboot or crash no longer rebuilds from uncommitted files, and `npm run build` in
  the game folder no longer changes the site. `npm run serve` no longer builds. The log moves to
  `neon_c/live/server.log`.
- **`npm run deploy -- <functions|client|rules>`** (`server/deploy.mjs`) replaces `vite build && firebase deploy`, which
  would have deployed hosting, rules and functions at once. Parts always run in the order functions → client → rules,
  each behind its tests (`test:functions`, `npm run release` with `test:live`, `test:rules`); functions and rules
  must be committed; with no parts it only prints help; `--dry-run` deploys nothing. The unused `hosting` section is
  gone from `firebase.json` (nothing was ever deployed to `roadsters-race.web.app`). `DEPLOY.md` describes the PC set-up.
- App Check groundwork (no change until the reCAPTCHA key is set): dev builds use an App Check debug token; the game
  sends its App Check token with the FREE ROAM sign-in (`hi.ac`, optional, protocol unchanged) and the relay passes it
  on when it reads the profile, so enforcing App Check on Firestore won't lock players out of FREE ROAM. A refused or
  failed profile read is now a sign-in error (logged), not "no nickname". `MAX_HI` 4096 → 8192 bytes.
- `server/install-autostart.ps1` installs the new launcher (run once, as admin); `restart-server.ps1` now restarts
  without building.

## v0.2.1 (2026-11-01): VOID CROWN, race saving fixes
Cloud Functions deployed first (race gap 60 s), then the client and live relay (protocol 6) with one server restart; no rules change.

### Added
- **VOID CROWN, the second Killer rank** (design sheet `Assets/06_Effects/killeraura2.png`): at **10 Killer kills**
  (`VOID_KILLS` in `src/online/protocol.js`) the CRIMSON FLAME (from 3 kills) becomes the VOID CROWN: a crystal crown
  above the roof, crystal shards circling the car, and the underglow, flank flames, pod wreaths, ember trail and sparks
  in dark purple and neon blue. *VOID CROWN* flashes on screen; other players' kill feed says *NAME WEARS THE VOID
  CROWN*. `/pkclear` clears it like any Killer status. New textures `public/fx/voidaura_*.webp`, made by the new
  `tools/fx/voidaura.mjs`. Shards per tier: 7 / 7 / 5 / 3 (high, menu, remote, low); the crown shows on every tier.
- Live protocol 6: `join` carries `kc` (the Killer kill count), so everyone draws the right aura; `kill.kc` moves it
  up. The count was already saved (`killerKillCount`), so no rules change.
- Dev: `?mock&killer` makes the mock test profile a Killer on every load (3 kills); `?mock&killer=void` (or a number,
  e.g. `=9`) sets the count.

### Fixed
- **Races sometimes not saved.** The server keeps one race ticket per player; pressing RACE AGAIN while the last result
  was still being saved (a slow first call) replaced the ticket, and the finished race was refused. A new race now
  waits for that save before asking for its ticket. A failed ticket request is retried once.
- **"Races too close together"** no longer loses the race: it counts down and saves it when the gap has passed
  (before, the dedicated message never showed: a generic *Something went wrong (resource-exhausted)* with TRY AGAIN).
- **Gap between saved races: 1 minute** (was 2; `RACE_GAP_S` = `MIN_RACE_S` = 60), so real back-to-back races never
  wait. Cloud Functions redeployed.

## v0.2 (2026-11-01): shop, server coins, cockpit view, nine roads, KILLER STATUS
Launched on roadstersrace.com: Cloud Functions deployed first, then the new `firestore.rules`, then the client and the
live relay (protocol 5) with one server restart.

### Added
- **Shop** (main menu **SHOP**): two cars to buy with wallet coins, **VIPER** 2,500 and **TITAN** 7,500, next to the
  free **ROADSTER**. Looks only: same physics and weapons. Car stage with drag-to-orbit and zoom, confirm and success
  dialogs, **EQUIP NOW / LATER**, keyboard support, phone layout, a **NEW** tag on the newest car.
- **Two new car models** made with Meshy and `meshy/build_game_car.py`: `car02.glb` (VIPER) and `car03.glb` (TITAN),
  loaded only when first shown.
- **Cockpit view** (I, or VIEW on phones): a per-car illustrated dashboard over a raised first-person camera, with live
  speed, boost, PULSE heat, lap and time, and a yoke or grips that follow the steering. Remembered per device.
- **M8 server coins:** Cloud Functions `startRace`, `finishRace` and `buyCar` (`functions/`, us-central1). Races
  carry a server ticket; coins need a RACE of at least 60 s by the server's clock; purchases are one transaction with
  a purchase log.
- **Nine roads** on the one map (`src/roads.js`): SKYLINE LOOP plus VIOLET RUN, GLASS CANYON, HALO DRIFT, NEON SPIRAL,
  STORM LINE, RAZOR CROWN, VOID SPRINT and APEX OVERDRIVE, from 5.64 to 10.40 km a lap and 8 to 19 gates, each harder
  than the last (more and tighter corners, bigger climbs, slightly faster and quicker-shooting rivals). Generated with
  `tools/roadgen/` and checked by a physics-only autopilot lap on every road.
- **Road picker**: RACE and TIME ATTACK (and Enter on the menu) first open the road list: length, gates, difficulty
  bars, best lap, locks. **Unlocks**: win a road in 1st place (RACE) to open the next; *🔓 ROAD n UNLOCKED* and
  **NEXT ROAD ›** on the results screen. Progress is kept on the device and in the profile (`roadsWon`, server-written).
- **World ranking** tabs renamed **COIN · TIME · KILLS**; TIME has one ranking per road (road row 1-9 under the tabs).
  Profile fields `best2` (road 1, unchanged) and `best2r2` … `best2r9`; race tickets carry the road.
- **Menu showcase**: the car now hovers well out in front of one of the big LED screens on the towers (facing away from
  it, so the screen glows behind the car), and the camera sways in front of the car instead of circling it.
- **KILLER STATUS / CRIMSON FLAME**: 3 confirmed FREE ROAM player kills make a Killer (`isKiller`,
  `killerKillCount` on the profile; counted by the live relay). The CRIMSON FLAME aura (`src/killer.js`, textures from
  `Assets/06_Effects` in `public/fx`) burns on all three cars in FREE ROAM, races, the menu and the shop, and on other
  players' cars; graphics tiers keep it light (no embers or trail on LOW). R respawn is locked while a Killer.
  `/pkclear` in the FREE ROAM chat clears it. First chat command system (`cmd` message; unknown commands refused,
  never broadcast). Live protocol 5. New rules branch for the two fields; `?mock` test profiles start as Killers.
  Tests: `npm run test:live` (7, the relay), 5 new rules tests (40).
- **Version label** on the main menu, read from `package.json`.
- **Sculpted scenery** made with Meshy from `Assets/03_Scenery` (image-to-3D) and optimised in Blender
  (`meshy/map_props/optimize.py`): the **city tower** crowns about a third of the towers near the course, a new
  **control tower** at the start deck, and the **blue bonsai** replaces every tree (the magenta bonsai is retired).
- **Life in the sky** (Meshy models): courier drones and sky taxis looping near the course, advertising blimps high
  above it (each with a screen for rent on both flanks, showing the ad slides), floating sky islands with neon spires (solid), antenna masts on roofs and chrome koi sculptures on the
  tallest towers. All load after the start, so they never slow the loading screen.
- City and scenery are built once around all nine roads (`Corridor` in `track.js`); a road's rings, guidance line,
  signs and coins are swapped by `setRoad`. Towers near a low road under a high one keep stepping down until both are
  clear. The results card is more compact so the extra unlock line and button fit a 720p screen.
- Tests: `npm run test:functions` (18, new: road times and unlocks) in the emulators; `npm run test:rules` rewritten for the new rules (35, 40 with KILLER STATUS).

### Changed
- **Firestore rules:** the browser can no longer write coins, races, lap times or cars, only renames and FREE ROAM
  kills. `raceTickets` is server-only; `users/{uid}/purchases` is readable by its owner only.
- A race started signed out can't be saved later: the results screen offers **SIGN IN TO SAVE YOUR NEXT RACE**.
- New save errors: *THE SERVER DIDN'T SEE THIS RACE START*, *RACE TOOK TOO LONG*.
- Race coins use the shop's gold wheel-coin art; the HUD coin icon too.
- FREE ROAM: a wrecked player is down **3 s** (was 2.4 s) before respawning, with a *RESPAWN IN 3 · 2 · 1* countdown.
- **City colours** matched to the night sky: window light is mostly cyan, violet-blue and cool white (less orange),
  the glass is violet-navy and a little more reflective. Towers got rooftop plant rooms, helipads with neon rings,
  glass fins near the course and a lit top band on every tower type.
- **Car paint** has a clear coat: the cars catch the city's reflections instead of reading flat black.
- `?mock` test profiles get a 10,000-coin test wallet so both cars can be bought.
- Code: the map is now data (`src/maps.js`: laps, floor, sky, fog, lights) and is built by `buildWorld` in `main.js`.
  The game keeps one map, SKYLINE CITY, now with nine roads (above). The two extra maps that were started (NEON DOWNPOUR, MONSOON FALLS)
  are shelved and kept in `archive/maps-r1-r3/`.

### Known gaps
- FREE ROAM shows other players' cars as ROADSTERs (in their color).
- App Check isn't set up yet.

## v0.1 (2026-10-31): first documented version
The baseline: everything built up to this date. Earlier history lives in `milestones/` and the README; version
control starts here.

### The game at v0.1
- Three modes: **RACE** (2 laps, 10 rivals, coin race), **TIME ATTACK**, **FREE ROAM** (online, up to 24 players).
- Flying car with mouse aim, boost, three weapons (PULSE, SEEKER, EMP) and the SPIN dodge.
- Google sign-in, nicknames, wallet, lifetime coins and kills, three world rankings.
- Desktop (mouse + keyboard) and phones (touch and tilt).
- Self-hosted at roadstersrace.com (home PC, Cloudflare Tunnel), data in Firebase.

### Added in the last days before v0.1 (2026-10-30 to 31)
- **FREE ROAM kills** saved to accounts, and a **KILLS** ranking tab.
- **Chat** in FREE ROAM (Enter / T, or CHAT on phones): 120 characters, rate-limited.
- **FPS · ping · time online** line under the FREE ROAM status.
- **Spin dodge** (SPIN): barrel roll with a sideways hop and a speed surge.
- **System requirements** popup on the menu, with a check of your device.
- **Menu car showcase:** drag to orbit, scroll to zoom; the **CAR COLOR** swatches repaint it live.
- **Protocol check:** client and server versions must match; an outdated tab reloads (at most once a minute).

### Changed
- **Shooter-decided hits** in FREE ROAM: hits now land as they look on the shooter's screen.
- **Right mouse = free look**; boost is on Shift only.
- The main menu scrolls on short screens, with a tighter layout up to 820 px tall.

### Fixed
- Ranking didn't open on phones.
- Crosshair and markers were shifted left on small screens (HUD zoom).
- First-person camera showed the car's insides.
- The menu showcase camera ran every frame, fighting the race cameras.
- Phones showed the CHAT button in RACE and Time Attack too, on top of the lap panel (found while making the docs).
- The results screen couldn't scroll: on short windows (720 px) its top and the MAIN MENU button were cut off.

### Project
- Git repository started; baseline commit, then the documentation, tagged `v0.1`.
- Documentation: player manual, technical reference, feature inventory, screenshot tool (`npm run docs:shots`).
