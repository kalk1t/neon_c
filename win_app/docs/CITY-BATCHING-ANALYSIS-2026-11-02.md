# Exact frustum and batching analysis - 2026-11-02

Completed a **test-only** comparison of actual browser batches, tighter bounds
and three spatial/material grouping sizes. No runtime renderer,geometry,material,
collision,resource cap,lock/progress or user-data changes.1897 grid towers still
lack runtime geometry. This analysis is not whole-scene parity or performance acceptance.

## Reference fidelity and representative views

Working-tree package0.2.3; capture 2026-11-02T21:25:41.446Z. Exact source hashes in
reference/city-batching.json, rechecked after builds. All38352 source geometry
parts/1695222 vertices were retained in every scheme, with source transforms,
material categories and complete primitive membership. Metadata does not emit
new meshes or replace any runtime assets. Sphere centers/radii use actual
transformed Float32 positions, not just AABB diagonal approximations.

An independent CPU-only invocation of the **unmodified** browser buildCity
constructed the actual merged scene. A Canvas raster sink supplied material
canvas calls; no renderer/window/network/GPU was used. All45 CH900 material batches,
vertex counts,AABBs and bounding spheres matched the capture exactly (sphere
maximum error0). The initial experimental material iteration order was corrected
to the source body/neonC/neonM/tip/board order; final fixtures/tests use this order.
Initial passing internal-fixture logs are preserved separately and not used to
assert actual-source ordinal equivalence. Source shader/material appearance was
not validated by the Canvas sink or metadata tests.

264 representative poses:24 positions/tangents per each nine roads (216),32
free-roam grid poses at two altitudes,eight distant ring viewpoints,one above-city,
six center-facing axes and the previous landmark camera (48). Each is evaluated
with near.3,fov70,aspect4:3 and far1000/current native range or far22000/source camera:
528 frusta. These are synthetic coverage poses,not human input traces or timing.

The primary scheme reproduces browser per-material chunks of900 parts and actual
sphere frustum decisions. Alternatives assign complete source primitives to
128/256/512-unit horizontal cells by center, keep material partitions, and compute
both exact geometry spheres and AABBs. Every primitive remains whole in one group;
no triangle,poster or material category was removed to meet limits. A fifth
per-structure scheme provides a finer bound baseline,not a material draw strategy.
Actual Three.Frustum.intersectsSphere/intersectsBox supply expected set hashes,
draw counts and vertex sums. Alternatives intentionally change grouping/culling
granularity; they are not asserted to have the browser's original45 draw IDs.

## Fresh native/WASM verification and independent collision coverage

Full isolated native and WASM builds succeeded; final corrected test fixtures
each passed **19/19** suites. The added suite checks5280 visible sets/counts/vertex
sums, source topology/material vertex totals across all schemes, invalid bounds
and exact sphere/box decisions. Test-only volume metadata1151744B; not linked to
rr_core or RoadstersRace. After source-order correction only the changed test
target required rebuilding, then all19 suites reran. Initial outputs are preserved.
No headless regression was detected. Existing pinned raylib WASM warnings remain.

Collision completeness is tested separately from rendering residency: the prior
full-city catalog suite reran5482 browser queries covering all2741 city bodies,
max six original-cell candidates,worst correction error1.11e-16. Culling schemes
cannot remove those catalog candidates. This remains **city-only experimental**
coverage; course/props/actors/long-segment/transition unions are not installed in
runtime. No invisible full-world collision was added ahead of missing geometry.

Independent loader/material QA continued through fresh source facade pixel,
mesh-fit/size-guard and other regression suites, per-material topology conservation,
all413 runtime city asset/staged-native SHA checks and identical complete WASM
runtime preload compared with the prior loader build. Existing loader/game source
hashes remain unchanged. No texture/material setting was altered.

## Measured bounds: no fixed scheme satisfies both current limits

Limits for comparison:64 building mesh draws/residents and141312 vertices /
5087232B RGBA attributes per copy, derived from current64*2208 building maximum;
two uploads per frame. Bodies only: crowns,traffic,props,source texture/material
memory and CPU/GPU copies add costs. Maxima below may occur in different poses.

