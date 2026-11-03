# Offline projectile, damage and EMP state effects - 2026-11-03

New shared C rr_combat.h/c advances initialized local projectiles and resolves
offline damage/EMP state effects. Full isolated native/WASM applications compile
and both headless suites pass **32/32** after correction described below. Production
combat/rivals/coins remain inactive. Browser files, existing core/app/presentation
code, assets, caps, progression and account/network behavior are unchanged.

## Source behavior implemented

Current browser working-tree package0.2.3 is pinned by hashes of main.js, fx.js,
ai.js, car.js, physics.js and package.json in reference/combat-reference.json.
The fixture adapter executes actual Weapons.update/purge with rendering, texture
and FX sinks; actual damage body with event-bit annotations; the exact wreck
state prefix ending before the reward branch; the actual EMP loop; actual
Car.dodge and timer decrement statements. It invokes no renderer or network.

Shots update newest to oldest. Acceleration updates scalar speed before homing.
Live targets guide only while in front unless always=true. A live target behind
a non-always shot leaves velocity unchanged even though scalar speed increases;
a missing/dead target lets acceleration change velocity length. Finished-but-alive
targets still guide. Zero velocity/zero separation retain source normalization
behavior. Motion then decrements lifetime, tests the travelled segment against
eligible racer spheres with strict distance-squared < radius-squared, and selects
the first intersecting roster entry, not the nearest along the path. Owner,
dead and finished racers are excluded. Zero-length segments use denominator1.

Racer hits suppress world queries; otherwise the caller world-segment callback
precedes the strict endpoint y<floor check. Racer/world hit evaluation precedes
expiry, so a shot reaching life<=0 may still hit that frame. Trail timer subtracts
dt and requests a trail before impact/removal; its cadence resets to0.022, with
at most one request per frame. Synchronous impact callbacks allow damage/wreck
to affect eligibility of the next older shot. Survivors remain in insertion
order through stable compaction. EMP purge walks newest first, excludes owner's
shots and includes the exact radius boundary. Clear resets caller shot count.

Damage rejects dead/finished/non-race victims; a roll timer>0 dodges before HP or
knock. Applied damage subtracts HP, multiplies scalar speed and raises stun to
the kind threshold: bolt0.985/0.12, missile0.5/1, EMP0.65/1.6, generic0.6/0.7.
Unspecified shot damage uses current source30 on player /51 on rival; explicit
damage0 remains0. Lethal damage sets wreck2.4, HP0, visibilityfalse, speed0 and
velocity0. No attacker/self exclusion is added to direct damage; self exclusion
belongs to the projectile roster test. Source local damage has no additional
spawn-protection gate; free-roam protection/network damage is a separate system.

EMP iterates source roster order, excludes player/dead/finished actors, includes
distance95, delays rival fireCD by3, and adds outward velocity
(30*(1-distance/95)+8)*normalized_delta before damage25*(1-0.5*distance/95).
At zero separation the direction is zero. Rolling actors receive impulse/fire
delay but dodge damage. Wreck results are immediate before the next actor.

Existing rr_car_dodge is reused unchanged and freshly checked against source:
roll0.55, surge2, minimum launch speed47.5 times1.5, signed22 sideways impulse,
strict roll timer>0 protection, timer boundaries and placement clearing roll/surge.
Full motion/render/dodge coupling remains a later integration qualification.

## Fresh evidence and corrected first implementation

Reference cases:1600 damage states,8 default/explicit damage checks,64 EMP rosters,
150 dodge/timer cases and308 projectile/purge scenes. Projectile fixtures include
strict sphere boundaries, self/dead/finished exclusions, roster ordering, simultaneous
world/expiry hits, zero segments, live/dead/behind homing, immediate lethal callbacks,
floor boundaries and purge ownership/radius. Fresh results: 316 ordered
events, 228 world queries and 32 purged shots.
Maximum measured fixture difference: 0, tolerance1e-10.
Native/WASM functional summaries match; native shot storage144B,
WASM136B. This finite fixture result is not a universal
floating-point/near-boundary guarantee. World callbacks in this new suite are
controlled hit/miss fixtures; full scenery geometry coupling is not qualified.
The preexisting collision suite passed separately.

An initial C EMP implementation incorrectly added stun2 before damage. Actual
source-equivalence tests caught it among442 assertions (112 EMP numeric failures
plus330 invalid-large-timestep fixture assertions, max difference2).
The source contains no such extra stun; it was removed from C without changing
the source fixtures. The initial native/WASM31-pass/1-fail logs and initial C/header
are preserved under verification-20261103/combat-initial-*-emp-failure.log and
pre-combat-emp-correction-20261103. combat-emp-correction.json records the correction.
The remaining330 assertions came from passing a duration>0.05 as a single tick
to existing rr_car_step, which intentionally rejects it. The timer fixture now
uses identical repeated<=0.05 ticks on both actual source statements and C, with
no weakening of the production guard and no preexisting core edits. Originals
and rationale are in pre-combat-bounded-dodge-fixture-20261103 and
combat-bounded-dodge-fixture.json. Final complete32/32 logs are separate.
Initial adapter execution also encountered
the browser texture loader's missing document in Node; the adapter now supplies
a texture sink while executing the actual class body, without browser/source edits.

Previous4320-tick/nine-road pack, targeting and4324-case arsenal summaries match
their preserved prior target results exactly. No production loop calls this module.
Caller owns initialized shot storage/roster pointers; no pool cap or allocation is
invented and no production memory budget changes. Synchronous callbacks must not
structurally mutate the shot array. Wreck flag signals future host lock release,
FX and other integration; no takedown/reward, boost/missile refill, coin transfer,
wallet, progression, persistence or account operation is performed here.

## Important unqualified integration

The numerical projectile/damage/EMP slice makes no random draws. Source graphics
impact/trail/explosion sprites do consume RNG; those effects are sinks here and
their random consumption/FX capacity limits are not claimed ported. Ordered event
callbacks retain the point at which future host effects must execute. Existing
arsenal/rival RNG and fixtures are unchanged. Full shared RNG equivalence requires
that remaining visual effects path before production integration.

Projectile fire/default initialization, muzzle/quaternion/lead setup, rival firing,
player input/session/pack integration, lock-release handling, full world coupling,
audio/FX, takedown semantics and integrated lifecycle acceptance remain open.
No free-roam/network or account policy was inferred. This is not whole combat or
human race acceptance. No product decision was required for the bounded state slice.

Deliverables: src/core/rr_combat.h/c; tests/test_combat.c and generated reference
header; tools/capture-combat.mjs WIN_APP_ROOT OUTPUT.json and
tools/prepare-combat-reference.py STAGING_DIRECTORY; reference/combat-reference.json,
combat-results.json and verification-20261103/combat-builds.json with full native/
WASM thirtytwo-suite logs/checksums. CMake originals are in pre-combat-20261103.
All earlier source/assets/evidence hashes, native copied canonical meshes and Web
preload were verified preserved, including finish screenshot/Library receipt.

No graphics launch, desktop/approval retry, process query, active-log read or
termination occurred. Old window absent does not prove process shutdown: Task
Manager app-access approval timed out and shutdown remains unverified. Await fresh
owner/process evidence or supported approval and verify old process stopped before
a replacement launch. Poster/GPU/visual gates remain open; no new culling work or
city budget change (413 canonical /1947 missing source-grid towers).
