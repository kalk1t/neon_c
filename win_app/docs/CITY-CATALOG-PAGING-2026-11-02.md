# Experimental city catalog and bounded paging - 2026-11-02

Completed a **headless-only** source catalog, deterministic paging model and
city-collision equivalence tests. It is compiled only into rr_city_catalog_tests,
never rr_core or RoadstersRace. No runtime city coverage, geometry, material,
collision, account/progress or resource cap changed.1897 grid towers remain
unrepresented by runtime assets. This is not whole-city or playable-world parity.

## Full metadata coverage and explicit missing geometry

Browser working-tree package0.2.3, capture 2026-11-02T21:10:50.663Z; exact source
hashes in reference/city-catalog.json and build receipt, rechecked after builds.
Actual buildCity through grid/ring/skyways ran with unchanged full RNG and original
transforms/OBBs. Catalog contains2360 grid towers,380 ring towers,one accepted
skyway and all2741 corresponding colliders in original insertion order, plus450
separate crown anchors. Exact transformed Float32 body/trim/poster AABBs and
1695222 source vertices are represented by metadata; no new runtime mesh was baked.
Hero geometry,traffic,other props/course and full source materials are not modeled
by the body pager. All body records retain colliders regardless of visibility.

413 catalog entries map to existing native mesh IDs.97 of those omit source
poster-plane geometry (a previously documented slice limit); only316 have complete
body/trim/poster **geometry** for this model. Even those316 do not imply full
material/lighting parity. Missing poster geometry and1897 unrepresented grid towers
are surfaced, never treated as complete uploads or silently removed from catalog.

Browser remains full CPU city construction,45 merged material/frustum batches
with lazy first-visible GPU upload/caching, not sector paging. This experiment
instead uses finer per-structure AABB-derived sphere culling. The108 views are
12 positions/tangents per nine browser roads with fov70,aspect4:3,near.3,far1000
to study the current bounded native range. They are not captures of actual browser
draw batches or its22000 far plane. Actual Three.Frustum sphere results supply
the expected visible sets/hashes. Crown anchors are cataloged, not paged here.

## Deterministic planner and transactional scheduler

Planner reports every logical visible body, unavailable geometry and budget
shortfall. It sorts nearest first with stable source-ordinal ties, at most64
records and141312 source vertices (64*2208, the current mesh-derived bound).
It cannot declare a view complete unless all visible geometry is available and
fits. The scheduler models eviction and at most two additions per step, with
64 resident records/vertex accounting. Incomplete/invalid/duplicate plans leave
the cache unchanged. Synthetic scheduler fixtures use actual available source
records, not fabricated unlocks or a claimed usable camera view. No file/GPU IO
or player-motion/loading behavior was installed; the module cannot make runtime
structures invisible or remove their collisions.

City collision queries gather **all** source candidates for the original120-unit
query cell, in catalog order, into the existing1024-shape scratch-world type before
resolving. Overflow/invalid input rejects the query without moving the point;
there is no silent truncation. All city-only cell populations are at most six.
This is experimental equivalence, not production full-world collision: course,
props,actors,weapons/long segments/respawn unions and real streaming transitions
must be integrated/tested before activation. Do not install invisible collisions
ahead of their missing geometry.

## Fresh validation, resources and measured limitation

Full isolated native Release and WASM builds each passed **18/18** headless suites.
New suite validates2741 body/collider records,450 crowns,5482 actual full-city
browser collision queries (two per record, worst error1.11e-16,max candidates6),
108 actual Three.Frustum sphere visible sets, repeated deterministic planning,
64-record/141312-vertex/two-addition bounds, rejected-plan atomicity,invalid inputs,
duplicate protection and retained/evicted scheduler state. Seventeen prior suites
reran fresh, including source facade/loader and current nine-road/runtime-city tests.
No regressions were detected by these tests; graphics/performance remain unrun.

Read-only catalog metadata is610056B **in the test binary only**. Existing runtime
app source/linkage and asset bytes are unchanged. The runtime remains679/700
planned buildings,991/1024 maximum active shapes,64 resident building meshes,
two mesh uploads/frame,64 building/16 furniture/4 crown draws,256 dots,800x600
world,960x720 window,30FPS,MSAA/glow off and focus/overload pause. No runtime cap
increase or hidden coarser/fewer geometry representation was applied.

Offline budget analysis recomputes all108 fixture visibility sets and matches
their Three counts. Maximum235 logical structures and171546 body vertices,
6175656B RGBA attributes. All108 views exceed64 per-structure records;21 exceed
141312 vertices. All108 current plans correctly block as incomplete (also missing
assets/posters); worst171 budget-skipped bodies and235 unavailable bodies. These
are conservative fine-grain sphere working sets, excluding crowns/traffic/props;
not GPU timing or a proof that the final required visible triangles need that
budget. They demonstrate this per-building sphere model cannot be wired into
runtime as a complete pager under present bounds.

## Smallest decision and safe next work

Do **not** enable this diagnostic nearest64 selection as a full-city renderer.
The safe next step with unchanged caps is offline tighter source AABB/submesh
culling and exact spatial/material batching analysis, while retaining every source
triangle and reporting any remaining coverage shortfall. Batching may reduce record
count; it must also account for bytes/vertices rather than hiding larger meshes
behind a64-count limit. Prove full collision candidate unions and paired geometry
coverage before any runtime catalog/physics integration.

If tighter faithful visibility/batching still exceeds the bound, the smallest
owner performance decision is to retain the current bound/explicit coverage limit
or authorize a measured higher vertex/byte budget after controlled profiling.
For this conservative body-only model,171546 vs141312 vertices is about21.4% more
attribute exposure (6175656 vs5087232B per copy), before other assets. No such
increase, fidelity reduction or profiling was authorized/applied here. No decision
is needed to continue the proposed independent headless analysis.

Whole-scene PBR/normal/roughness/environment/ACES/bloom,source anisotropy8,poster
assets,crown/landmark readability,full race,Web graphics/performance and the1897
unrepresented runtime grid towers plus ring/skyway/traffic remain open.

## Evidence, files and unresolved window

reference/verification-20261102/city-catalog-builds.json records build/artifact
hashes, metadata/resources and preserved evidence; city-catalog-{native,wasm}-
eighteen-tests.log contains the fresh suites. reference/city-catalog-budget.json
contains all108 working sets; tools/analyze-catalog-budget.py reproduces them.
New code/data:src/experimental/rr_city_catalog.{h,c},generated_city_catalog.h;
tools/capture-city-catalog.mjs;tests/test_city_catalog.c/generated reference;
reference/city-catalog.json. Only existing CMakeLists.txt and the headless-check
CMake file changed to add the separate test target; originals are backed up in
reference/pre-city-catalog-20261102. No game source or runtime asset was changed.

Window3148188 remains unresolved from the prior camera trial. No desktop input,
denied active-log read,launch or termination was retried. Its executable/bin assets
remain untouched. Earlier facade clean shutdown,blocked second trial,loader and
city-sector evidence remain separate and hash-checked. No graphics qualification,
screenshot or upload occurred. Locks/progress/Claude-owned browser files remain
unchanged; all project writes stay within win_app.
