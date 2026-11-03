# Targeted visibility witnesses and whole-polygon proofs - 2026-11-02

Addressed the specific gap between coarse conservative retention and actual
visibility with a **new bounded test-only hypothesis**: exact city-geometry ray
witnesses,then full-polygon coverage checks driven by their concrete blocker IDs.
Only two existing targeted poses were used:worst retained pose255 and route pose0.
No extra equivalent camera sampling,no distance/detail reduction,no larger budget,
runtime integration or graphics. Source fidelity/caps remain unchanged.

## Concrete new finding

The whole-polygon check proves38 triangles hidden that the existing coarse culler
retained:16 in pose255 and22 in pose0. These are actual **geometric coverage
certificates under the stated CPU model**,not inferred from blocked point samples.
Thus at least part of the remaining bound is demonstrably looseness in coarse
coverage. This does not prove enough can be removed to meet the frame budget.

| Targeted pose | Source triangle candidates | City-only witnessed triangles | All five samples blocked* | Unresolved | New whole-polygon proofs beyond coarse |
|---:|---:|---:|---:|---:|---:|
|255|277828|64097|212981|750|16 of first32 examples|
|0|61888|6146|55622|120|22 of first32 examples|

*Blocked samples **do not** imply the whole triangle is hidden. Examples are the
first32 matching source-ordinal records per pose,a biased targeted set;do not
extrapolate16/32 or22/32 into a scene-wide occlusion rate. All example IDs,blocker
IDs,coarse-mask membership and proof results are in reference/city-witness-results.json.

The identity comparison uses exact coarse-model masks for only these two poses,
with the same22000 far plane,128x96 field and existing guards. Populations match
the preserved model:73 coarse-hidden triangles at255 and7060 at0. Every claimed
ray-visible witness is checked **not** to be in that coarse-hidden set;no
contradiction was found. The masks make the additional38 proofs measurable rather
than assuming they were new. Triangle-only geometric upper bounds after just
these extra proofs improve833265->833217 vertices at255 and164484->164418 at0.
They still exceed141312 and remain upper bounds,not irreducible requirements.

## Witness model, lower bounds and limitations

A deterministic BVH covers all38352 source primitives and565074 source triangles,
using the existing byte-verified source position/normal/UV fixture. Every source
position is validated inside its primitive bounds before building. Source body/
neon/tip FrontSide and board DoubleSide are retained;only actual source-visible
parent batches participate. Rays honor the source near/far depth range. Endpoint,
near-plane,grazing,coplanar and boundary ambiguities are excluded from witness
claims. A candidate must independently pass its own source face/ray intersection
and the tested point must lie strictly within the source frustum.

Up to five strict interior points per clipped candidate triangle are tested.
An unoccluded point witnesses that triangle in the **city-only continuous
double-precision opaque geometry model**,within the tested numerical tolerances.
This is a measured witness lower bound for that model,not final GPU pixels or a
full-scene lower bound:sculpted crowns,props,course and other objects could occlude
those points;GPU rounding,depth ties,MSAA and fragment/material behavior remain
unqualified. Subpixel/continuous visibility is not equivalent to rendered pixels.
No witness result authorizes removing other geometry or changing production.

The worst-view witness subset contains127924 distinct exact attribute tuples.
Its ordinary indexed-triangle representation would use4093568B Float32 attributes
plus769164B32-bit indices =4862732B;adding legacy RGBA would make5374428B. This is
only the witnessed subset,not a complete packet,and the tuple count permits
sharing across materials.16-bit chunking/duplicate costs and remaining geometry
are not qualified. It straddles the current5087232B charged-attribute bound under
different representations,so it cannot establish an unavoidable budget increase.
The prior larger indexed retained set is an upper representation for that view,
not a lower requirement. No complete-scene memory choice is justified yet.

## Full-polygon proof,not more point sampling

For each of the first32 all-samples-blocked examples per pose,the test tries the
recorded actual opaque blocker triangles. It projects the entire strictly clipped
candidate polygon and blocker with the source camera. All candidate vertices must
lie strictly inside the same blocker triangle,and reciprocal blocker depth must
exceed candidate reciprocal depth by1e-6 at every vertex. Perspective reciprocal
depth is affine in screen coordinates for these planar source triangles;strict
positive difference at all vertices of the clipped polygon proves it over the
whole polygon. The geometric proof therefore covers its interior,not just rays.
Partial containment,near instability,degenerate polygons and insufficient depth
separation fail conservatively. Analytic positive,front and partial-coverage unit
cases validate the predicate. Chosen guards do not constitute a formal production
GPU float/depth error proof;this remains an ideal geometric certificate.

