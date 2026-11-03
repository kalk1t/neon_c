# Exact material repacking and coverage gate - 2026-11-02

Built and tested a **diagnostic-only** repacker preserving source primitive bytes
and five material groups. It solves the sampled near-range draw-count problem
without raising caps, but that subset is not source coverage. **Dependent runtime
integration is stopped**: distance omission and full-source/transition budget
limits remain. No runtime asset,source renderer,collision,account/progress or cap
changed.1897 source grid towers remain unrepresented in the game.

## Source byte fidelity and bounded assembly

Browser working-tree package0.2.3; capture 2026-11-02T21:36:40.397Z. Source hashes in
reference/city-repacking.json were rechecked after builds. The unmodified actual
buildCity CPU scene independently verifies45 batch spheres with zero difference.
The capture normalizes38352 original primitives and preserves all1695222 source
vertices,world transforms,triangle order,Float32 positions/normals/UV and material
identity. SHA256 streams by each of the five materials match the actual browser's
merged geometries byte-for-byte. Canvas raster sinks supply material setup only;
no renderer/window/network/GPU. No shader/pixel/PBR parity follows from this.

The54247104B interleaved source geometry fixture is **test reference data**, outside
assets and never included in the RoadstersRace runtime preload. Native tests seek
and read selected primitive ranges; WASM tests preload it only into their separate
test VFS package. The repacker concatenates complete selected primitive ranges
into five source-material subranges, preserving source order within each material.
It does not merge material identities,decimate,drop triangles within a selected
primitive or change UVs/normals. Byte/vertex/count metadata and guards are validated
before reads. Oversized source requests fail before payload reads; failed reads
return failure and no upload/publish is attempted. Partial diagnostic buffers on
an IO failure are not a ready frame or atomic runtime cache replacement.

Test-only metadata2454528B and plan arrays are not linked into rr_core or the app.
Output capacity is141312*32=4521984B position/normal/UV, plus16 guard bytes; using
the current native RGBA attribute charge would be5087232B at that vertex bound.
The actual near peak76434 vertices is2445888B float attributes /2751624B charged
with RGBA. At most five material draws,under64. No GPU upload code was installed;
two uploads/step remains an unqualified scheduling constraint rather than assumed
permission to upload five each frame.

## Exact request accounting: frustum rejection is not distance omission

264 representative poses from the preceding analysis are reused:216 route and48
free-roam/landmark poses,near.3,fov70,aspect4:3. Each has two frusta:source22000 far
and native1000 far. Actual source CH900 bounding-sphere visibility gates each
primitive's parent batch. A source geometry request then refines that set using
actual Three.Frustum.intersectsBox on its primitive world AABB. Repacking changes
grouping,not this source request. These are exact set classifications under
conservative source bounds; they are not visible-pixel/occlusion measurements.

Every primitive is counted separately as:

- source-frustum rejected:parent source batch rejected or primitive box outside
  the **source22000** frustum;
- source requested and retained by the diagnostic near packet;
- source requested but omitted **only by1000 far-plane reduction**.

The test checks source-rejected + source-requested =38352,selected set identity,
source requested minus near selected =distance omissions, and a separate source
coverage flag. The near packet never relabels its distance omissions as source
frustum rejection. Full-source plans keep all source-requested primitives even
when they exceed the budget; they are rejected rather than trimmed to fit.

| Measured strategy | Max vertices | Draws | Coverage/budget result |
|---|---:|---:|---|
|Full source request,far22000|1674882|at most5 after repack|246/264 requests exceed141312; only18 fit|
|Diagnostic near subset,far1000|76434|at most5|All264 fit single-packet bound;262/264 have source-requested distance omissions|
|Ideal unique-primitive union of two near views|147432|not an upload qualification|Exceeds141312 even assuming perfect overlap reuse|

Maximum distance omission is37794 primitives/1672038 vertices. Those counts are
conservative source-requested geometry,not proof each omitted triangle produces
a visible pixel; fog/occlusion do not authorize silently removing it. Two poses
without omissions do not qualify whole-city parity. The full requested maximum
is53596224B Float32 attributes or60295752B under the native RGBA charge; no larger
budget was approved or allocated as an output frame.

