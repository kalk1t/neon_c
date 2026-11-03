# Combat-free local pack orchestration - 2026-11-03

The shared C modules now run together in a source-ordered **post-player local pack
pass**: ten rivals, their lifecycle/race rules, contacts, ranking and coins. Native
and WASM each pass **29/29** headless tests. **Production rivals/coins remain inactive.**
This is a bounded combat-free projection with caller-supplied player stepping,
not complete gameplay, a human-driven race or graphical/integration acceptance.

## Reference and explicit boundary

Browser working-tree package0.2.3 is pinned by hashes in reference/pack-reference.json;
main.js, ai.js, car.js, coins.js, track.js, roads.js, maps.js and package.json are
read-only inputs. The adapter executes actual source reset/frame CPU branches,
Racer/Car/Track/Coins methods, AI, contacts and ranking. Rendering/audio/UI/storage
are sinks; rival/player combat, projectile/FX updates and static-world collisions
are excluded. No backend/account function is called and no browser source is edited.

The player phase is an explicit **caller boundary**: the reference supplies the
post-player physics/rules/FX state to C. The pack borrows that player object and
does not own/free its storage or certify a new player controller/countdown clock.
The host also supplies the branch entered by frame(), positive already-capped dt,
global time and race clock. In particular, the final GO countdown frame still
uses the idle countdown pass, even though the source changes g.state to race.

## Source ordering and lifecycle interactions

New rr_pack.{h,c} preserves:

1. Grid placement: source player slot7, ten rivals in source livery order, skipping
   that slot. The 12-slot grid leaves slot11 unused. Persistent driver seed/lane/
   dodge and car effects follow the existing source reset semantics.
2. Countdown: rivals receive IDLE physics only. No AI, lap/recovery stepping,
   contacts or rank timer decrement. Coin pop/timers advance with collection inactive.
3. Playing: reaction gate is strict raceTime>react. Rivals advance sequentially;
   later drivers see updated earlier cars, including newly respawned cars. Each
   driver's coin target is selected before the end-of-frame collection pass.
4. Local contacts resolve after all rivals, then periodic ranking/rubber banding
   runs at the source rankT<=0 cadence, resetting rankT to0.2 without carrying
   overshoot. Stable ties start from source racer order on each ranking update.
5. Player finish triggers the source immediate ranking before the rival pass,
   using the previous cached player sample: completeLap ranks before raceRules
   refreshes that sample. Finish position is retained separately. Start/restart
   preserves the outer rank timer rather than silently resetting its cadence.
6. Paused ticks do not advance rivals, coin timers/pop, contacts or rank timer.
   Global time belongs to the caller, as source time advances before this branch.
7. Coin pickup/respawn/pop and racer counters are updated with the tested source
   order. Finished-but-alive cars continue driving/contacting, while coin collection
   skips finished/dead racers, matching the source.

Rival lifecycle now offers a demand-driven random callback. Reset requests one
two-value pair per rival; recovery/wreck respawn requests a pair only when it
occurs, in actual racer order. The old fixed-value APIs remain wrappers, and all
previous lifecycle equivalence tests still pass. Tests supply actual controlled
source values; no runtime RNG algorithm or cosmetic RNG policy was invented.

**Weapon phase limitation:** the source has weapon/FX work between ranking and
the later coin pass. This combat-free helper combines those numerical sections
only because the excluded operations are empty sinks. Stateful combat integration
must split or hook the coin tail back to its exact source position; otherwise a
same-frame wreck could wrongly collect a coin. The header documents this boundary.
The final header clarification changed comments only, with C tokens verified
unchanged after the tested builds; its pre-comment file is preserved.

## Fresh headless scope

| Coverage | Count |
|---|---:|
| Roads / controlled traces | 9 /18 |
| Post-player pack ticks | 4,320 |
| Countdown / playing /paused ticks | 2,376 /1,899 /45 |
| Restart probes | 9 |
| Rival respawns | 18 |
| Requested two-value RNG pairs | 288 |
| Native contact resolutions exercised | 633 |
| Ordered coin collections compared | 58 |

Source/reference state is compared every tick for each rival and the borrowed
player's contact/coin output: position/velocity/forward, steering body state,
boost/multiplier, cached sample/dodge, lap/checkpoint/history count, hp/wreck/lost/
stuck, coins, visibility and finish state. Ranking order/position/timer/shake,
coin active/timer/pop state, collection event identities and RNG request values/
owner/order are also compared. Rival states persist across ticks; they are not
replaced by source snapshots. Player stepping is supplied externally by design.

Fault probes explicitly inject wreck/stuck/lost, a finished rival, a coin/contact
cluster, pause/resume, a scripted player finish and a restart. These are test
inputs, not newly enabled production behavior or driven full races. The633 contact
resolutions are exercised inside source-state-equivalent traces; per-pair source
audio/particle event parity is not claimed by this new pack suite. The earlier
843-pair isolated contact test remains passed.

Maximum compared state error: native **2.8244e-13**,
WASM **2.8155e-13**, tolerance1e-5. Functional counts/
RNG/collection/ranking assertions pass on both; floating error maxima and structure
sizes differ as expected by platform. This is numerical parity for the stated
projection, not graphics/performance, full static-world collision, combat or
human race acceptance. No fresh renderer/culling experiment was performed.

## Resource and ownership bounds

Pack size is 14,328 bytes native /14,160
bytes WASM, plus a 3,408-byte output event object. It owns ten rival
states and one bounded coin field, borrows the player/track, and uses fixed eleven-
car contact/coin views. The orchestrator adds no per-frame heap allocation; existing
source-compatible rival lap-history growth remains separate. Dispose frees only
owned rival histories. No new world shapes, GPU texture, draw, upload or production
allocation is activated, and existing safety/resource caps remain unchanged.

Both full applications build in new local-pack-native/web folders. Existing app/
presentation files, assets, copied canonical meshes and Web asset preload remain
unchanged. The only pre-existing gameplay module changed is rr_rival.h/c for the
callback adapter; its prior state is backed up and all legacy fixed-draw tests pass.
Historical logs/receipts and the Library finish fixture are preserved. All project
edits stay within win_app; Claude-owned files and user progress remain untouched.

## Pending gates and actual remaining gaps

No desktop/control or approval retry, process query, active-log read, termination
or graphics launch occurred. The previous window-absent/process-unverified state
and Task Manager approval timeout are preserved. Await the owner's process answer
or fresh supported approval, verify the intended old process is stopped, then
resume bounded source Canvas/native GPU work. No replacement instance was launched.

Before production activation: implement/validate the player/session adapter and
the source clock/phase boundary, add static-world coupled fixtures, preserve the
weapon/coin phase barrier during combat integration, then qualify model counts,
graphics and a human-driven race. Combat targeting/heat/charges/projectiles,
damage/rewards, presentation/audio and shared account/network integration remain.
Useful independent headless work still exists in the source weapon state machine;
no need for speculative culling or an invented account/progression policy.
City status remains413 canonical meshes and **1,947** unrepresented grid towers;
poster raster/material graphics are still pending.

## Deliverables

- src/core/rr_pack.h/c; rr_rival.h/c callback adapter with fixed APIs retained.
- tests/test_pack.c and generated_pack_reference.h.
- tools/capture-pack.mjs WIN_APP_ROOT OUTPUT.json and
  tools/prepare-pack-reference.py STAGING_DIRECTORY.
- reference/pack-reference.json and local-pack-results.json.
- reference/verification-20261103/local-pack-builds.json and both complete
  twentynine-suite logs. Backups: reference/pre-local-pack*.
