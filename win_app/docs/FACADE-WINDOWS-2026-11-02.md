# Canonical facade windows - 2026-11-02

Canonical city buildings now show the browser's existing deterministic window
map. Placement, geometry, collision bodies, UVs and crown anchors are unchanged.
This bounded material slice adds no outer city cells, grants no progress and
does not claim whole-scene PBR/lighting or manual race acceptance.

## Browser reference and implementation

Working-tree package0.2.3, capture20:39:18Z; source materials.js/windowTexture,
city.js and package hashes plus exact RGBA digest in reference/facade-windows.json.
Current main.js calls buildCity without options, retaining default windows=.95;
its hash is retained in the canonical manifest. All source hashes were rechecked
after builds; no commit/deployment identity is implied. Claude-owned files were read only.

The actual JS windowTexture function ran CPU-only against a fillRect Canvas
adapter: all calls have integer positions, opaque RGB and axis-aligned bounds,
so it preserves these raster operations exactly without a browser/GPU. It emits
256x512 top-row-first RGBA8, 512 window rectangles (142 lit), black gutters,
seed99 source RNG/palette/short-circuit evaluation. Native shared C generates
the same pixels into a temporary buffer at scene load; no AI image/new art or
extra runtime disk texture was introduced. Reference raw pixels are evidence
only, outside assets/preload.

One shared GPU texture uses repeat, linear/trilinear and10 mip levels. Source
facadeBox UVs already retained in canonical meshes are used unchanged. Shader
sampling reverses V to reproduce CanvasTexture flipY; source darkUV(.004,.004)
continues sampling black gutters. Emission is added only to source body tint
(13,11,36), with sRGB decode and source .95 intensity, before existing native fog.
Neon/tip surfaces bypass the window contribution; a separate canonical material
and reset uniform prevent it affecting prototypes, cars, props or UI. This uses
the current native diffuse/light/output pipeline: source metalness/roughness,
environment, ACES/output, anisotropy8 and bloom are explicitly unported.

## Fresh tests and native observations

Full native Release and WASM builds each passed **16/16** headless suites.
The new test compares all131072 RGBA pixels against the captured browser map
(FNV1762891081), checks512 source window colors/rectangles, gutters/alpha,
null/short-buffer rejection and guard bytes. The fifteen earlier suites ran
fresh, including canonical600 collision queries/18052 route samples/86 crown
matrices. All295 canonical mesh SHA256s and staged native copies were rechecked;
no placement or UV bytes changed. Existing raylib WASM ES3 override and stb_vorbis
warnings remain; no app/core build failure. WASM graphics were not executed.

Normal native20:40:27-20:41:07Z: road picker retained locks, Skyline countdown
rendered visibly lit facade windows on both sides of start, followed by pause.
Native world shader compiled, map loaded256x512/10mips, cache reached64/64,
and clean shutdown was verified by logs and subsequent window inventory.
No full race or graphics performance/stability qualification follows from this.

Second camera-only trial remains **interrupted, not passed**. It reached the
road picker; Enter was blocked by user-input protection. A fresh observation
showed concurrent desktop activity, so no input retry, close or termination was
attempted. Window3148188, executable build/facade-windows-view/bin/RoadstersRace.exe,
remains open at menu as last observed. No later clean shutdown is claimed.
Previous canonical camera obstruction remains unresolved and no building was
moved to improve the landmark view. Existing prior closed/interrupted trials
remain separate. Reading active build/facade-windows-view/bin/logs/roadsters.log
was denied with OS PermissionError13; not retried, no camera snapshot was saved.
Original normal log,
two test logs and executable/WASM hashes are listed in
reference/verification-20261102/facade-windows-builds.json.

## Resources, reporting and next work

Temporary CPU pixel buffer524288B is freed after texture upload. The shared
RGBA8 texture plus10 mips totals699052B estimated GPU storage, excluding driver
overhead; this is not measured GPU allocation. No extra draw calls, building
mesh vertices, runtime disk assets or placements. Existing64-mesh residency,
two mesh uploads/frame,64 building/16 furniture/4 crown draws,256 dots,
800x600 world,960x720 window,30FPS,MSAA/glow off and focus/overload pause remain.
Cache64 was observed in the completed normal trial.

No persisted city comparison PNG exists in the graphics evidence directory;
only older finish fixtures were found. CUA development screenshots are visible
in tool output, but no existing local city image is available for supported
file upload. No new reporting render or duplicate upload was made. The successful
earlier finish Library receipt and fixture hashes were rechecked unchanged.

Next independent gaps: outer canonical city/prototype seam, poster images,
skyways/traffic/start artwork and whole-scene material/lighting. Crown/landmark
readability and manual race/performance/Web graphics acceptance remain open.
The current camera-window input blocker needs reconciliation before further
desktop work; headless/source work can proceed. No account decision was needed.

Changed existing: src/presentation/rr_scene.{h,c}, rr_shaders.h, CMakeLists.txt,
tools/race-rules-check/CMakeLists.txt. Added: src/core/rr_facade.{h,c},
tools/capture-facade.mjs, tests/test_facade.c, generated_facade_reference.h and
reference/facade-{browser-rgba.bin,windows.json}. Exact code/evidence hashes are
in the receipt; originals in reference/pre-facade-windows-20261102. All edits
stay in win_app; prior source geometry, reports and builds remain preserved.
