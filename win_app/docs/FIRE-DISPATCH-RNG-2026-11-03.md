# Offline fire initialization and synchronous RNG-aware dispatch - 2026-11-03

Full isolated native/WASM applications compile and each pass **33/33** headless
tests. New rr_fire.h/c connects player arsenal events to initialized projectiles,
dodge/EMP state effects, projectile impacts/damage/wrecks and numerical FX lifetime/
random-consumption accounting. The arsenal API now offers synchronous dispatch;
its legacy no-callback wrapper retains all previous fixture results. Production
combat/rivals/coins remain inactive; this is not integrated-game acceptance.

## Initialization and source-order dispatch

rr_fire_init ports Weapons.fire defaults: inherited speed plus nullish speed240,
normalized direction, accel||0, maxSpeed||computed speed, nullish life2.4 and
turn(owner player5/other2.2), optional explicit damage (zero retained), length||18,
width||6, radius||3.2, trail/always and kind. Zero speed/life/turn are distinct from
absent values. Length/width return as initialization metadata; initialized shot
state uses existing RRShot storage. It allocates nothing and consumes no RNG.

rr_arsenal_step_dispatch invokes callbacks at each original source event, before
continuing later arsenal code. This resolves a real composition hazard: replaying
events after the old step would place flash/audio draws after an unsteered spin
draw. The old rr_arsenal_step calls the same code with no callback and is unchanged
in behavior. Event structs count arsenal draws only; the shared callback stream
also records FX/engine/audio draws. Reentrant/state-mutating callbacks are forbidden.

The local dispatcher uses supplied muzzle/side-axis/aim and current lock. Bolts
offset the alternating cannon, lead target with existing targeting math, add spread,
initialize the source bolt and flash before blaster cue/heat. Seekers initialize
with source upward aim offset, acceleration/turn/lifetime and charged-lock choice.
Spin applies existing rr_car_dodge before cue. EMP requests shockwave FX, purges
enemy shots in reverse order, emits cue then applies roster-order impulse/damage.
Reverse projectile updates request trail FX before impact/damage; a wreck requests
explosion FX and releases the local lock. Takedown/reward branches are excluded.
FX age after projectile dispatch; caller must still execute the late coin tail.

## Explicit RNG profile and limits

Source identity: working-tree browser package0.2.3, SHA256 of main/fx/audio/ai/car,
package and pinned Three core in reference/fire-reference.json. Capture executes
actual source Weapons class, playerWeapons, lock/lead, nonreward damage/wreck,
and Sound with ctx=null. CPU-only Three objects run without a renderer/window,
with a cached-texture stand-in and prewarmed shared Sprite geometry.

The ledger tracks source FX times/lifetimes and stable removals because length
controls smoke/trail gates. Source-only draws: sprite2, flash/impact/wave2,
explosion69, smoke4, trail4. Smoke is blocked only when count>260; trail when
count>=300 (299 can become301). No distance cap or geometry omission is introduced.
The explicit pinned-Three warm-allocation profile additionally accounts8 UUID
draws per sprite,16 per wave's two mesh/material pairs and4 per shot Mesh. This
was verified against actual pinned constructors, not assumed from FX code alone.
Therefore warm flash/impact10, wave26, explosion165, smoke12 and trail20 draws.
Explosion randomized lifetimes retain their exact position in this stream.

Audio cue callbacks execute at source positions. Actual Sound.blaster consumes
one unconditional random value even with ctx=null; the test host reproduces it.
Other source audio calls in this test profile do not consume device/rumble draws.
This is not initialized-audio RNG equivalence: audio buffers, rumble voice gates,
asynchronous loading, cold textures/geometry, renderer internals and unrelated
game FX introduce additional draws not covered by this profile. No native visual
appearance, live audio, cold-start or whole-browser seeded equivalence is claimed.
Source-only/profile modes are explicit; neither is activated in the app.

## Fresh tests and retained evidence