| Far | Grouping / bound | Max draws | Max vertices | Views over64 draws | Views over141312 vertices |
|---:|---|---:|---:|---:|---:|
|1000|Source CH900 / sphere|45|1695222|0|262/264|
|1000|128 cells / box|265|92676|253/264|0|
|1000|256 cells / box|126|139308|251/264|0|
|1000|512 cells / box|69|218700|4/264|158/264|
|1000|Per structure / box|124|89634|227/264|0|
|22000|Source CH900 / sphere|45|1695222|0|264/264|
|22000|128 cells / box|5871|1675242|262/264|248/264|
|22000|256 cells / box|2442|1675386|262/264|254/264|
|22000|512 cells / box|919|1675782|262/264|260/264|

Tighter box culling removes many conservative sphere false positives at far1000,
bringing selected exact source vertices under the bound for128/256 cells. Many
small material batches then violate64 draws. Bigger cells lower draw count and
raise vertex exposure. Source batches have a low draw count but broad bounds:
central views can select the whole city even at far1000. This is draw/payload
accounting,not measured GPU time,actual allocations or proof that all selected
triangles contribute visible pixels. Original source remains full CPU city build
with lazy first-visible GPU upload/caching; none of these alternatives is browser
sector streaming or an authorized runtime replacement.

## Short option comparison before runtime work

| Option | Evidence / bound | Status |
|---|---|---|
|Keep caps; analyze post-cull exact material repacking at current far1000|128-cell box selection peaks92676 vertices/3336336B attributes. Repacking surviving complete primitives by five source materials could reduce draws to at most5, but is not implemented or temporally qualified.|Safe independent next headless task; preserve full selected geometry and expose incompleteness.|
|Reproduce source45 batches and far22000 behavior|Observed full selection1695222 vertices/61027992B native RGBA attributes per copy,about12x the current inferred bound,before other assets/driver overhead.|Needs explicit memory/performance budget review and controlled profiling before a cap change; not authorized/applied.|

The first option still needs upload scheduling and old/new working-set union
tests: five material buffers can take at least three steps at two uploads/step;
double buffering or changing views could exceed byte bounds. CPU assembly,source
material/shader binding and camera/loading semantics are not solved. Far1000 itself
remains a declared difference from the source22000 camera. No full-paging integration
or hidden clipping/LOD/fidelity reduction occurred.

No evaluated fixed scheme achieves source-range full coverage under both limits.
This does not prove every possible exact occlusion/repacking algorithm requires
a larger budget. The smallest decision before adopting the source-resident strategy
is whether to review its explicit higher byte budget; otherwise continue the
cap-preserving near-range repacking analysis and keep whole-city parity open.
There is no owner decision required to continue that independent headless work.

## Unchanged runtime, evidence and unresolved window

Runtime still has413 canonical meshes/266 prototypes,679/700 building plan,
991/1024 maximum world shapes,64 residents,two uploads,16 furniture/four crown
draws,256 dots,800x600 world,960x720 window,30FPS,MSAA/glow off and focus/overload
pause.97 source-mapped meshes still lack poster geometry;1897 grid towers and
distant ring/skyway/crown/traffic/material visual coverage remain incomplete.
Whole-scene PBR/normal/roughness/environment/ACES/bloom/anisotropy,manual race,
landmark readability,Web graphics and performance acceptance remain open.

reference/verification-20261102/city-batching-builds.json contains source/artifact
hashes and final test logs. reference/city-batching.json has all scheme/resource
summaries and pose categories. New src/experimental/rr_city_batching.{h,c} and
generated_city_batches.h,tests/test_city_batching.c/generated fixtures and
tools/capture-city-batching.mjs are test-only. Existing root/headless CMake files
only add that separate target. Originals/initial fixtures and logs are preserved
under reference/pre-city-batching* and verification-20261102. No runtime asset
was written or uploaded; source browser files are read only.

Window3148188 current state remains unresolved. No desktop/denied-log retry,
graphics launch,termination or change to its facade-windows-view executable/bin
assets occurred. Earlier facade clean shutdown and blocked camera evidence stay
separate; existing Library receipt/fixture hashes remain unchanged.
