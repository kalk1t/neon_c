# Canonical city core and source crowns - 2026-11-02

This is a bounded prototype-to-canonical replacement, not complete city parity.
Within the fixed 24x24 browser cell tile x[-901,1067), z[-1019,949), 96 old
prototype buildings were removed and replaced by 295 source buildings. The
308 prototype buildings outside it remain: 603 total. No individual building
was deleted or moved to improve a screenshot. Existing locks/progress are unchanged.

## Source and implementation

Browser working-tree package 0.2.3; capture 19:43:26Z. Exact ten source/asset
hashes are in `reference/canonical-city-core.json`, rechecked after both builds.
This is content identity, not a Git commit or deployed-release guarantee.
Actual browser city.js grid generation ran with its full RNG/branch flow,
seed1337, STEP82, BASE-260, center(200,0), extent2700. Outside-tile allocation
sinks preserve RNG flow. The complete grid has 2360 towers/450 crown anchors;
this tile contains 295/86. Native uses baked source geometry and collision data,
not a new runtime generator or invented layout. Distant ring/skyways/traffic
are excluded. Source body/neon/tip geometry and RGB are retained; poster image
planes are omitted, with frame geometry retained. Facade window/PBR maps remain open.

The existing city_tower.glb supplies the crowns, losslessly flattened in original
triangle order: 14357 source vertices, 27000 runtime vertices/9000 triangles,
1024px base map, double-sided, source glow2.6 and linear/trilinear/repeat sampling
with 11 mips. All 86 source anchors and source width fits/yaws are preserved.
Nearest four visible crowns draw per frame. This cap is an explicit rendering
limit, not a claim that all source crowns are simultaneously visible.

## Fresh validation

Full native Release and WASM builds passed **15/15** headless suites each.
The new suite compares 600 actual browser collision queries, 18052 all-route
sample spheres (radius2.1, zero city contacts), 86 dressScenery crown matrices
(maximum Float32 error3.42e-05), replacement boundaries/counts and capacities.
Maximum combined world915/1024, city603/700. A first two-face collision query
rounding failure is preserved in `reference/canonical-first-query-failure.log`.
Using captured browser cosine/sine for source box bases resolved the tie without
loosening test tolerance. No claim is made for every continuous path/collision.

Normal native trial 20:34:21-20:34:43Z: menu, road picker with locks, Skyline
countdown/flight rendering and pause observed; clean shutdown verified by log
and window inventory. Source geometry and cyan edges render around the start.
Camera-only trial 20:34:56-20:35:19Z used the unchanged prior hero eye/target.
It remains heavily obstructed by canonical buildings: a narrow glimpse of the
control complex is visible, but this does **not** demonstrate improved landmark
readability or visually qualify crown fits. No layout was altered to hide this.
Crown load/transforms are established by logs/headless tests, not visual acceptance.
Both windows closed cleanly, no overlapping instances. No smoke/fallback flags.

Receipts: `reference/verification-20261102/canonical-core-builds.json` and the
two fifteen-test logs; original graphics logs in `reference/graphics-20261102/`.
Older fourteen-test, interrupted and successful graphics evidence is preserved.
WASM graphics and a complete human-driven race were not run in this slice.

## Resources and remaining gaps

295 city meshes: 196884 vertices, 7090184 bytes; largest2208 vertices. Cache
evicts nonvisible meshes before uploads, at most64 resident building meshes;
both native logs observed64 canonical/64 total. Worst source attribute storage
64*2208*36 =5087232 bytes each in CPU/GPU copies, excluding object/driver overhead.
Two uploads/frame,64 building draws,16 furniture draws,256 dots remain capped.
Crown runtime972008-byte mesh +2437177-byte PNG =3409185 disk bytes, roughly
6.26MiB estimated GPU attributes plus RGBA8 mip storage (not measured allocation).
Four crowns allow36000 source triangles/frame. Total added runtime disk10499369B.
World800x600, window960x720,30FPS,MSAA/glow off and focus/overload pause remain.
No performance qualification or whole-scene stability claim follows from these trials.

Still open: full outer canonical city and transition seam; facades/windows/posters,
skyways/traffic/start artwork; whole-scene PBR/normal/roughness/environment,
ACES/output and bloom; crown/landmark visibility validation from useful source
views; human race/performance/Web graphics acceptance; rivals/combat/audio and
native account/profile adapter. No account/progression decision was needed here.

Existing-source modifications: CMakeLists.txt, tools/race-rules-check/CMakeLists.txt,
src/core/rr_city_plan.{h,c}, rr_world_plan.c, src/app/main.c,
src/presentation/rr_scene.{h,c}. New canonical core/data/crown headers, test and
two generated references, four capture/conversion tools, 295 city meshes and
crown mesh/map plus source/conversion manifests are installed. Pre-change originals
are in reference/pre-canonical-core-20261102; historical reports remain unchanged.
All writes were inside win_app. Claude-owned browser files were read only.
