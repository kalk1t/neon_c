# Offline full-roster post-player combat coupling - 2026-11-03

Shared C now couples all ten local rivals to the pack's existing AI/lifecycle,
source rival firing, contacts/ranking, player lock/weapons, projectiles/damage/FX
and late coin/onCoin effects. Full isolated native and WebAssembly applications
compile and each pass **34/34** headless tests. Production combat/rivals/coins
remain inactive. This is a post-player integration slice, not full game acceptance.

## Ordering and implementation

rr_pack_rivals_dispatch adds an optional synchronous hook immediately after each
rival step. The hook requests smoke from the actual step flag, then runs source
rivalCombat before any later rival advances. This preserves sequential peer
positions, target eligibility, FX counts and RNG order. Existing combat-free
wrappers pass null hooks and retain their exact earlier results.

Rival firing gates at raceTime>=6, live/nonfinished and race state, then decrements
fireCD. Target search follows source roster order, excludes self/dead/finished,
includes distance15 and230, and accepts cone cosine>=cos(0.3). The nearest eligible
target wins; ties keep the earlier entry. No target resets fireCD0.4 with no draw.
An accepted shot initializes the generic rival projectile (length22,width7.5),
consumes the warm Mesh UUID allocation draws, then draws the next firing interval
(4+random*4)*road.fire. No spread, flash or redesign is added to rival fire.

Numeric rr_combat_muzzle reproduces source Car.sync YXZ pose, including roll,
barrel-roll curve and stun wobble. Rival muzzle sampling happens before contacts.
Player muzzle/side-axis/aim are supplied from the caller's existing source pose;
contacts move simulation position but do not resync the source model at that point.
The new fixture checks the resulting initialized shot poses and subsequent motion.

RRLocalBattle composes: rival step/smoke/fire -> all contacts -> ranking -> player
lock/weapons -> reverse projectile/impact/damage/wreck and FX aging -> coin update
and immediate onCoin impact FX. A new optional coin observer executes right after
the in-memory award, before later coins. Coin FX are born after this frame's FX
aging, matching source; they are not aged early. No wallet/progression update.
Paused frames skip all of this work; countdown advances idle rivals and nonplaying
projectile/FX/coin timing. Race/alive player gating remains explicit.

Wreck countdown, stuck and lost respawns stay in existing rival rules. Their two
spawn RNG draws occur at the actual respawn, followed by source same-frame
rivalCombat when eligible. Smoke is skipped on the wreck-return path. Respawn
preserves the source firing cooldown and effect history instead of inventing a
new cooldown or protection gate. Reset restores pack/coins/resources/locks/shots,
preserves alternating cannon side and existing FX, and retains ranking cadence.

An optional shared shake pointer in RRFireLocal now connects source weapon/damage
shake with pack contact shake. It mirrors bolt clamp, seeker/EMP additions, player
hit clamp and player-wreck value without rendering. Existing callers leave it null
and retain their earlier fixture outcomes. Camera rendering/kick/UI are not claimed.

## Actual source reference and fresh results

Working-tree browser package0.2.3 is pinned by hashes of main/fx/audio/ai/car/coins/
track/roads/maps, package and Three core. The adapter executes actual source
post-player rival loop/step/rules, rivalCombat, contacts/ranking, lock/playerWeapons,
Weapons class and Coins.update/onCollect. Only the takedown/reward branch is omitted;
nonreward wreck and player shake remain. No renderer, window, service or account
operation runs. Source files remain read-only.

Three traces cover roads1,5,9,720 post-player frames and3 restarts. Controlled probes
set firing readiness, move the player to a coin, create a contact, trigger smoke,
short/long wreck timers, stuck/lost recovery, pause and restart. Player pre-phase
state/pose and frame clocks are supplied; all ten rival states, shots, FX and RNG
persist through the sequence. RNG is not replaced per rival or per frame.

Fresh scoped outcomes on both targets: 25504 continuous sequence
draws, 118 contacts, 14 coin collections,
12 respawns, 753 smoke requests, 15
paused frames and12 countdown frames. Peak17
shots /112 FX in these traces. Separate192 source rival probes
exercise range/cone/cooldown/early-race/live/finished/tie conditions and sampled
stunned/rolling muzzle pose; 13 accepted fires. Comparisons
include roster numeric/lifecycle state, fireCD/react/smoke/roll timers, contacts'
resulting positions/velocity/shake, rank metadata, arsenal/lock, all surviving shot
fields, FX time/life, all coin state/events, exact RNG state and draw count.

