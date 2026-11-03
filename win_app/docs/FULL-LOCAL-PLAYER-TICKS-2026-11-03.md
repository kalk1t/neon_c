# Full offline local player ticks and primitive-world coupling - 2026-11-03

The new shared-C local run controller closes the previous post-player boundary:
player motion/rules/lifecycle now precede the coupled ten-rival combat/coin pass.
Full isolated native and WebAssembly applications compile; each passes **35/35**
headless suites. Production gameplay remains inactive. This is a meaningful
simulation integration result, not human driving, UI or renderer qualification.

## Source order and behavior

rr_local_run.h/c owns one offline two-lap run's global/race/countdown clocks and
borrows RRLocalBattle and its player. Reset follows source Racer.reset ordering,
including the player's two random fire/reaction draws before rival reset draws.
It preserves global time, rank cadence, car effect history, cannon side and live
FX; clears source resources/locks/shots/coins and lap history count. No race ticket,
best-lap storage, rewards, account, wallet or progression operation is performed.

Countdown decrements first and steps the player idle, then idle rivals. The GO
frame retains its entered countdown branch: no race-time/player-rules/weapons or
coin collection that frame. Active/finished ticks advance race time once. A live
player captures previous position, runs existing world-aware car movement, advances
lap time once while unfinished, then applies checkpoint/lap/finish, local sample,
wrong-way and lost recovery. Lap history and safe pose are retained; source finish
changes phase before rival firing/player weapon gating, while the existing pack
preserves the old-sample finish-ranking point. Finished car movement continues with
supplied processed autopilot controls; lap and finish times remain frozen.

A wrecked player counts down and respawns from the current safe pose with speed35,
HP100 and visibility restored, with no player spawn RNG draws. Lost recovery follows
source floor34/off-track650 and timer>1.1. Smoke requests occur after player rules,
before the outer nitro/impact work and rivals. Nitro ignition uses source lastNitro
gate and contact shake; world impacts>4 add shake and spark FX at the correct point.
No new dodge/spawn/balance rule is introduced.

The frame then executes existing rival step/smoke/fire -> contacts/ranking -> player
lock/weapons. A new optional after-weapons hook applies an allowed manual reset
before projectile collisions and coin eligibility. Reset eligibility is a supplied
policy boolean; no identity/killer/account policy is guessed or read. The fixture
uses the source offline allowed case, not a production grant. Respawn events expose
the host aim-reset point; raw mouse/touch settings integration remains outside.

Player muzzle and side-axis now come from numeric source Car.sync YXZ pose, including
roll/spin/stun wobble. The pose is sampled before contacts; source contacts do not
resync the model at that point. Projectiles/world impacts/damage/FX aging and late
coin FX then run in their existing source order. Pause freezes movement, cooldowns
and FX lifetimes while global time and player idle animation time advance, matching
source Car.idle's later effect on resumed muzzle pose. No frame is double-stepped.

The existing flight API gained an optional RRCarStepReport carrying the actual
collision normal/amplitude; its legacy void wrapper retains old behavior and car
layout. The existing battle API gained an optional manual-reset placement hook and
a side-axis pose helper; null-hook callers retain earlier outcomes. App/session/
presentation call sites remain unchanged. Future integration must use one tick
owner rather than step both old session and this controller.

## Reference and fresh scoped evidence

Working-tree browser package0.2.3 is pinned by hashes of main/fx/audio/ai/car/coins/
track/roads/maps/physics, package and Three core. The adapter executes actual source
frame CPU branches, updateAim/autopilot, Car movement, stepRacer/raceRules/respawn,
nonreward damage/wreck, ten rivals and firing, contacts/rank, weapons/FX and coins.
Rendering/vehicle updateFX and results timer/UI are excluded. Source checkpoints,
lap/finish/respawn/smoke events are instrumented at their actual function calls.

Two road traces (1 and9),640 complete local ticks and2 restarts replay processed
control/aim inputs, not player states or per-frame RNG results. Player/rival states,
clock/lap history/safe positions, source event bitmasks, weapon state, all shots,
FX lifetimes, coins and continuous RNG remain live across frames. Inputs include
raw dt0.2 capped to0.05, boost/steering/brake/climb/fire, pause/resume, HP/wreck/lost
faults, manual reset, a scripted checkpoint and two scripted eligible finish-plane
crossings. These teleports establish bounded rules/finish behavior; they are not a
legitimately driven two-lap race or progression validation.

