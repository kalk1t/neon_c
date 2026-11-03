# Poster geometry/material preparation and independent texture QA - 2026-11-02

The 97 omitted poster planes now have an exact **test-only C preparation path**,
with source placement/UV/material checks and a bounded merge plan. They are **not
rendered by the application**. Production runtime sources, assets, limits, locks
and progress remain unchanged; no files used by window3148188 were altered.
Speculative culling remains paused; existing culling suites only ran as regressions.

## Exact reference and ownership

Browser working-tree package 0.2.3 is the reference, identified by content hashes
in reference/poster-reference.json. No commit/deployment claim or browser edit.
Actual buildCity CPU geometry and actual billboardTexture/board material code
were executed with a Canvas **command recorder**, without a browser, renderer,
pixel rasterization or GPU. It recorded 892 Canvas calls, four gradients, 28
seed-5 glyph paths and all source material/texture defaults. All 349 source poster
planes match the pre-existing full-source Float32 fixture exactly; 97 belong to
the 413 canonical native meshes, and 252 lie outside that represented tile.

Source city.js owns placement and the material. Posters occur in near towers
(the source near/corridor decision is retained), conditional on its RNG and height
clearance. Width is min(building width,26)*0.7; height is 26+r()*40; center height
is top-height/2-15-r()*60. A seeded face rotates by multiples of pi/2 and offsets
by the relevant wall half-width/depth +0.6. Existing source transforms determine
world-space Float32 positions/normals. The existing top/bottom neon border boxes
are already in native meshes and are not duplicated. The atlas quadrant is chosen
by source RNG and scales/translates plane UVs into a 0.5-by-0.5 tile.

Source materials.js owns the procedural 512x1024 CanvasTexture, not a disk image
or GLB texture. It uses four magenta/cyan/purple gradient panels (alpha 0.55),
borders, abstract white glyph strokes, circles and 2-pixel scan lines every six
pixels. The texture is sRGB, flipY=true, clamp-to-edge, linear magnification,
trilinear minification/mipmaps, anisotropy 8, premultiplyAlpha=false. The material
is MeshBasicMaterial (unlit), linear RGB multiplier 1.5, DoubleSide, opaque blending,
depth test/write, fog and tone mapping enabled. Raw source alpha is not permission
to blend the poster with its wall. Current native lit facade shading cannot be
reused unchanged for this material. Browser ACES/exposure/fog behavior must also
be accounted for; whole-scene lighting/tone-map parity is still open.

## Completed headless implementation

src/experimental/rr_posters.{h,c} and generated_posters.h expose exact positions,
normals and UVs for all 97 canonical posters and their stable canonical/mesh/catalog
bindings. Expansion supports the source six vertices or twelve vertices with
reversed back-face winding and normals; this permits DoubleSide without changing
global face-culling state. A checked seeded glyph function reproduces all 28
actual source paths; material metadata and mip-byte accounting match the actual
Three defaults. It does not generate an approximate bitmap or claim pixel parity.

All 97 source-capture hashes match, all 97 source colliders match native bindings,
and every poster position already lies inside its building's existing mesh bounds.
UV quadrants, winding, back-face UVs/normals, finite coordinates, coplanarity,
invalid indices and insufficient output capacity are checked. No collision shape,
building placement or culling bound needs changing for this plane slice.

## Resource decision

| Representation | Worst building draws | Geometry consequence |
|---|---:|---|
| Separate poster mesh per selected building | up to 128 | exceeds current 64 draw policy |
| One aggregated extra poster draw | up to 65 | exceeds current 64 draw policy |
| Merge double-sided geometry into its existing building mesh | 64 | 112,668 vertices in largest possible 64 canonical meshes |

The current largest-64 canonical total is 112,320 vertices. Merged source posters
increase that upper bound by only 348 vertices to 112,668, below the existing
141,312 conservative geometry bound. Prototype cached buildings have 288 vertices
each and do not displace the canonical largest-64 set. At the current 36-byte charge,
merged largest-64 storage is 4,056,048 bytes versus 5,087,232 (1,031,184 bytes slack).
All 97 double-sided planes add 1,164 vertices / 41,904 bytes over the complete
fixed tile. The largest individual mesh would be 2,220 vertices, rather than the
historical measured 2,208; the existing 100,000-vertex raw-loader limit is unchanged.
Two building uploads/frame and 64 resident buildings can be preserved because the
poster joins the same CPU mesh allocation and UploadMesh call. These are prospective
geometry bounds, not a changed renderer or measured frame-time acceptance.

