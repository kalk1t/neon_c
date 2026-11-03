# Browser city loading and bounded native loader - 2026-11-02

Implemented a bounded loader improvement and audited the actual browser loading
model. No sector/geometry/material/lock/progress/account changes, cap increases,
graphics launches or denied desktop/log retries.413 of2360 source grid towers
are represented; **1897 remain outside the tile**. Whole-city parity is open.

## Actual browser model and measured source content

Working-tree package0.2.3, audit 2026-11-02T21:00:38.780Z; exact seven source hashes
in reference/browser-city-loading-audit.json, renderer support hashes in the
build receipt. All rechecked after builds; no commit/deployed-release claim.

main.js/buildWorld constructs all city geometry/colliders once around all nine
roads. Changing roads swaps race furniture rather than paging the city. Map
rebuild traverses/disposes prior geometry/material/textures and clears the map.
city.js merges material lists into chunks of900 geometry parts, computes bounding
spheres and adds regular meshes. These are frustum draw batches, not spatial
residency sectors. Perspective camera far plane22000. The installed Three renderer
tests object.intersectsFrustum before objects.update: GPU attribute upload occurs
when a mesh is first rendered; uploaded buffers persist until geometry dispose.
Thus full CPU construction with lazy first-visible GPU upload/caching should not
be mistaken for distance-based loading/eviction. Current source has no such pager.

skyprops.js/loadModel uses GLTFLoader.loadAsync for sculpted replacements, keeping
fallbacks until success and warning on failure. City crowns use one450-instance
mesh with a combined bound; no per-crown residency manager. Traffic uses a global
InstancedMesh with frustumCulled=false. These assets/traffic are excluded from the
geometry totals below and remain additional resource/visual parity work.

The CPU-only audit executes actual buildCity through grid, distant ring and
skyway construction with a topology-count sink (no renderer/network/GPU or source
edits). It counted2360 grid towers,450 crown anchors,380 distant-ring buildings,
one source-accepted skyway,2741 city colliders and45 merged material batches.
Total1695222 non-indexed vertices,54247104B position/normal/UV attributes (32B each).
Largest source batch45900 vertices. Native RRC1 RGBA adds4B/vertex, so the same
complete topology would need61027992B attributes before allocator/driver overhead,
textures,sculpted geometry,traffic and any CPU/GPU duplication. These are payload
estimates, not measured allocations or a claim that every batch draws each frame.

The source city-only120-unit collision grid has2692 populated cells and at most
six city shapes in any cell. Browser queries choose the original query cell and
retain insertion order through pushes; collider residency is independent of draw
frustum. Course/props/racers/segment traces are not included in this six-shape bound.

## Implemented guard and fresh verification

Before raylib LoadFileData, the loader now rejects reported file lengths outside
the existing100000-vertex RRC1 budget or incompatible with a canonical asset's
expected count. The actual returned payload is revalidated. This preflight bounds
normal asset reads but is not an atomic-file guarantee against concurrent writers.
Canonical load then requires exact expected count and source Float32 world AABB
before CPU mesh allocation/VRAM upload. A valid-format mesh displaced into another
building no longer passes merely because all numbers are finite. Existing missing/
invalid handling and explicit silhouette fallback remain; no substitute is counted
as canonical fidelity. Bounds/count matching is not cryptographic content identity;
offline source/asset SHA256 checks remain separately recorded.

Full native Release and WASM builds each passed **17/17** headless suites. The new
suite uses an existing3896B canonical asset fixture, accepts the correct payload,
rejects a finite-but-displaced valid-format payload, wrong count, NaN/inverted
bounds, null expectation and overlarge/misaligned sizes. It tests the production
validator used by rendering. Additional native command
`build/loader-guards-native/rr_mesh_loader_tests.exe assets/city-core`
validated all413 installed city assets with0 failures. No repeat solely to recreate
stdout; the successful combined tool invocation/result is recorded in the receipt.
All413 source and staged-native mesh hashes and prior facade/east/west evidence
were rechecked unchanged. Sixteen earlier suites reran fresh, including facade
pixels,1239 city collision queries/18052 route samples and115 crown matrices.

No new runtime asset,texture,draw call or mesh allocation was added. Canonical
raw file preflight is at most79496B with current2208-vertex maximum; generic RRC1
nominal file cap3600008B (largest legal triangle count99999 =>3599972B). The current
64-mesh/2-upload caps,679/700 planned buildings,991/1024 maximum world shapes,
64 building/16 furniture/4 crown draws,256 dots,800x600 world,960x720 window,
30FPS,MSAA/glow off and focus/overload pause are unchanged. GPU behavior/performance
was not qualified. Existing pinned raylib WASM warnings remain.

## Concrete bounded continuation plan; no hidden budget change

1. Build a read-only full-source catalog and cell index separately from active
   RRWorld/draw residency. Test browser insertion-order/original-cell equivalence,
   borders,respawns,fast segments and all-road course/prop unions. Six city shapes
   per cell suggests query-local working sets could fit unchanged1024 active slots,
   but this must be proved including all other colliders. This is a proposal,
   not implemented full-world collision. Never add invisible distant collisions
   while their visual coverage is still absent.
2. Catalog all source geometry without treating catalog size as mesh residency.
   Page exact source geometry through the existing64 mesh/two-upload scheduler,
   adding explicit byte/vertex accounting, prefetch and eviction. Preserve source
   fits/materials and diagnose unmet visible coverage instead of hiding it with
   arbitrary geometry removal or unannounced LOD.45 browser batches fit a draw-count
   number but contain about12x the current141312 worst-case building-resident
   vertices; substituting them would silently change memory/triangle exposure.
3. First compute bounded visibility/working-set traces offline, then perform
   controlled profiling only after the existing desktop blocker is resolved.
   Full payload totals alone do not prove a particular visible frame exceeds the
   present budget. No owner hardware tradeoff is necessary for these loader/catalog
   checks. If faithful visible sets exceed it, the smallest concrete owner decision
   is whether to approve a measured higher building vertex/byte budget or retain
   the current bound with the coverage limitation explicit. A complete-resident
   alternative would require reviewing at least61.0MB native geometry attributes
   plus any CPU/GPU copies and other assets; it is not authorized or implemented.

Remaining sectors are still north/south/west/east outside gx17..47/gz21..44;
1897 grid towers,380 distant-ring towers,remaining skyway/traffic/poster/start art,
whole-scene PBR/normal/roughness/environment/ACES/bloom/anisotropy8 and crown/
landmark readability/manual-race/Web-graphics/performance acceptance remain open.

## Evidence, changed files and blocker

Tests/artifact sizes+SHA256s and source support hashes:
reference/verification-20261102/loader-guards-builds.json and
loader-guards-{native,wasm}-seventeen-tests.log. Original code backups:
reference/pre-loader-guards-20261102. Changed:src/core/rr_mesh_asset.h,
src/presentation/rr_scene.c,CMakeLists.txt,tools/race-rules-check/CMakeLists.txt.
Added:tests/test_mesh_loader.c,generated_mesh_loader_fixture.h,
tools/audit-browser-city.mjs,reference/browser-city-loading-audit.json plus receipts/docs.

Graphics remain **unrun**. Window3148188's current state is unresolved after the
prior input-protection block; its active-log read had PermissionError13. Neither
action was retried, no new instance launched, and the older facade-windows-view
executable/bin assets were untouched. Prior first-view clean shutdown remains
separate from the blocked second trial. No new screenshot/upload was made; existing
Library receipt and finish fixture hashes remain unchanged. All writes stay in
win_app and Claude-owned browser files are read only.