Maximum numeric difference: native 2.5934809855243657e-13, WASM
2.3980817331903381e-13; tolerance1e-5 inherited for coupled physics.
Functional summaries and RNG counts match across targets. These are bounded
samples, not a universal floating-point threshold or human-driven race guarantee.
The prior4320-tick/nine-road combat-free pack, targeting,4324 arsenal cases,2130
combat cases and seeded fire/FX summaries retain their exact previous per-target
results. All34 suites are fresh on both targets. Focused native and complete
native/WASM checks passed on their first execution for this slice; no failed run
was repurposed as acceptance. Historical earlier corrections remain preserved.

## RNG, ownership and resource limits

The existing explicit RNG profile is retained: cached textures/shared Sprite
geometry and pinned Three UUID allocation accounting; actual Sound ctx=null,
including unconditional blaster variation. Rival smoke consumes the same stream
before rival fire's Mesh/interval draws. Respawn pairs, projectile trails/impacts/
explosions and coin impacts consume their source-order draws. Initialized audio,
cold loading, renderer/asynchronous work and vehicle updateFX are outside this
profile. They must not be called equivalent based on these headless results.

Caller supplies shot and FX storage. Test128-shot/512-FX arrays are isolated
fixtures, not production allocations or new resource caps. Capacity errors remain
explicit and unqualified; no geometry/draw/upload/cache/texture budget changed.
RRLocalPack/coins layout is unchanged. No new app/presentation call site activates
this module, no progress/unlock changes, no account/network writes, paid calls or
publishing. All writes stay inside win_app; browser/Claude-owned files are unchanged.

Deliverables: src/core/rr_battle.h/c; optional hook extensions in rr_pack.h/c and
rr_coins.h/c; optional shake coupling in rr_fire.h/c; CMake wiring;
tests/test_battle.c/generated_battle_reference.h; tools/capture-battle.mjs and
prepare-battle-reference.py STAGING_DIRECTORY; reference/battle-reference.json,
battle-results.json and verification-20261103/battle-builds.json with full native/
WASM thirtyfour-suite logs and hashes. Originals: pre-battle-coupling-20261103.
App/presentation, all other existing core, assets, prior evidence/Library receipt/
screenshot and desktop evidence were verified preserved. Native copied canonical
meshes and WASM preload hashes match prior artifacts. Existing safeguards remain.

## Remaining integration and exact gates

This bounded full-roster post-player slice is complete. It does not establish:
- The actual player input/session/movement/physics -> pose/muzzle -> this boundary,
  player automatic wreck/respawn or manual reset through a complete race. Player
  pre-phase states are supplied fixtures, not controller or human driving evidence.
- Real static-world car/shot collisions, complete city coverage or resource/perf
  feasibility. Rival car colliders and shot world callbacks are empty in these
  sequences; floor behavior remains source34 car /24 projectile. Existing world
  tests remain separate; city coverage stays413 canonical /1947 missing grid towers.
- Coupled finish/results/menu/shop behavior beyond existing isolated tests. New
  sequences cover countdown/playing/paused only. No full two-lap race acceptance.
- Takedown loot/boost/seeker/streak rewards, wallet/progression/profile/remote
  identity or live protocol. Reward branch is deliberately excluded in reference
  and C; local coin awards are in-memory only.
- Vehicle FX, complete camera/HUD, rendered weapon visuals/materials, initialized
  audio/voice gates and cold/asynchronous allocation RNG. Full-browser seeded RNG
  identity, graphics stability/performance and human race acceptance remain open.

Further source-only work can still address the player-phase bridge; no new product
choice was needed here. Production activation requires that bridge and the actual
graphics/performance/interaction gates, not more claims from headless fixtures.
Do not silently grant a production shot/FX budget from fixture storage sizes.

No graphics launch, desktop/approval retry, process query, active-log read or
termination occurred. Window absence still does not prove shutdown: the previous
Task Manager app-access approval timed out and process state remains unverified.
Next desktop check requires fresh owner/process evidence or supported approval;
verify the intended old process is stopped before launching any replacement.
Then controlled renderer/FX/texture/GPU checks can resume under the existing
safeguards, followed by actual player interaction/race validation. No current
headless result substitutes for those checks.