The union maximum is across arbitrary tested view pairs,poses136/172 (different
road viewpoints),not adjacent recorded frames. JS exhaustively checks all such
pairs; a direct C test independently builds that pair and verifies147432. It
disproves an unconditional two-view residency assumption,not every continuous
movement trace. Real camera/road-change/menu transitions still need a bounded
temporal policy and measured traces. Five new material buffers require at least
three upload steps at two/step; no stall/pop/camera restriction was invented.

## Fresh tests and independent loader/collision QA

Full isolated native and WASM builds succeeded; final suites each passed
**20/20**. Added suite checks264 source/near plan pairs,deterministic membership,
per-material counts and assembled byte checksums,source coverage/distance flags,
oversize rejection before reads,IO failure,output guards and the transition-union
counterexample. Initial20-suite results before adding the direct union assertion
are preserved separately; final tests include it. Checksums are test validation,
not cryptographic asset authentication; offline SHA256 validates the complete
fixture and source material streams. No headless regression was detected.

Independent full-city collision suite reran5482 browser queries for all2741 city
bodies,max six cell candidates,error1.11e-16,without reference to rendering/page
residency. This remains city-only experimental coverage,not installed complete
game/course/props/actor collision. Existing facade pixel,loader fit/length guard,
nine-road/current-city and race lifecycle suites reran;source/runtime asset
hashes were rechecked. The entire game WASM preload remains byte-identical to the
earlier loader build,confirming the large test fixture did not enter game assets.
Pinned raylib WASM warnings remain; graphics/performance were not exercised.

## Smallest owner decision before using this strategy

| Choice | Consequence | Status |
|---|---|---|
|Keep caps and require source-range fidelity|This whole-primitive repacker cannot qualify all sampled source requests. Keep runtime integration gated and continue exact visibility/occlusion or temporal-streaming analysis.|Current safe default; no new permission needed for independent headless work.|
|Explicitly accept a declared distance/detail limit|The1000-range packet fits five draws/76434 vertices, but262 sampled poses omit source-requested distant content;transition/upload gates still need qualification.|A material fidelity choice requiring owner authorization before use. Not authorized/applied.|

A larger source-range budget is a separate explicit review,not implied by choosing
repacking; the roughly12x browser-batch option was not taken. This evidence applies
to the tested conservative whole-primitive method,not a proof every possible
exact occlusion/streaming algorithm is impossible under the caps. No dependent
runtime integration proceeds by silently changing far range,materials or geometry.

Independent loader/material QA is complete for this slice. Safe further work can
study exact clipping/occlusion bounds or real transition requirements offline,
while preserving this coverage gate. Full source textures/PBR,97 existing meshes'
missing poster geometry,1897 unrepresented grid towers,ring/skyway/crowns/traffic,
manual race,landmark readability,Web graphics and performance remain open.

## Deliverables, unchanged files and window blocker

Evidence:reference/verification-20261102/city-repacking-builds.json and final
city-repacking-{native,wasm}-twenty-tests.log. Exact source/reference hashes and
per-pose classifications:reference/city-repacking.json; source attribute fixture:
reference/city-repacking-geometry.bin (54247104B,test-only). New experimental
rr_city_repacking.{h,c},generated layout,tests/capture tool are never app-linked.
Only existing root/headless CMake files changed to add that separate test/preload
target; originals preserved in reference/pre-city-repacking-20261102.

Runtime remains413 canonical meshes/266 prototypes,679/700 building plan,
991/1024 maximum active world shapes,64 residents,two uploads,16 furniture/four
crown draws,256 dots,800x600 world,960x720 window,30FPS,MSAA/glow off and existing
focus/overload pause. No browser/account/progression edits or uploads occurred.

Window3148188 current state remains unresolved. No desktop input,denied active-log
read,graphics launch or termination was retried; its facade-windows-view executable
and bin assets remain untouched. Earlier clean-shutdown,interrupted camera,
catalog/batching/loader evidence and Library receipt/fixture hashes are preserved.
All project writes remain inside win_app; source browser files were read only.
