# Offline targeting and explicit late-coin phase - 2026-11-03

Shared C now has local target-angle/lock/lead logic and an explicit weapon-phase
boundary in the local pack. Full native and WASM builds each pass **30/30** tests.
**New combat, live rivals and coins remain inactive in production.** No weapon
fire, projectile damage, reward, account, wallet or network behavior was activated.

## Source order and the resolved pack limitation

Current browser main.js orders the relevant work as follows:

player step/rules/FX -> sequential rival step and rival firing -> car contacts ->
ranking -> player lock/weapons/manual reset -> camera/FX and projectile update ->
coin update. Caller phase/state checks matter: player weapons run only in race
state while alive; the final GO tick still follows the countdown branch, and
finished-state coin eligibility differs from whether player weapons may run.

The previous combat-free pack helper combined the last coin pass with the earlier
rival/rank segment while excluded weapon operations were empty sinks. That public
boundary is now split into **rr_pack_rivals_after_player** and
**rr_pack_coins_after_weapons**. The latter reads the resulting current racer
eligibility/positions, not a stale pre-weapon view. It owns the saved frame dt and
entered branch, consumes the coin tail once, rejects duplicate consumption and
prevents starting another rival pass while a tail is pending. Paused has no tail.
Start/restart clears pending state. The old combat-free convenience wrapper composes
both phases and retains the exact earlier reference behavior.

Four probes compare actual source Coins.update behavior after an interposed
unchanged/wreck/move/finished state. They show no collection before the tail, no
pickup after becoming ineligible or moving away, and no duplicate update. The
weapon mutation is a controlled test input, not an implemented damage system.

The prior4,320-tick/nine-road pack suite passes unchanged: countdown2376, playing1899,
paused45, nine restarts,18 respawns,288 requested RNG pairs and58 coin collections.
Targeting itself consumes no random values. The demand-driven rival RNG adapter,
stable racer order, rank cadence and finish cached-sample handling remain intact.

## Source-grounded targeting slice

New src/core/rr_targeting.{h,c} ports actual aimAngle, updateLock and leadDir behavior:

- Current local-roster membership, alive/finished checks, distance>=8 and inclusive
  acquisition distance400/retention distance460. Visibility is not silently added
  as another eligibility rule; source membership handles inactive local rivals.
- Acquire with angle strictly<0.3; keep the existing target while angle<0.5. A
  centered angle0 remains a valid retained target, matching source strict checks.
- Acquisition ties retain the earlier roster entry; a valid retained target is not
  replaced merely because another target has become closer to the crosshair.
- Target changes reset charge, then advance it in the same update; no target sets
  charge0. Charging is dt/0.5 capped at1. Acquisition/full-charge event bits match
  the source sound cues without playing audio. Stable indices refer to the fixed
  local roster; no remote identity or protocol policy is assumed.
- Lead direction uses source distance/speed, target velocity times0.9, subtraction
  from muzzle origin and normalization. No quadratic intercept, new balance or
  projectile/homing mechanic is invented. Zero-length output matches source math;
  caller projectile speed must be finite and positive.

Aim direction/player phase are supplied by the host. This does not port mouse/
gamepad aim, car root/muzzle quaternion, weapon resource state, firing, projectile
motion, EMP effects, health/rewards or remote combat. Correct caller alive/state
gating is still required before this code is wired into the app.

## Fresh reference and verification scope

Browser working-tree package0.2.3 is pinned by content hashes in
reference/targeting-reference.json (main.js, ai.js, coins.js, fx.js and package).
The adapter executes actual source aimAngle/updateLock/leadDir and Coins.update.
Exact angle-state branch tests supply the angle dependency directly; separate
geometry and live tracking fixtures exercise the real vector math. No browser,
renderer, network or account function runs; UI/sound are numerical event sinks.
All source hashes were rechecked after builds. No commit/deployment claim.

| Check | Fresh source cases |
|---|---:|
| Geometric angle/range/eligibility | 576 |
| Exact supplied-angle cone/tie/charge branch | 384 |
| Persistent geometry-based lock tracking frames | 480 |
| Moving-target lead calculations | 256 |
| Late coin eligibility/phase probes | 4 |

Membership loss, dead/finished targets, acquisition versus retention ranges,
strict cone boundaries, angle0, ties, switching, losing/reacquiring a target and
charge completion are represented. Observed targeting fixture difference is
**0** on both targets, with1e-12 tolerance; this is bounded fixture
precision, not a universal cross-platform libm/near-threshold guarantee. Live lock
frames and exact scalar branch tests are deliberately distinguished.

Full native/WASM applications compile in isolated targeting-phases output folders,
and all30 suites pass. Prior pack states remain within their previous measured
error (about2.9e-13); unchanged functional counts match the preserved earlier
report. Earlier isolated AI/lifecycle/coin/physics/art tests remain passed.
No new renderer/culling experiment or graphical acceptance was substituted.

## Bounds, state preservation and pending work

Target candidates are bounded to ten local rivals; no new per-frame heap, world
shape, texture, upload, draw or production allocation is added. The pending-tail
state takes the pack to 14,336 native /
14,168 WASM bytes; event output remains3408.
Existing resource and graphics safeguards are unchanged. Only the shared pack API
was adapted; app/presentation files and runtime assets remain unchanged. Native
copied meshes and Web asset preload were verified, with prior reports/logs/Library
receipt and finish screenshot preserved. Project changes stay in win_app; browser
ownership, locks, user progress and accounts remain untouched.

No desktop/control or approval retry, process check, active-log read, termination
or graphics launch occurred. Last window-absent/process-unverified distinction and
Task Manager approval-timeout evidence are preserved. Await the owner process
answer or fresh supported approval; verify shutdown before any replacement launch.
Poster raster/material and GPU feasibility gates remain open. City coverage still
has413 canonical meshes / **1,947** unrepresented grid towers.

The next explicit independent gameplay slice is player arsenal heat/cooldowns/
seeker charges/spin/EMP triggers and later projectile/damage processing, preserving
the now-explicit phase barrier and source RNG calls. Player/session bridge, full
static-world/weapon coupling, manual race acceptance, visual/audio/performance and
account/network integration remain open. No product decision was needed for this
bounded offline targeting/phase work; broader network/economy choices were not made.

## Deliverables

Core rr_targeting.h/c; split rr_pack.h/c with convenience wrapper retained.
Tests test_targeting.c / generated_targeting_reference.h. Read-only source adapter:
tools/capture-targeting.mjs WIN_APP_ROOT OUTPUT.json; staging fixture generation:
tools/prepare-targeting-reference.py STAGING_DIRECTORY. Full source output and
results: reference/targeting-reference.json and targeting-phases-results.json.
Checksums/full logs: reference/verification-20261103/targeting-phases-builds.json
and both thirty-suite logs. Backups: reference/pre-targeting-phases*.
