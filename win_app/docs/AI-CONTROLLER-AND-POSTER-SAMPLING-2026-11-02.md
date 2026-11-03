# Shared AI controller and source poster sampling - 2026-11-02

The next source-established core gameplay gap now has a **shared C AI controller**,
with actual-browser equivalence on all nine roads. Poster source sampling/orientation
is also verified headlessly. Live rivals remain inactive and poster runtime integration
is pending; this is not full W3, a complete AI race or native material appearance.

## Poster pixel verification: completed and genuinely pending

The supplied source owns its atlas in materials.js:billboardTexture(), creating
procedural Canvas artwork. Project public/dist/runtime/reference image inventory
and the actual source dependency confirm there is no supplied source-owned poster
raster to decode. Source calls do not load one from a GLB or disk texture. A new
approximate CPU raster would not independently verify browser antialias/pixels,
so none was invented. No browser, rasterizer or graphics context was launched.

The useful independent check is now implemented in rr_posters.c:rr_poster_sample.
It matches **121 actual Three Texture.transformUv cases** plus base-level bilinear
address/weight goldens: clamping outside [0,1], atlas seams on both sides, texture
edges, half-texel positions and quadrant centers. The source flipY means UV
quadrants 0,1,2,3 select Canvas panels **2,3,0,1**, not identically numbered panels.
This avoids a concrete upside-down/wrong-panel atlas port. Source edge sampling
can straddle panels; no unrequested half-texel inset or UV rewrite was added.
NaN/invalid output guards are checked. Existing exact plane/glyph/material/binding
checks still pass. Results do not include color pixels, filtered mip LOD/derivatives,
alpha compositing, shader appearance or GPU sampling qualification.

**Exact pending action:** obtain/decode the source 512x1024 Canvas RGBA raster and
compare actual pixel/alpha/glyph/gradient/scanline data. No source raster input is
available; raster capture remains under the no-graphics gate while window3148188
is unresolved. This action was not attempted or denied again. The existing no-render
PNG CRC/unfilter audit can validate a captured PNG when available. The smallest
next graphics prerequisite is reconciling the old window/input state, followed by
bounded source Canvas capture or an owner-supplied exact source-hashed raster.
Native material appearance still requires its separate controlled graphics check.

## Why the AI controller was selected

The supplied ROADMAP W3 explicitly requires ten rivals using shared car physics.
Browser ai.js defines ten named liveries and deterministic aiInput behavior;
main.js applies Racer.progress and road-based rubber banding. Native had shared
flight, roads, recovery and solo race logic but no AI controller. These functions
have clear source behavior and need no account, unlock, economy, product or
graphics decision. They are a substantive independent gap while the atlas is gated.

## Implemented behavior

New shared core rr_ai.{h,c} and captured rr_ai_liveries.h provide:

- Exact ten-driver names, paint/accent, skill and optional metal metadata. Missing
  metal properties remain explicit rather than assigning an invented default.
- Source speed-dependent look-ahead, sampled track right vector and racing line.
- Ordered avoidance of live cars strictly ahead by <32 and lateral separation <5.5;
  later peers preserve source override behavior, with exp(-1.2*dt) dodge decay.
- Lane sway and height offset fading to the center near the target gate, optional
  coin diversion at lerp 0.85, wrapped yaw, brake/steer/climb and boost thresholds.
- Source start-line progress wrapping and road-factor rubber band clamped to
  -5%/+7%. Skill controls boost decisions, not an invented lower top speed.

The controller accepts caller-provided driver state and ordered peers, with no
random spawn policy or account dependency. It returns the existing RRInput for
RRCar and mutates only the source dodge state. It allocates no heap memory, uses
the existing shared track and caps peers at ten. A caller passes already-prepared
simulation time/dt; the controller does not add another frame cap.

The new module is compiled into rr_core and available to both targets. **No app
code starts or draws rivals**, adds contacts/weapons/rewards, changes session
results or user progress. Existing application/presentation sources and runtime
assets remain byte-identical. Activating an entire pack without its lifecycle,
contacts, combat and renderer qualification would overstate this bounded slice.

## Actual-source evidence

Read-only tools/capture-ai.mjs calls actual aiInput, Racer.progress, updateRanking
and Car.update. Only Car rendering objects and collision calls are stubbed for
short no-contact drive traces; no graphics, network or account code runs. Source
working-tree package 0.2.3 and content hashes identify the reference, not a deployed
version or commit. Sources are rehashed before reporting.

| Check | Fresh coverage |
|---|---:|
| Independent steering/avoidance/lane/coin/boost cases | 288 across nine roads |
| Coupled AI -> existing Car physics traces | 18 |
| Mixed-dt coupled drive steps | 3,240 |
| Start-line/lap progress cases | 108 |
| Actual updateRanking road rubber-band cases | 45 |
| Source poster UV/address cases | 121 |

Strict avoidance boundaries, dead peers, iteration order, invalid pointers/indices,
all ten roster entries and progress/factor bounds are also checked. The 18 drive
traces are short single-vehicle, no-collision traces, not successful full AI races.
Maximum observed controller/progress error: **8.948e-14**; maximum
observed coupled car-state error: **2.542e-13**. Tolerances are 1e-8
for control and 1e-5 for coupled car state; both targets produced identical summaries.

## Builds, preserved state and remaining work

Full native and WASM applications compile in new ai-sampling output folders.
Each complete fresh headless suite passes **26/26**. Existing culling suites ran
only as unchanged regressions; no new culling hypothesis or runtime integration.
No graphical test or FPS/resource acceptance is implied. Existing runtime source
hashes/assets, native copied meshes, unchanged Web preload, source manifests,
historical test logs and Library receipt/finish fixture were rechecked/preserved.
Project changes stay in win_app; Claude-owned files are read-only. No paid call,
network/account operation, publishing, desktop input, active-log retry, app launch,
termination or change to facade-windows-view executable/bin/assets occurred.

The corrected current city count remains **1,947 unrepresented grid towers**;
97 tile posters are prepared but still absent from runtime graphics. Pixel and
native material verification are pending as above. Other independent gameplay
work is not exhausted: the next concrete W3 source slice is car-car contacts and
rival race lifecycle/reset/respawn rules before combat/render activation. That
can be evaluated headlessly without account/product decisions. No need to invent
more culling experiments or claim every useful task is blocked by graphics.

## Deliverables and reproduction

New core: src/core/rr_ai.h, rr_ai.c, rr_ai_liveries.h.
Tests: tests/test_ai.c, generated_ai_reference.h; poster sampling extends the
existing experimental poster module/test with generated_poster_sampling.h.
Source references: reference/ai-reference.json, poster-sampling-reference.json,
ai-sampling-results.json and poster-pixel-blocker.json. The latter records exact
pending action/state and project image inventory, not a fabricated raster.
Checksums/full logs: reference/verification-20261102/ai-sampling-builds.json and
the two twentysix-suite logs. Earlier sources are under reference/pre-ai-sampling*.

Source-only adapters: tools/capture-ai.mjs WIN_APP_ROOT OUTPUT.json and
tools/capture-poster-sampling.mjs WIN_APP_ROOT OUTPUT.json. Regenerate staging
C fixtures with tools/prepare-ai-reference.py STAGING_DIRECTORY. These adapters
never create a renderer or activate a game, and never write browser sources.
