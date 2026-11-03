# Canonical eastern city extension - 2026-11-02

Four source grid columns44..47 extend the fixed canonical tile eastward from
x1067 to1395 (now gx20..47/gz21..44,28x24 cells; z[-1019,949), minX-901).
This replaces21 additional prototype buildings with68 source buildings and
19 source crown anchors. Totals:363 canonical buildings,287 prototypes,650
planned buildings and105 crown anchors. No source building was moved or removed
for visibility. The prior fixed-camera landmark obstruction remains unresolved.

## Reference and unchanged content

Browser package0.2.3, source capture20:45:21Z; content identity is in
reference/canonical-city-core.json, not a commit/deployed-release claim.
Original full-grid RNG/branch execution is retained, with actual city.js
seed1337/BASE-260/STEP82 source geometry and collider basis. All ten source/asset
hashes were rechecked after builds. Every previous295 mesh is byte-identical;
their collider/bounds rows and86 crown placement rows are unchanged. New meshes
append b0295..b0362, preserving previous indices. dressScenery generated all105
crown fixture matrices using the original asset bounds/source fits.

Canonical body geometry keeps the source facade UVs and existing shared window
material. No facade shader, window pixels, crown assets, locks, profile/progress
or Claude-owned browser files changed. Source changes are isolated from the
previous camera build: build/facade-windows-view executable/bin/assets were not
rebuilt, replaced or modified. New native/WASM outputs use canonical-east-*.

## Fresh headless evidence and limits

Full native Release and WASM builds each passed **16/16** suites, including
the facade pixel suite. The expanded city suite now runs1089 actual browser
collision queries (three per each363 source building),18052 route sample
spheres radius2.1 with0 city contacts,105 source crown matrices, replacement
boundary/preservation and all-nine-road world capacity checks. Native maximum
collision numeric error4.44e-16; Float32 crown matrix error6.05e-05. No continuous
path/collision guarantee follows from sampled coverage. Placement tests ran
fresh; earlier facade graphics evidence is reused only for that earlier build.

Maximum combined collision world962/1024 (62 slots remaining);650/700 planned
buildings (50 remaining). Maximum per-building vertices remains2208;64 resident
meshes still bound worst attribute storage to5087232B each CPU/GPU copy excluding
overhead.68 added meshes total1580368B (43884 vertices); all363 city meshes now
8670552B/240768 vertices. Crown draw cap4 and all existing upload/draw/30FPS,
world/window resolution,MSAA/glow and focus/overload safeguards remain unchanged.
No performance/memory allocation/graphics stability claim is made from headless tests.

**No graphical test was run for this slice.** Window3148188 was last observed
at the prior camera-build road picker; its current state is unresolved. Parent
reported owner notification. No fresh authority/input-idle evidence arrived,
so denied desktop actions and the PermissionError13 active-log read were not
retried. No duplicate launch, termination or safeguard bypass. The previous
facade16/16 logs and first normal-view clean-shutdown evidence were hash-checked
unchanged and remain separate from the blocked second camera trial.

Receipts/test logs/artifact sizes+SHA256s: reference/verification-20261102/
canonical-east-builds.json and canonical-east-{native,wasm}-sixteen-tests.log.
Installation mapping/prefix preservation: reference/canonical-east-install.json.
Earlier finish Library receipt/fixture hashes remain unchanged; no upload occurred.

## Remaining work and exact changed paths

Outer city beyond this tile, prototype transition seams, distant ring, skyways,
traffic/poster/start art, whole-scene PBR/lighting/ACES/bloom and anisotropy8
remain open. Crown/landmark readability, manual complete race, Web graphics
and performance acceptance remain open. Further city expansion must measure
the remaining50 building/62 collision slots before installation; full source
world coverage will need bounded world streaming/partitioning rather than
silently increasing the existing caps. No product/account decision was needed.

Modified six files: tools/capture-canonical-city.mjs,
src/core/rr_canonical_city_data.h, tests/test_canonical_city.c,
tests/generated_canonical_city_reference.h, tests/generated_city_crown_reference.h,
reference/canonical-city-core.json. Added68 assets/city-core/b0295..b0362.rrmesh
and installation/build/report evidence. Originals preserved under
reference/pre-canonical-east-20261102; historical reports/builds preserved.
All writes stay inside win_app. Graphics reconciliation remains pending with owner.
