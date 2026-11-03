# Browser collision shapes and scoped solid world — 2026-11-02

The largest independent W2 gameplay gap was flight through buildings and course
structures. Native sessions now resolve sphere contacts with city tower bodies,
the start/deck/control-tower structures and selected-road checkpoint hoops,
pedestals and columns. Collision is applied inside the car step, before attitude
and gate/lap rules, including countdown and post-finish/results motion.

## Reference, implementation and visible change

Current browser package 0.2.3 is identified by seven hashes in
`reference/collision.json`, checked before/after capture and after full builds.
Actual browser OBB/Cyl/Torus/Colliders and Car methods execute CPU-only; start
and checkpoint collider declarations are extracted from actual course.js.
No DOM renderer, account, storage, backend or deployment operation runs.
No commit identity is claimed: Git history was unavailable earlier.

New pure C physics implements Y-rotated boxes, vertical cylinders, vertical
hoops, strongest contact, ordered push-out and 3-unit projectile segment sampling.
It retains the browser's original query cell (120 units), insertion order,
strict boundaries, off flags, tie behavior and reflection/speed rules. A bounded
1,024-shape allocation-free list filters by cell bounds rather than allocating a
Map/grid. For these bounded world coordinates this returns the same cell list;
it scans all shapes each query, so future larger worlds need profiling/indexing.

Current Skyline has 437 shapes: 404 native prototype city bodies, nine start
structures, 24 checkpoint shapes. Maximum current route: 470. Road start/restart
rebuilds this world, so previous-road rings do not accumulate. Session start
clears its optional world pointer; the app supplies the newly rebuilt world.
An absent context preserves all prior collision-free numerical fixtures.

The city uses its existing prototype body positions/base/dimensions, not the
complete browser city layout. Body colliders omit cosmetic roof overhang/neon,
consistent with this scoped body policy; do not claim complete mesh collision.
Start deck/gantry and ring pedestal/column simple rendered volumes now use the
browser collider envelopes instead of the old narrower/lower native approximations.
The start underdeck/legs and simple control-tower volumes are now visible too,
avoiding invisible solid objects. This matches structural dimensions, not the
complete beveled/textured browser prefabs. Existing culling/building upload and
30 FPS/800x600 renderer limits remain unchanged.

## Explicit finite-state exception to reference

Five captured browser shape cases produce non-finite torus normals. Exactly at
the hoop center, its zero radial vector yields a false contact and 0/0 normal;
the native guard leaves that hole point clear. Exactly on a tube centerline,
the normal is undefined; the native guard supplies an axial unit normal with
the same penetration depth. All captured finite reference contacts, including
other axis points, retain browser behavior. These are narrow numerical guards,
not a claim of byte-for-byte parity for invalid JS normals. Cylinder axis zero
normals remain as observed where finite; no general collision redesign occurred.

Initial staged tests failed at the exact hoop centers (117 route gates) and
one test setup incorrectly left a second overlapping collider enabled. The
first failure log is preserved in `reference/collision-first-headless-failure.log`.
The finite guards and disabled-world fixture correction resolved those failures;
the manifest still counts and exposes the five invalid browser cases.

## Fresh verification

Full MSVC native and Emscripten 6.0.13/raylib WebAssembly applications build in
`build/collision-native` and `build/collision-web`. Both pass **11/11** suites:
1,344 shape cases, 400 ordered world/cell queries, 120 projectile segments,
720 actual browser Car collision steps and 432 actual start/checkpoint shapes
across all nine roads, worst finite numeric error 3.87e-12. The five non-finite
reference cases have separately asserted finite native behavior, not numeric
equivalence. Tests also check clear hoop centers/solid rims, off flags, repeated
road world rebuild, and collision context during countdown/finished states.
Prior flight, race, lifecycle, recovery, picker, city and finish suites reran
on the new core; their standalone race fixtures do not themselves certify full
two-lap flight through the newly solid world. Continuous/swept collision is not
added: endpoint sampling/tunneling limits match the current browser approach.

Known upstream Web ES3 override and stb_vorbis warnings remain; linking/tests
passed. No Web graphics run occurred. Receipts and exact artifact hashes:
`reference/verification-20261102/collision-builds.json`.

