# Validation and performance targets

## What was checked for the delivered handoff

The packaged QA report records actual file-level validation: PNG decode/alpha, item and prompt presence, cross-references, route math validation, Python syntax and ZIP integrity. Source image bytes are retained without retouching. The route report describes its own test scope.

Blender was unavailable during pack preparation, so there is no claimed Blender execution, rendered .blend validation, GLB export/import validation or gameplay test. The source kit is intended to make those next steps concrete and repeatable.

## Production acceptance checks

Run the supplied route validator after any route or placement edit. Rebuild/re-export and recheck the game after changing collider shapes. Source route/placeholder tests do not account for the final detailed mesh, camera envelope, physics integration or fast vehicle sweep.

Inspect every imported/generated asset for silhouette, dimensions, pivot, normals, alpha/material behavior and collision. Test the hollow openings explicitly. Recount final GLB triangles and materials after export rather than relying on a Meshy request value.

Use at least one slow diagnostic lap and several normal-speed laps with all three cars. Test reversed travel, missed gates, off-course recovery, attacks, respawns, restart, lap completion, cockpit view and map switching. Test the lowest quality setting too.

## Suggested initial browser budget

These are **design targets**, not measured performance or universal device limits.

| Area | Initial target |
| --- | --- |
| Visible scene geometry | Around 150k-300k triangles at medium quality, including cars; reduce for lower-tier devices |
| Main opaque/alpha-tested draws | Prefer roughly 80-150 visible draws before effect/post passes |
| Prop LODs | Three useful levels; preserve silhouette first and remove hidden small detail |
| Repeated props | Shared geometry/materials and batching/instancing through the existing engine |
| Textures | Reuse atlases and 512-1024 px props; reserve larger maps for visible hero detail |
| Lighting | One main light plus restrained ambient/environment light; avoid per-neon point lights |
| Rain | Start around 400-900 nearby particles; lower mode around 150-300 |
| Splashes / mist | Small pools; monitor alpha overdraw and fill rate |
| Target frame time | User's chosen platform goal; measure median and slow-frame behavior, not only an average FPS label |

Do not change the project's established budget blindly. A scene can be draw-call bound, fill-rate bound, CPU bound or memory bound before reaching a triangle target. Measure on the real target browser/device. Lower weather density, transparent card coverage and distant scenery complexity before removing essential route signs.

Asset-level triangle targets are listed in each item prompt. Those targets do not sum to a promised visible-scene total; placement density and LOD selection determine the actual cost.