96 actual fire/default fixtures,35 FX gate/allocation/lifetime probes and480 full
bounded combat-phase frames across two seeded streams with4 restarts. The seeded
host supplies static car poses, source roll/surge timer ticks, deterministic input,
explicit rival-shot injection and a controlled hit/miss world callback. It executes
lock -> player weapons -> projectile/damage/FX aging; actual scene objects and FX
methods consume the source stream. It does not simulate full player/AI movement.
Restart resets resources/actors, preserves cannon side and existing FX, and clears
shots as source Weapons.clear does. Every frame checks arsenal/lock, actor state,
all surviving shot fields, every FX lifetime/time, RNG state/draw count and world
query count. Pure fire initialization also verifies visual length/width defaults.

Fresh results: 7828 sequence draws, 1209 FX
probe draws, max10 simultaneous shots and48 FX in
these traces. Native/WASM summaries match exactly; maximum measured difference
0 (tolerance1e-10). This is bounded fixture precision, not
a universal libm/threshold guarantee. The existing4320-tick/nine-road pack,
targeting,4324 arsenal cases and2130 combat cases retain their exact prior target
summaries. Full33 suites are fresh on each target; earlier graphics evidence is reused
only as historical evidence and does not qualify this work.

Validation history is preserved. Initial MSVC /WX rejected uninitialized test locals
whose initialization depended on first fixture metadata; explicit zero initialization
resolved it, with no core change. The first numeric run exposed a reference-harness
omission: weapons.racers was not set like startRace, so source shots missed all cars.
Assignment fixed the reference, preserving the original (first divergenceframe6,
30254 downstream assertions); no C core changed for that discrepancy. Final references
and both complete successful logs are separate. See fire-test-initialization.json,
fire-roster-reference-correction.json and pre-fire-test/pre-fire-roster-reference
backups. No failed reference was treated as game acceptance.

## Bounds, deliverables and remaining full-world couplings

Caller owns shot/FX arrays; no per-frame heap, production allocation, texture/upload,
draw or resource-cap change. Test storage64 shots/512 FX is an isolated validation
fixture, not a new production budget. Capacity shortage latches an explicit error
and cannot be a qualified run; no silent drop/cap substitution. A shortage probe
verifies failure before RNG consumption. The module models FX count/time/life,
not sprite positions/appearance, waves, lights or materials.

Changed implementation: new src/core/rr_fire.h/c, src/core/rr_arsenal.h/c callback
extension, CMake/test wiring; tests/test_fire.c/generated_fire_reference.h,
tools/capture-fire.mjs and prepare-fire-reference.py STAGING_DIRECTORY. References:
fire-reference.json, fire-results.json; verification-20261103/fire-builds.json and
native/WASM thirtythree-suite logs. CMake/arsenal originals are in
pre-fire-dispatch-20261103. App/presentation, all other existing core, assets,
earlier logs/reports/Library receipt/screenshot and desktop evidence were verified
preserved; native copied canonical assets and WASM preload hashes match.

Still open, specifically:
- Actual player input/session/physics movement and body/muzzle quaternion/pose bridge.
- Sequential ten-rival AI/rivalCombat target selection/fireCD RNG, contacts/ranking
  and this weapons phase in one orchestrator. Fixtures inject generic rival shots;
  they do not claim the full pack is coupled to this dispatcher.
- Real full-world segment geometry, all-route city coverage/resource feasibility;
  new sequence fixtures use controlled world callbacks. Existing collision tests
  remain separate; canonical city still413 meshes /1947 missing source-grid towers.
- Late coin collection and onCoin/other game FX consuming the shared RNG stream;
  source-phase boundary exists but is not invoked by the dispatcher.
- Complete wreck/respawn/session lifecycle and takedown/boost/missile/loot rewards.
  No wallet/progression/account/network or remote ownership behavior was connected.
- Cold resource-loading/renderer RNG, initialized audio, full FX rendering, camera,
  manual race, graphical/performance and human acceptance. These remain gates.

The next source-defined offline slice can be rivalCombat and ordered local-pack
dispatch with the now-explicit RNG hook, rather than enabling production combat.
No product decision was needed for this bounded slice.

No graphics launch, desktop/approval retry, process query, active-log read or
termination occurred. Last window-absent/process-unverified state remains: Task
Manager app-access approval timed out, so shutdown has not been verified. Await
fresh owner/process evidence or supported approval, then verify old intended
process stopped before any replacement launch. Existing graphics safeguards remain.