## Fresh tests and independent QA

Final isolated native and WASM builds each passed **23/23** headless suites.
Witness/proof summaries,sets and all64 example records match across both targets.
600 independent actual Three.Ray triangle cases check face/intersection behavior;
64 selected full-city rays compare BVH hit presence and nearest distance against
brute force. Nearest ties may identify equivalent surfaces;distance equality is
the accelerator correctness check. Original body bounds contain every source
vertex. No headless regression detected;existing raylib WASM warnings remain.

Independent collision completeness reran5482 actual source city queries for2741
bodies,max six cell candidates,error1.11e-16,without rendering-residency filtering.
Facade pixel,mesh loader,exact material repacking,all-road and race lifecycle
suites reran. All413 current city asset/staged-native hashes and complete game
WASM preload are unchanged. Other world/course/actor collision unions and GPU
image/performance qualification remain open. No new runtime source geometry was
created;the54MB fixture is test-only and never part of game preload.

Analysis memory bounds are explicit and test-only:BVH reserves4909056B,uses
1763264B in27551 nodes;primitive IDs153408B;source fixture54247104B;witness tuple
table8388608B. Actual source-catalog/static test metadata and WASM VFS packaging
add further test overhead. None is app-linked or a production allocation promise.
Test elapsed time is not per-frame performance. No shader,GLFW,graphics or desktop
session was launched by these tests.

## Actionable next step and measurement dependency

Scale the exact single-blocker containment/depth check to the already identified
blocked candidate records in the worst view,then test exact unions of the few
recorded blockers where one is insufficient. Preserve explicit unknown cases and
recompute the certified retained upper set;do not label more ray samples as a
coverage proof. The recorded IDs and deterministic clipped-point rule reproduce
each example without new cameras or graphics. This is useful further headless
proof work,so a graphics dependency is **not** a reason to repeat speculative
sampling or stop independent QA.

Actual pixel/GPU-rounding,MSAA/material correctness,full-scene visibility and
performance do require later authorized graphical measurement once the unresolved
window/input state is reconciled. Runtime integration stays gated on those plus
full geometry/collision pairing,index/byte/draw/upload and temporal working-set
qualification. No irreducible complete-frame gap is proven;do not ask for an
unavoidable budget increase or a fidelity concession from these data.

## Deliverables,unchanged runtime and blocker

reference/city-witness-results.json contains both targeted summaries and all64
examples;reference/verification-20261102/city-witness-builds.json records hashes
and final23-suite logs. Coarse bit masks/reproducer metadata:
reference/city-coarse-witness-masks.{bin,json},tests/generated_coarse_witness_masks.h
and tools/generate-coarse-mask-header.py. Source-only geometry/reference hashes
remain tied to browser working-tree0.2.3 content,not a commit/deployment claim.

New test-only rr_city_witness.{h,c},test_city_witness.c and separate CMake target
are not linked into rr_core or RoadstersRace. test_city_occlusion.c gained an
optional output path mode that processes only poses0/255 for identity masks;
normal regression mode still checks all264. Earlier ray-only,bounded-proof and
coarse source states/logs are backed up under reference/pre-city-witness* and
verification-20261102. All project writes stay in win_app;browser files read only.

Runtime still413 canonical meshes/266 prototypes,679/700 plan,991/1024 active
world maximum,64 residents,two uploads,16 furniture/four crown draws,256 dots,
800x600 world,960x720 window,30FPS,MSAA/glow off and existing safety pause.1897
grid towers,97 missing poster geometries,PBR/lighting,other artwork,manual race,
Web graphics and performance parity remain open. Locks/progress/data unchanged.

Window3148188 remains unresolved. No desktop input,denied active-log read,graphics
launch,termination or change to its facade-windows-view executable/bin assets.
Old completed/interrupted graphics evidence and Library receipt/fixture are
preserved separately. No upload or paid call occurred.
