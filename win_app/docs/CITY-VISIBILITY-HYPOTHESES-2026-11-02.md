# Source-fidelity visibility hypotheses and resource accounting - 2026-11-02

Tested two **new hypotheses**,not more equivalent camera sampling: exact
triangle/frustum plus source face rejection,then conservative opaque occlusion
with continuous full-tile coverage. All work is headless/test-only. No near-range
substitution,distance/detail loss,runtime cap increase or game integration occurred.
Source-fidelity remains the required target;1897 grid towers still lack runtime assets.

## Result and what it does not prove

Exact triangle clipping/face rejection lowers the worst sampled source request
from1674882 to833484 original vertices;86/264 poses fit141312. Guarded coarse
occlusion lowers the worst to833265;94/264 fit. Therefore **these methods are not
qualified** under the current bound. These are conservative retained-geometry
**upper bounds**,not irreducible visible-geometry lower bounds. It would be false
to conclude all faithful culling is impossible or demand a larger budget from
these results. Finer exact occluder-union/visibility proof remains unresolved.
Dependent runtime integration stays gated; no production budget proposal is
presented as necessary or approved.

## Source rules and exact clipping hypothesis

Working-tree package0.2.3; source hashes in reference/city-triangle-reference.json,
reference/city-repacking.json and build receipt, rechecked after builds. Reuses
the byte-verified54247104B source geometry fixture and264 source22000-far poses.
No1000-far subset or fog threshold is used. All source parent-batch decisions
remain tied to the actual45 source batches; primitive source-frustum candidate
counts must match earlier actual Three references before triangle processing.

Source body/neonC/neonM/tip materials use FrontSide; source board uses DoubleSide.
World-space Float32 triangles retain original attributes/order. A double-precision
Sutherland-Hodgman clip against all six actual source planes rejects triangles
wholly outside. Remaining original triangles would be submitted unchanged for
GPU clipping/interpolation; no CPU-generated replacement vertices or LOD. Source
front winding rejects backfaces; ambiguous edge-on faces are retained. Both zero
plane margin and an expanded.02-world-unit margin are counted to expose boundary
sensitivity. This is a CPU geometric model,not certification of GPU float32 clip,
MSAA,driver depth or pixel equality. It must not be enabled based on these tests alone.

600 independent actual Three.Box3.intersectsTriangle SAT cases validate the clip
predicate,using an algorithm different from polygon clipping. Actual Three.Ray
backface cases validate the front rule. Camera eye positions match the existing
source side planes (maximum error9.09e-13). Native/WASM all264 per-view counts and
triangle-set hashes agree exactly. Face/plane tests use source distance throughout.

## Conservative occlusion hypothesis and its refinement

An analysis-only128x96 depth field stores an occluder only when its projected
opaque triangle covers the **entire continuous tile**,with.01-NDC edge guard.
Perspective reciprocal-depth interpolation bounds its farthest tile depth;
candidates are culled only when every tile touched by an expanded candidate box
has a strictly closer bound. Unknown tiles retain geometry. Depth guard is
1+32*z*z/(.3*2^23),and projections nearer than one world unit are left uncullable.
City materials' default opaque/depth-write behavior is required; no texture-alpha
discard/transparent object is treated as an occluder. Source boards are kept
double-sided. Extra props are not needed as occluders for this city-only proof.

The first version required one triangle per full-tile proof,leaving diagonal
holes across two-triangle walls. A distinct refinement uses two **actual** adjacent
source triangles only when they share exactly four world points,are both source-
facing,lie inside the source frustum,and their projected union equals a convex quad.
It retains a global farthest-depth bound for that exact union,so nonplanar pairs
do not get an invented interpolated surface. Concave/partial/near/ambiguous pairs
are rejected. No numerical guards,distance or geometry were relaxed. Unit checks
cover full/partial coverage,unknown tiles,front/behind ordering,convex and invalid
quads,and the independent600 triangle cases. Initial triangle-only occlusion
evidence is preserved separately; final tests include exact pair unions.

The refinement increases fitting poses92->94 and maximum culled geometry to424947
vertices in some views. Worst pose255 remains833265 retained vertices;coarse
coverage/guards still keep many possibly hidden surfaces. This does not establish
an irreducible gap. Screen/depth guards are chosen analysis bounds,**not a formal
interval proof of production GPU rounding**. Even a fitting result would require
that proof,material/depth-state qualification,temporal/resource tests and later
authorized graphical validation. No visible fidelity was changed in production.

## Fresh evidence and independent QA

