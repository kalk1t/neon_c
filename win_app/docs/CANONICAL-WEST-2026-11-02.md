# Canonical western city extension - 2026-11-02

Three source columns17..19 replace21 additional prototype buildings with50
canonical buildings and10 crown anchors. Totals:413 canonical buildings,
266 remaining prototypes,679 planned buildings and115 crown anchors. Bounds
are now x[-1147,1395),z[-1019,949), source grid gx17..47/gz21..44 (31x24 cells).
No individual geometry was dropped or moved to satisfy caps or improve a view.

## Source identity, asset IDs and resource limits

Browser working-tree package0.2.3; capture 2026-11-02T20:50:17.463Z. Exact ten source/
asset hashes in reference/canonical-city-core.json were rechecked after builds.
This identifies content, not a Git commit or deployed release. Actual browser
full-grid generation retains original RNG/branches, BASE-260 and STEP82.

Western records precede existing records in original browser generation order.
A stable mesh_id field now separates record/collider order from asset filenames:
the previous363 files/paths remain byte-identical;50 new files append b0363..b0412.
Renderer loads that recorded mesh ID with an invalid-record guard. No collider
reordering was introduced to preserve filenames. Prior collider positions,
dimensions, bases and bounds match exactly; prior105 crown anchors are unchanged,
with only logical indices shifted by the source-order western insertion.
Headless tests check mesh-ID range/uniqueness. Additional offline validation
checks every413 RRC1 payload size,finite position/normal/UV,vertex count and
world AABB against the correct record/mesh mapping and native staged asset hashes.

Added50 meshes:34752 vertices/1251472 bytes. All city meshes now275520 vertices/
9922024 bytes. Largest mesh stays2208 vertices: existing64 resident mesh cap
still bounds worst attribute storage to5087232B each CPU/GPU copy, excluding
object/driver overhead. No extra runtime texture/draw cap/upload allowance.
Maximum city679/700 (21 slots left); all-nine-road combined world991/1024
(33 slots left). Two mesh uploads/frame,64 building/16 furniture/4 crown draws,
256 dots,800x600 world,960x720 window,30FPS,MSAA/glow off and focus/overload pause
remain unchanged. Actual GPU residency/performance was not measured in this slice.

## Fresh headless evidence; graphics pending

Pre-install isolated capacity build passed16/16, then full installed native and
WASM builds each passed **16/16**. Expanded city test covers1239 actual browser
collision queries (three per413 source building),18052 all-route sphere samples
(radius2.1,zero city contacts),115 source dressScenery crown matrices, replacement
boundaries and capacities. Collision numeric error4.44e-16; maximum Float32
crown matrix error6.05e-05. Sampled clearance is not continuous-path acceptance.
The exact source facade131072-pixel/512-window test reran successfully as well.

An initial sandboxed staging CMake compiler probe failed C1902 from its debug
PDB manager. Failure evidence is preserved; a new local staging config using
/Z7 passed without system/toolchain/security changes. Full native also used
that local probe setting. Existing raylib WASM ES3/stb_vorbis warnings remain.

**Graphics unrun.** Window3148188 was last observed at the older facade camera
picker; current state is unresolved. No fresh input-idle/closure authority arrived,
so no desktop action, denied-log read, duplicate launch or termination was retried.
The in-use build/facade-windows-view executable and its staged assets were never
modified. Prior facade16/16 and first-view clean-shutdown evidence, blocked
second-trial record and eastern-slice evidence remain separate and hash-checked.
No new screenshot, upload or graphics acceptance is claimed. Landmark occlusion
remains unresolved; source layout was not altered for a screenshot.

Receipts/test logs/artifact sizes+hashes: reference/verification-20261102/
canonical-west-builds.json and canonical-west-{native,wasm}-sixteen-tests.log.
Pre-install suite and initial probe failure are preserved there separately;
reference/canonical-west-install.json records installation/prefix checks.

## Explicit parity boundary and remaining sectors

413 of2360 accepted grid towers are ported;1897 remain outside this fixed tile.
The full66x66 grid complement is: north rowsgz0..20 and south45..65 for all
gx0..65; westgx0..16 and east48..65 across middle rowsgz21..44. Distant silhouette
ring,skyways/traffic,poster planes/start art and remaining prototypes/seams are
still partial or absent. This is a declared coverage boundary, not silent
geometry removal to pass caps. Full coverage will require bounded world/collision
streaming or partitioning; simply appending all source geometry would exceed
the unchanged700/1024 plan caps. Any further small sector must fit21/33 headroom.

Whole-scene PBR/normal/roughness/environment/ACES/bloom and source anisotropy8,
manual full-race, Web graphics, performance and crown/landmark visual acceptance
remain open. Locks/progress/user data and Claude-owned files are unchanged.

Modified eight paths: tools/capture-canonical-city.mjs,
src/core/rr_canonical_city.h,src/core/rr_canonical_city_data.h,
src/presentation/rr_scene.c,
tests/test_canonical_city.c,tests/generated_canonical_city_reference.h,
tests/generated_city_crown_reference.h,reference/canonical-city-core.json.
Added50 assets/city-core/b0363..b0412.rrmesh plus installation/build/report evidence.
Exact changed-path hashes are in the receipt; originals preserved under
reference/pre-canonical-west-20261102. All writes stay within win_app.