The additional shared atlas requires 2,097,152 bytes at RGBA base level and
**2,796,204 bytes (2.67 MiB)** for its complete mip chain, plus transient decode/staging
memory and driver overhead. This is additional to existing textures and startup
uploads. No total GPU allocation or anisotropy performance qualification exists.
No budget was raised or new runtime allocation added. A single added sampler with
an explicit per-vertex poster discriminator could preserve one draw per building;
the shader must select the unlit sRGB/linear material and source opaque semantics,
rather than use the current lit facade. Exact encoding/GL blending/tonemap behavior
must be validated before such integration; no placeholder or lossy atlas was added.

## Exact blocker and bounded continuation

There is no verified pixel bitmap for the procedural browser poster atlas among
the existing runtime assets. Capturing/rasterizing that Canvas texture through a
browser and validating the native material require graphics work, which is pending
while window3148188/input state remains unresolved. That action was not attempted
or denied again. The command recorder is not a substitute for pixel/antialias,
alpha/color-space, seam/filtering or driver verification. Geometry fitting the
bound alone does not make the combined texture/material path qualified.

Next steps after fresh graphics evidence permits it:
1. Capture the existing source CanvasTexture pixels with hashes and independent
   quadrant/alpha/glyph/scanline checks; no new artwork or paid generation.
2. Build a separately named poster-validation application, appending the prepared
   planes at CPU decode and retaining original RRC1 asset files and body fit checks.
   Keep old binaries and all assets used by the unresolved window intact.
3. Add the shared sampler/unlit opaque material branch, duplicate back faces and
   preserve 64 draws/residents, two building uploads, geometry and safety limits.
4. Measure exact texture allocation, startup/upload costs and bounded front/back
   material views before normal race acceptance. Do not infer GPU success from C tests.

Smallest current owner action is reconciling the old Roadsters window and confirming
the desktop is idle before controlled graphics/texture capture is reconsidered.
No account/profile decision is needed; no cap increase is currently justified.

## Independent substantive QA completed

While material integration is gated, tools/audit-texture-bindings.py audited the
existing cyan-tree, control-tower and city-crown source/converted GLBs and runtime
PNGs. Each converted GLB preserves the original source BIN prefix byte-for-byte,
and mesh/accessor/node/scene/material/sampler metadata is unchanged. Each native
base-color PNG is exactly the image selected by its converted source material.
Every PNG chunk CRC, compressed scanline length and scanline filter was validated
and the full decoded pixel stream hashed, without an image renderer or browser.

All three are 1024x1024 with alpha uniformly 255, white source baseColorFactor,
OPAQUE material, DoubleSide and trilinear source minification. Their full RGBA mip
chains would each cost 5,592,404 bytes. This is a texture binding/binary/PNG audit,
not a new WebP-vs-PNG pixel equivalence claim or PBR/lighting completion. The current
tree runtime path remains bilinear without generated mipmaps; tower/crown paths
generate mipmaps. Those filter differences are documented, not silently changed.
No raster decoder was installed and no source or runtime texture was rewritten.

## Verification and status

Full native and WASM applications compile in new poster-plan output folders.
Each fresh headless suite passes **25/25**; poster summaries are identical. Existing
source collision/route/race tests remain passed. Source hashes, runtime files,
all copied canonical meshes and unchanged Web app preload were rechecked, and prior
evidence/Library receipt preserved. Experimental code is not linked into rr_core
or RoadstersRace. Graphics and poster pixels remain unrun/unverified.

Current grid status remains **2,360 source towers - 413 canonical meshes = 1,947
unrepresented grid towers**. The historical 1,897 prose error remains superseded
by this count; historical reports/evidence were not rewritten. The 97 native-tile
planes are still absent from runtime graphics, with 252 more outside the tile.
Whole-city geometry, source ring/skyway, full materials/PBR/lighting, performance
and manual race acceptance remain open. Locks, progress and Claude-owned files
are unchanged. No graphics launch, desktop/log retry or termination occurred.

## Reproduction and deliverables

- tools/capture-posters.mjs WIN_APP_ROOT STAGING/poster-reference.json records
  actual CPU geometry and Canvas calls only; no graphics context is created.
- tools/prepare-poster-plan.py WIN_APP_ROOT STAGING verifies against the existing
  full-source fixture and writes only staging headers/inventory.
- tools/audit-texture-bindings.py WIN_APP_ROOT NEW_REPORT.json repeats binary and
  PNG checks without renderer/dependencies, refusing to overwrite the report.
- reference/poster-reference.json, poster-inventory.json and poster-plan-results.json
  contain full source calls, all bindings, resource totals and native/WASM summary.
- reference/texture-binding-audit.json records independent texture findings.
- reference/verification-20261102/poster-plan-builds.json records hashes, changed
  paths and the two complete twentyfive-suite logs. Backups: reference/pre-posters*.