Fresh outcomes on each target: 5667 exact sequence RNG draws,
2 GO transitions, 4 laps /2 scripted
finishes, 6 player respawns (including2 manual
resets), 6 player smoke requests, 6 nitro ignition
events, 30 paused ticks and6 local collections.
Peak8 shots /58 FX. Every final source event
bitmask matches the C output, not just final racer state.

Road1 uses an actual empty source Colliders instance. Road9 uses actual source OBB,
vertical cylinder and torus, matched by the existing C RRWorld primitives: a rotated
box at(8,118,55), cylinder at(-8,45), and torus at(0,126,10). The player and all
rivals run collision resolution against that world; shots use source segment
marching and existing rr_world_segment with radius0.6. Fresh matched coverage is
5 player impacts>4 and9 projectile world hits,
including normals/amplitudes and per-frame segment call/hit counts. This is real
algorithmic coupling on a small analytic world, not a hit/miss stub. Full canonical
city/course assets, route clearance and large-world resource/performance remain
separate; no claim of whole-world collision acceptance is made.

Maximum measured numeric difference: native 5.1869619710487314e-13, WASM
5.1869619710487314e-13, with inherited1e-5 coupled-physics tolerance.
Native/WASM functional summaries, RNG states/draws and event counts match. Earlier
pack/targeting/arsenal/projectile/fire/battle fixtures pass fresh; their functional
counts are unchanged (their fresh numerical maxima are recorded in run-results.json).
Earlier test/fixture hashes were checked unchanged. No near-threshold universal
floating-point guarantee or graphical acceptance is inferred.

Validation refinement is preserved in run-event-sequencing.json and
pre-run-event-sequencing-20261103. The first focused run already matched640 states
and RNG frames, but review found an outer current_events |= player_step(...) could
overwrite flags set inside player_step. Separating the call from the assignment
fixed sequencing; actual-source per-frame event hooks/assertions were added before
final qualification. Original core/test/reference versions and initial summary are
retained. No graphical trial or prior failure was represented as passing evidence.

## Assumptions, bounds and deliverables

The explicit warm-resource profile remains: cached textures/shared Sprite geometry,
pinned Three UUID draw accounting and actual Sound ctx=null, including unconditional
blaster variation. Cold assets, renderer/asynchronous allocations, vehicle FX and
initialized audio/voice gates can consume further RNG and remain unqualified.
Car/model/muzzle numeric pose is covered; visible models, camera/HUD effects and
audio output are not. Processed input/control and policy are host boundaries; native
mouse/gamepad/touch/settings and pause-button timing were not exercised.

Caller-owned test128-shot/512-FX storage stays isolated; peaks do not establish a
production budget. Lap history reuses the existing growable local history pattern.
No cache/mesh/upload/draw/texture cap, safety setting, runtime asset, unlock, wallet,
account contract, network, paid call or publication changed. All writes stay inside
win_app; Claude's browser files are read-only. Native copied canonical assets and
WASM preload match; earlier evidence, Library receipt/screenshot and desktop record
are preserved. Potentially in-use executable/assets were not replaced.

Implementation paths: new src/core/rr_local_run.h/c; optional reporting in
rr_flight.h/c; hook/pose extension in rr_battle.h/c; CMake wiring;
tests/test_local_run.c/generated_run_reference.h; tools/capture-run.mjs and
prepare-run-reference.py STAGING_DIRECTORY. Reference: run-reference.json,
run-results.json and verification-20261103/player-run-builds.json plus full native/
WASM thirtyfive-suite logs. Originals: pre-player-run-20261103; documentation
originals: pre-player-run-docs-20261103. No app/presentation integration was activated.

## Real next boundary

The offline simulation spine now has a tested complete tick owner. Adding more
scripted loops would not qualify the remaining native input/host ownership bridge,
actual full-scene collisions/loading, visual weapon/FX/audio behavior, manual race,
GPU resource feasibility or cold-start behavior. Production rewards/account/live
cross-play remain separate agreed-integration work; source reward branch is still
excluded, while volatile local race/coin state is simulated normally.

Next useful checks are controlled native host integration and interaction/graphics
qualification with one tick owner, supplied real world data and unchanged safeguards.
That desktop path remains gated: no graphics launch, desktop/approval retry, process
query, active-log read or termination occurred. Previous Task Manager app-access
approval timed out; RoadstersRace.exe shutdown remains unverified even though the
old window was absent. Obtain fresh owner/process evidence or supported approval,
verify the intended old process is stopped, then launch one isolated controlled
build. Validate input/start/pause/reset/finish, visible FX and shutdown; measure
resources/GPU before any budget decision. City still has413 canonical meshes and
1947 unrepresented source-grid towers. No whole-game/manual acceptance is claimed.
