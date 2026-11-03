# Offline player arsenal resource and trigger parity - 2026-11-03

New shared C rr_arsenal.h/c ports the resource and trigger portion of browser
playerWeapons. Full native and WebAssembly applications compile in isolated
arsenal output folders; both headless suites pass **31/31**. Production combat,
rivals and coins remain inactive. No projectile, damage, EMP target effect, dodge
motion, audio, account, wallet, progression or network operation was activated.

## Exact source behavior covered

The working-tree browser package0.2.3 reference is pinned by SHA256 of main.js,
ai.js, car.js and package.json in reference/arsenal-reference.json. The capture
executes the actual playerWeapons function and actual startRace resource-reset
statement with controlled effect sinks. It does not reproduce these rules in a
second reference implementation, start a renderer or contact a service. Target
aim/muzzle math and effect delivery are intentionally sinks in this slice.

The C stage executes bolt -> seeker -> spin -> EMP in source order. Bolt cooldown
subtracts dt without clamping; source fires at most once per call and resets to
0.09 rather than catching up missed shots. Cooling happens first at0.45 times
dt and factor1.4 while overheated /0.35 while holding fire /1 otherwise. Recovery
at heat<=0.25 can permit a shot in the same call. A shot adds0.06, clamps at1 and
emits overheat. Each bolt flips the cannon side and requests exactly two random
values for unnormalized X/Y spread (value-0.5)*0.008; no extra draw occurs on a
cooldown or overheated frame.

Seekers refill before checking the input edge, at dt/4.5 capped at3. A charge>=1
fires and subtracts1, otherwise emits deny. Homing requires an existing target
and lock charge>=1; dumbfire remains allowed. Spin cooldown decrements/clamps
before its input edge, resets to6 on acceptance, and chooses steering sign or
one random side (<0.5 left, >=0.5 right) only when steering is zero. EMP cooldown
likewise decrements/clamps before its edge, denies while positive and resets to11
on acceptance. It emits a trigger only; purge, push and damage are not applied.
Input edge detection belongs to the host, matching source input.hit; held bolt
fire is a separate input. Host race/alive gating remains required.

Events retain source order, including deny and overheat among simultaneous
requests. The random callback is demand-driven in source order: two bolt draws
then optionally one unsteered-spin draw. No RNG is needed for quiet/blocked
calls. Reset restores heat0, overheatfalse, boltCD0, missiles3, spinCD0, empCD0.
Restart preserves alternating cannon side, matching startRace's omission of
g.side; fresh initialization sets side1 from the initial browser g state.

## Fresh verification and bounds

There are4324 source cases:720 synthetic timing/resource boundary probes,
3600 persistent stream frames (four streams, with four midstream restarts), and
four explicit restart/initialization probes. They exercise held/released fire,
cooldown equal/below/above dt, overheat/recovery, recharge before spending,
simultaneous requests, exact lock threshold, steering/random branches and large
dt source no-catch-up behavior. Synthetic boundary states are analytical probes,
not a claim that every state occurs during normal capped frame play.

Across these cases:4316 playerWeapons calls, 1727 requested RNG values,
2870 ordered trigger events, 39 overheat cues and
908 denied requests. Native/WASM fixture summaries are identical;
maximum measured numerical difference is 0 with1e-12
tolerance. Tests also verify no RNG dependency on a quiet call. This bounded
fixture result does not establish universal floating-point boundary identity.

The prior targeting summary and complete4320-tick/nine-road local-pack summary
match their preserved earlier results exactly on each target. The new module is
not coupled into that pack or the player/application loop: this validates the
isolated resource/trigger stage, not complete combat ordering or race acceptance.
Its future host location remains after ranking/lock and before projectile update,
with the existing explicit late-coin tail after all weapon/projectile effects.

The state is 48 bytes; event output is 64
bytes on both targets. Fixed maximum6 event slots, no heap allocation, textures,
uploads, world shapes or draw calls. Existing resource caps/safety settings,
app/presentation code, all preexisting core code and assets are unchanged. Native
copied canonical assets and Web preload hashes were checked. All prior preserved
verification files, finish screenshot/Library receipt and desktop evidence hashes
were checked; no duplicate upload or existing artifact replacement occurred.

## Deliverables and remaining gates

New src/core/rr_arsenal.h/c, tests/test_arsenal.c and generated_arsenal_reference.h;
tools/capture-arsenal.mjs WIN_APP_ROOT OUTPUT.json and
tools/prepare-arsenal-reference.py STAGING_DIRECTORY (containing the JSON);
reference/arsenal-reference.json and arsenal-results.json. CMake files add only
the core module/test target. Originals are in reference/pre-arsenal-20261103;
documentation originals are in reference/pre-arsenal-docs-20261103. Full logs and
artifact checksums: reference/verification-20261103/arsenal-builds.json and the
native/WASM thirtyone-suite logs. Current-source hashes were rechecked after build.

Next true gameplay gap: consume these outputs through source-defined car/dodge,
projectile and damage behavior, including target eligibility, spawn/spin protection,
collision/lifetime, EMP effects and timing. No firing effects, resource state or
new rivals/coins are wired into production yet. Player/session/input bridge,
graphics/audio, human race acceptance and account/network integration stay open.
No product choice was necessary for this resource/trigger-only slice.

No graphics launch, desktop/approval retry, process check, active-log read or
termination occurred. Previous window-absent/process-unverified status remains:
Task Manager app-access approval timed out, so shutdown is unverified. Await fresh
owner/process evidence or supported approval and verify old intended process has
stopped before any replacement launch. Graphics/texture/GPU measurement remain
blocked on that gate, not headless gameplay work. City coverage remains413
canonical meshes and1947 unrepresented source-grid towers; no new culling work.