Controlled native visual fixture: `build/collision-visual-fixture`,
18:35:13Z–18:35:46Z, window 8262370. The opt-in
`RR_COLLISION_VALIDATION` flag defaults OFF and is labeled in title/log/watermark.
Observed normal menu, locked picker (only road 1 open), resized start complex,
countdown and auto-pause after a scripted start-post approach. No smoke/fallback
flags. Log records world 437 shapes, GO at 18:35:29Z, then impact 59.967856,
x=20.500000, vx=-23.987142 and speed=6.000000, establishing push-out/rebound/speed
cap in the actual native integration. The paused screenshot is supporting UI
evidence, not a visual numerical measurement or a human-driven impact course.
Closed only the fixture, verified its window absent and clean-shutdown log.
No physical stop/input-protection rejection occurred. Loading/menu heartbeats
include about 159ms, so sustained performance/freeze resolution remains unproven.
Original log: `reference/graphics-20261102/collision-fixture-183513-original.log`.

Prior 8/8 control, 9/9 city and 10/10 finish/scripted result evidence remain
separate and unchanged. Prior interrupted trials remain interrupted.

## Existing screenshot delivered

The already inspected, unchanged `finish-final-results-fixture.png` was uploaded
successfully through the native Library create-file tool. It remains visibly
labeled SCRIPTED GATES / NO PROGRESS; the 0.600-second total is artificial.
No image was regenerated, no sharing/publishing action occurred, and the local
SHA remains cac9964e7873c6428340766453c3235d4de98e866bf1817f8ed9be47bc1c0613.
Library path: `/finish-final-results-fixture.png`; stable identity and upload
receipt are in `reference/finish-screenshot-library.json`.
The bundled metadata helper failed because Windows Python lacks os.setxattr.
Thus upload succeeded, but Library extended attributes were not persisted on
the local file. The returned identity is retained in that receipt; no alternate
transfer/bypass or duplicate upload was attempted. This metadata limitation does
not mean the Library upload failed.

## Current gaps and smallest decision

- W2: remaining browser course beacons/barriers/signs/platform collider layouts,
  full browser city/skyway layout, car-to-car contacts and manual full-route race
  acceptance with the now-solid world. The tested projectile segment is not yet
  connected to weapons, since combat is unported.
- W3: rivals/rankings, combat/health/effects/coins, full car/shop/cockpit/art,
  audio and broader mouse/gamepad/mobile/settings qualification.
- W4/W5: same-UID identity, profile/persistence, trusted server rewards/shop,
  and cross-play/network behavior. W6 packaging/performance gates remain open.

No product choice is required to continue independent W2 course/art work.
Before account writes, a coordinated desktop sign-in/token/App Check approach
and account-versus-device local data scope still need resolution. Those do not
authorize backend or security changes; no new choice was guessed here.
Road locks are preserved (only road 1 selectable), fixtures do not grant progress,
and no account/profile/storage/reward writes, paid calls or publishing occurred.

## Exact changed paths

Modified: `src/core/rr_flight.c`, `src/core/rr_flight.h`,
`src/core/rr_session.c`, `src/core/rr_session.h`, `src/app/main.c`,
`src/presentation/rr_scene.c`, `CMakeLists.txt`,
`tools/race-rules-check/CMakeLists.txt`, `README.md`, `PROJECT_DIRECTION.md`,
`PLAYING.txt`, `docs/PARITY.md`.
Added: `src/core/rr_physics.c`, `src/core/rr_physics.h`,
`src/core/rr_world_plan.c`, `src/core/rr_world_plan.h`,
`tools/capture-collision.mjs`, `tests/test_physics.c`,
`tests/generated_physics_reference.h`, `reference/collision.json`, initial
failure log, screenshot Library receipt, this report, two eleven-test logs,
build/hash receipt and original graphics log. Originals:
`reference/pre-collision-20261102/`, status docs:
`reference/pre-collision-docs-20261102/`. Earlier builds, screenshot, reports and
Claude-owned files are preserved. All project writes stayed win_app; Doorways
was untouched. No action is denied/pending/uncertain. Only local Library metadata
persistence is unsupported as described above; development is not blocked.
