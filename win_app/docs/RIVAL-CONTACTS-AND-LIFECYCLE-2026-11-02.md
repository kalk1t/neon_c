# Local car contacts and rival reset/respawn - 2026-11-02

Shared C now implements the source local car-contact response and rival lifecycle,
validated against actual browser functions on all nine roads. **Live rivals remain
inactive.** This is an offline simulation slice, not a complete AI race, network
behavior, human-driven race acceptance or graphics qualification.

## Browser reference and implemented semantics

Browser working-tree package 0.2.3, identified by the content hashes in
reference/rival-reference.json. Captured sources: main.js, ai.js, car.js, track.js,
roads.js, maps.js and package.json. No commit/deployment claim or browser write.
The adapter calls actual Racer.reset and source respawn/raceRules/stepRacer/
carContacts; Car update/place and Track lookup remain the actual source methods.
Explicit RNG draws make reset/jitter reproducible without choosing a new runtime
random policy. Rendering/audio/FX and static-world collision calls are sinks.

New src/core/rr_rival.{h,c} provides:

- Ordered local pair contacts at **4.6 total separation**, not two 4.6 radii. Skip
  distances >=4.6 or <0.001, split penetration equally, apply the source 0.6 normal
  impulse only while approaching, and preserve scalar speed/forward/health. Dead
  cars skip; finished-but-alive cars still contact. Player hits strictly above
  speed5 emit the source volume/shake cue; no actual sound or damage is applied.
- Source rival reset: Car.place rather than a full Car state wipe, boost1, visible,
  cleared race/lap history/checkpoint set, hp100 and reset transient racer counters.
  Seed/lane/dodge, car multiplier and retained FX history remain as in source.
  Fire delay is specifically 4+random*(4/1.5), not the later combat 4-to-8 interval;
  reaction delay is 0.1+random*0.45.
- Rival respawn at last-safe yaw and speed35 with **world X/Y jitter** (+/-6,+/-3),
  no Z jitter, optional hp preservation, global sample rebase and lost/stuck reset.
  Lap/checkpoint/history, coins/takedowns, boost and other preserved effects remain.
- Wreck countdown skips movement/lap timing on the expiration frame before respawn.
  Stuck recovery uses strict speed<12, raceTime>3 and stuck>2.5. Lost recovery uses
  the source floor34/off-track>650 and timer>1.1. Sample lookup and post-respawn
  recovery ordering match source. Low-health smoke timing emits a numerical flag,
  without creating visuals.
- Existing shared race/checkpoint logic is reused, with a source checkpoint bitset
  and dynamically retained lap history. Allocation growth is checked; reset reuses
  storage and dispose frees it. No fixed artificial time-attack lap-history cap.

The module is local-rival-only for rules/respawn: it explicitly refuses player
and FREE ROAM respawn paths. Local contact arrays may include the player just as
source g.racers does; no remote roster, live spawn slot, roam invulnerability,
server damage, wallet, loot, unlock or account persistence has been implemented.
Resetting a simulated rival's in-memory counters is not an account write.

## Fresh equivalence evidence

| Actual source check | Coverage |
|---|---:|
| Racer.reset | 54 |
| respawn, keep-hp variants and jitter | 72 |
| Recovery/strict-threshold rule states | 108 |
| Scripted ordered gate/two-lap endpoint frames | 252 |
| Mixed-dt stepRacer frames | 1,296 |
| Two-car and 11-car contact configurations | 84 |
| Ordered contact pairs compared | 843 |
| Source player hit-volume/shake cues | 8 |

All nine roads are covered. Comparisons include position/velocity/forward,
scalar car/effect state, visibility, random delays, driver lane/seed/dodge,
last-safe transform, hp/wreck/recovery timers, lap/checkpoint/history and local
counters. Coincident and threshold distances, dead/finished cars, separation/
approach, ordered multi-car response, wreck expiry, stuck/lost recovery and
reset preservation are represented. Invalid indices/RNG, player/network respawn
boundaries and fixed contact-array limits are checked.

The two-lap fixtures **script endpoint crossings**; they do not drive a human or
AI through complete races. The 1,296 step frames are short no-static-contact
lifecycle scenarios, not a complete eleven-car/world/combat integration test.
Maximum observed numerical state error: **4.0927e-12** at tolerance
1e-7. Native/WASM summaries are identical. Full native and WebAssembly applications
compile in isolated rival-life output folders; each fresh headless suite passes
**27/27**. Existing AI, race, world and material tests remain passing. Culling suites
ran only as unchanged regressions; speculative culling remains paused.

## Resource/state boundaries

Contacts are bounded to eleven local racers and55 event slots, with no heap
allocation for pair resolution. Only lap history grows as required by the source
simulation. No application code activates these objects, no new world colliders
or rendered cars are added, and all existing graphics/resource/safety limits stay
unchanged. Existing app/presentation/core source files and assets remain unchanged;
only the new shared core module, fixtures, targets and reports were added.
Native copied assets and unchanged WASM preload were rechecked. Existing source
hashes, historical test logs and the Library receipt/finish fixture are preserved.
All project mutations stay in win_app; Claude-owned files are read-only.

## New desktop authorization and actual result

The owner authorized resumed desktop control at23:29 UTC: "You can take control
again." Supported cua_repl inventory succeeded. **Window3148188 is no longer
listed, and no native Roadsters window is present.** It was not closed by this
agent; the observed absence does not establish when/how the old process exited.

To verify process shutdown before any new launch, the existing Task Manager
window265208 was selected through cua.getApp. That operation returned **"Computer
Use app approval timed out"** after300s. No process inspection or input occurred.
The old input-protection blocker is therefore only partially reconciled: inventory
is available and the window is absent, but process shutdown and input delivery
remain unverified. This is an app-approval timeout, not a new input-protection
rejection. No repeated selection, alternate denied process lookup, kill, active-log
read/retry, graphics launch or duplicate instance followed.

**Exact pending action:** approve the supported Task Manager process check (or
provide explicit owner confirmation the old Roadsters process has stopped), verify
shutdown, then resume bounded source Canvas capture and native GPU/material tests.
The fresh owner authorization is preserved in reference/desktop-reconciliation-
20261102.json together with these scoped observations. Existing facade-windows-view
executable/bin/assets were preserved. No GPU/visual/performance result is claimed.

## Remaining gameplay and continuation

The native application remains a solo offline slice. The next useful independent
step is a local pack orchestrator using the now-tested AI, car, contacts, race and
lifecycle modules, with source start-grid/reaction/ordering and full simulation
traces before renderer activation. Combat targeting/heat/charges/projectile damage,
coins/rewards, ranking/results pack presentation, audio and manual race validation
remain open. Network/account behavior needs its own agreed integration; no such
contract was changed. City status remains413 canonical meshes and **1,947**
unrepresented grid towers; poster pixels/material graphics remain pending.

## Deliverables

- src/core/rr_rival.h and rr_rival.c, compiled into shared core with no app activation.
- tests/test_rival.c and generated_rival_reference.h.
- tools/capture-rivals.mjs WIN_APP_ROOT OUTPUT.json and
  tools/prepare-rival-reference.py STAGING_DIRECTORY.
- reference/rival-reference.json and rival-life-results.json.
- reference/desktop-reconciliation-20261102.json for exact UI/approval status.
- reference/verification-20261102/rival-life-builds.json and both complete
  twentyseven-suite logs; source/target backups in reference/pre-rival-life*.