Triangle hypothesis isolated builds passed21/21 native and WASM suites. Final
combined builds passed **22/22 each**; all264 triangle and occlusion result rows
are identical across native/WASM,including hashes. No headless regressions found.
Existing pinned raylib WASM warnings remain. No graphics or FPS/memory-allocation
measurement occurred. CPU test times are not game-frame timings.

Independent city collision tests reran5482 source queries over2741 bodies,max six
cell candidates,error1.11e-16,regardless of visibility. Course/props/actor/streaming
unions still are not installed in runtime. Facade pixel,loader guard,race,roads,
catalog and repacking regression suites reran. Source GLB/RRC1 hashes and all413
city assets match; complete runtime WASM preload remains byte-identical to the
earlier loader build. Library receipt/finish fixture and old clean-shutdown versus
blocked-camera evidence remain unchanged and separate.

## Conditional resource numbers, not a larger-budget authorization

For the worst triangle-count pose255,exact material-aware attribute deduplication
finds489086 unique position/normal/UV tuples and833484 index entries. Their exact
32-byte attribute plus32-bit-index payload is18984688B;retaining current RGBA adds
it to20941032B. This is for **one measured pose**,not a proven maximum indexed
packet across all views or a visibility lower bound. Source first two material
groups exceed65535 unique vertices; current raylib Mesh indices are16-bit,so an
actual implementation needs tested chunking/duplication or explicit32-bit-index
support. Five material groups and ideal indexing are not an implemented renderer.

Independent logical accounting reads only the earlier completed normal-view log
and unchanged runtime assets; reference/city-static-resource-budget.json records
all assumptions. Known nominal resources excluding city buildings:

| Resource | Logical bytes |
|---|---:|
|Current logged textures,mips and offscreen color/depth|63292076|
|Car/course CPU mesh attributes and16-bit indices;nominal mirrored GPU payload|9452032 per copy|
|Known GPU subtotal without city buildings|72744108|
|Known GPU subtotal with current5087232B city bound|77831340|

If the one-pose indexed RGBA packet above were adopted without further culling,
known GPU payload would be93685140B **before** excluded/default render batches,
swapchain,driver/program storage and future parity assets. Retaining that packet
on CPU plus current mesh arrays,full primitive metadata2454528B,98312B depth field
and the current25165824B dedup analysis table totals58111728B known CPU structures;
keeping the full source fixture resident adds54247104B (112358832B subtotal).
Streaming can avoid that full source residency but introduces unqualified IO/CPU
cost. These are conditional accounting subtotals,**not total application allocation
or a final minimal budget**. They exclude core/application state,allocator/driver
overhead and staging/old-new frame overlap. Double buffering adds another packet;
at two uploads/step five changed material groups need at least three steps. No
per-frame scan/dedup/upload timing or disocclusion scheduling has been qualified.

Thus there is no defensible complete CPU/GPU budget increase to ask the owner to
approve yet. A minimal production proposal must first settle stronger visibility
bounds,peak indexed geometry across views,16-bit chunk behavior,temporal overlap,
IO/CPU workload and full application overhead. The roughly12x source-batch path
was not chosen. All production limits remain unchanged.

## Next bounded question and unchanged runtime

The remaining new hypothesis is finer conservative coverage/occluder union or
visibility witnesses to bracket truly visible geometry. It should target the
specific worst cases and numerical proof,not merely repeat equivalent camera
samples. If a qualified retained set fits,then test exact source attributes,
paired collision coverage and transition/upload readiness. If an actual lower
bound exceeds the cap,then bring a concrete whole-application budget/fidelity
choice to the owner before implementation. Source-fidelity/distance is unchanged.

Runtime remains413 canonical meshes/266 prototypes,679/700 building plan and
991/1024 maximum world shapes;64 residents,two uploads,16 furniture/four crown
draws,256 dots,800x600 world,960x720 window,30FPS,MSAA/glow off and existing safety
pause.1897 grid towers,97 missing poster geometries,other city/world art,PBR/
lighting,manual race,Web graphics and performance acceptance remain open.

Deliverables:reference/city-visibility-results.json (all rows),reference/
verification-20261102/city-visibility-builds.json and preserved21/22-suite logs;
new test-only rr_city_triangle/rr_city_occlusion modules and fixtures/capture tools;
static budget script/JSON. Existing CMake files only add separate headless targets;
none of this is app-linked or shipped as runtime assets. Originals and earlier
occlusion hypotheses are preserved under reference/pre-city-triangle*,pre-city-
occlusion* and verification evidence. No browser/account/progression writes.

Window3148188 is still unresolved. No desktop input,denied active-log read,graphics
launch,termination or changes to its facade-windows-view executable/bin assets.
All project changes stay in win_app; Claude-owned sources were read only.
