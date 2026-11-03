# Recorded-blocker coverage: feasibility gate - 2026-11-02

The bounded single/combined-blocker hypothesis gives a measurable reduction,
but **does not qualify a faithful runtime path under current caps**. Three
highest-retention views from the existing 264-view source fixture were tested
in full, plus the route reference. No new equivalent camera sweep was added.
This branch is paused after this result; further speculative culling is not
the next implementation step. Runtime caps, assets, locks and progress are unchanged.

## Measured result and remaining gap

| Existing view | Coarse retained vertices | New whole-polygon proofs beyond coarse | Combined-only proofs (including prior coarse) | Retained vertices | Reduction | Vertices above 141,312 |
|---|---:|---:|---:|---:|---:|---:|
| 255 | 833,265 | 82,394 | 1,592 | 586,083 | 29.66% | 444,771 |
| 249 | 832,341 | 85,108 | 1,692 | 577,017 | 30.68% | 435,705 |
| 251 | 831,684 | 87,149 | 1,539 | 570,237 | 31.44% | 428,925 |
| 0 | 164,484 | 31,963 | 475 | 68,595 | 58.30% | 0 |

The three stress views remain 4.04-4.15 times the vertex bound. The route view
fits this particular geometry bound; it is not full-scene or frame qualification.
Source far remains 22,000. No distance, material, geometry or detail omission
was introduced. Coarse identity hashes exactly match the previous artifact.

| View | Retained nonindexed bytes at current 36-byte charge | Ideal shared exact 32-byte attributes + ordinary 32-bit triangle indices |
|---|---:|---:|
| 255 | 21,098,988 | 13,932,620 |
| 249 | 20,772,612 | 13,817,860 |
| 251 | 20,528,532 | 13,699,028 |
| 0 | 2,469,420 | 1,706,060 |

Compare with the current 5,087,232-byte charged geometry bound. These are upper
representations of the conservatively retained set, not required GPU memory or
an approved new indexed renderer. Ideal indexing shares exact attributes across
material draws; material/color payload, packet index-width limits, buffers,
alignment, CPU mirrors, temporal residency and upload/driver costs are excluded.
The model does not qualify 64 residents/draws or two uploads per frame. No
irreducible full-scene memory or draw lower bound is established.

## Conservative coverage model and its limits

All candidates in views 255, 249, 251 and 0 are processed, rather than the first
32 blocked examples. Full padded-frustum candidate polygons are used for bound
reduction; strict-frustum ray samples only discover potential blocker IDs.
Up to five recorded source-facing opaque triangles are checked. Single-blocker
containment is compared with combined coverage. Combined coverage subtracts the
guarded convex coverage region of each blocker from all remaining convex pieces.
The coverage region includes three barycentric edge inequalities and a fourth
inequality requiring blocker reciprocal depth to exceed candidate reciprocal
depth. Reciprocal depth is affine in screen coordinates for the planar source
triangle, so these are linear half-planes throughout each piece.

Candidate convexity, finite positive depth and affine reciprocal depth are
validated. Blocker edge guard is 1e-7 barycentric, depth guard 1e-6 reciprocal
depth. Up to 128 pieces of 32 vertices are allowed; invalid, degenerate, overflow
or incomplete cases retain the triangle. No positive-area remainder is erased
using an area cutoff. Shared-edge gaps are retained unless actual guarded
coverage overlaps; blocked point samples alone never trigger removal. Maximum
measured uncovered-piece count was 31. Both old coarse-hidden identities and
new proofs are unioned, without double counting. Witnesses do not contradict
the coarse culler. The previous 600 Three ray goldens and 64 accelerated-versus-
brute queries still run, plus overlap/order, gap, boundary and front-depth tests.

These are guarded continuous double-precision CPU geometry certificates, not
exact-arithmetic or GPU rasterization/FP/MSAA equivalence. The city-only ray
witness counts are model-scoped lower bounds; other scenery may hide them, and
continuous witnesses need not occupy rendered pixels. Neither the upper bounds
nor this model's lower bounds prove an unavoidable larger production budget.

## Feasibility decision and measurement dependency

The experiment casts 3,741,284 rays over the 54,247,104-byte geometry fixture.
It reserves a 4,909,056-byte BVH node pool (1,763,264 used), plus 153,408 bytes
of primitive IDs, two 8-MiB tuple tables and bounded polygon scratch. These
are offline test resources, not additions to the application. Under concurrent
native/Web testing the new four-view test took 63.86 s native and 43.65 s WASM;
these are workload wall times, not isolated profiling or per-frame GPU timings.
The method has not established a practical per-frame implementation. Stronger
CPU coverage algebra might reduce retention further, but cannot itself qualify
actual GPU visibility, rounding, pixels, resource residency or frame time.

**Stop further speculative culling work here.** Before selecting a renderer
architecture or requesting any larger budget, obtain a bounded graphics capture
of the reference/native worst-view workload with measured visible/depth coverage,
resident CPU/GPU buffers, uploads, draws and frame time on the owner's hardware.
Reconcile the existing window and current input state first. An eventual renderer
proposal needs those measurements plus full geometry/collision coverage and
temporal upload tests; a reduced CPU count alone is insufficient. There is no
justified cap-increase or fidelity-loss decision to request from this result.

## Independent useful parity QA

After the culling gate, the new tools/audit-city-mesh-parity.py independently
compared every canonical asset against the content-hashed browser 0.2.3 source
fixture in original per-building/material order. All 413 assets / 275,520 vertices
match byte-for-byte for Float32 position, normal, UV and captured base RGBA.
The audit identifies 97 omitted poster planes / 582 vertices across 97 buildings,
with a per-asset manifest for a future bounded poster/material slice. Shader,
poster-raster, PBR and lighting parity remain open. No asset was rewritten.

**Count correction:** the source has 2,360 grid towers; 413 have canonical native
meshes, leaving **1,947**, not 1,897. The old prose figure was an arithmetic error;
the existing machine-readable catalog already says 1,947. Historical reports and
their evidence are preserved. This correction does not include 380 distant-ring
structures and one skyway, nor does it imply the prototype buildings are canonical.

## Verification, files and blocker

Full native and WASM applications compile in separate city-coverage output folders;
each fresh final headless suite passes **24/24**, and all coverage rows are identical.
Existing 5,482 source city collision queries still pass. Source working-tree hashes,
all runtime sources/assets, native copied meshes and unchanged Web app preload
were rechecked; no browser or account files were written. Earlier results and
Library receipt/finish fixture are intact. The initial native build rejected two
shadowed test locals under /WX; names were corrected before the final clean build.
Initial WASM evidence and initial experimental sources are preserved separately.

New experiment: src/experimental/rr_city_coverage.{h,c}, tests/test_city_coverage.c,
and separate targets in CMakeLists.txt / tools/race-rules-check/CMakeLists.txt.
It is not linked into rr_core or RoadstersRace. Detailed results are in
reference/city-coverage-results.json; checksummed builds and full logs are in
reference/verification-20261102/city-coverage-builds.json and the twentyfour logs.
Independent QA: reference/city-mesh-parity-audit.json and its reproducible tool.

**Graphics remain unrun.** Window3148188's state is unresolved after prior input
protection/log denial. No desktop/log retry, termination, game launch or change
to its facade-windows-view executable/bin/assets occurred. The dependent graphics
measurement is pending fresh window/input evidence, not attempted or passed.
Smallest next owner action is confirming that old Roadsters window is closed and
the desktop is idle before controlled measurement is reconsidered. Independent
poster/material planning can proceed without that evidence; no account decision
is needed for that slice.
