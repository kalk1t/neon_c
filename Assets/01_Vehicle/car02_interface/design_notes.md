# Car02 interface — 2B Twin Controls

The owner selected option **2B** from the Car02 Wedge Interceptor cockpit collection. The visual identity is a low, angular black/chrome cockpit with cyan lighting, restrained magenta weapon accents, and two independent side control grips. There is no central steering wheel or yoke.

## Asset mapping

| File | Purpose |
| --- | --- |
| `references/cockpit_preview.png` | Illustrated full cockpit concept, with example race values and Neon Circuit scenery. Use to judge the chosen appearance. |
| `artwork/cockpit_base.png` | Static cockpit foreground with a transparent windshield, blank screen surfaces, and empty grip mounts. |
| `artwork/control_left.png` | Separate driver-view left control grip and pivot mount. |
| `artwork/control_right.png` | Separate driver-view right control grip and pivot mount. |
| `panels/speed_panel.png` | Blank cyan speed-screen housing for independently placed HUD layouts. |
| `panels/boost_panel.png` | Blank cyan charge-track housing; fill and percentage should be drawn dynamically. |
| `panels/attack_panel.png` | Blank magenta weapon-status housing; icon and status should be drawn dynamically. |
| `panels/race_panel.png` | Blank cyan lap/timer screen housing. |
| `references/interior_modeling_reference.png` | Cutaway interior illustration for manual modeling of the chosen cockpit. |

## Compositing

Draw the live game world first, followed by the static cockpit base, dynamic race instruments, then the separate control grips. The base already contains instrument recesses, so the separate panel PNGs are optional reusable skins for an alternate HUD arrangement or closer instrument views. Avoid drawing a second set of panel housings over the existing bezels unless the layout intentionally needs that.

All runtime artwork and panel PNGs contain actual alpha outside their silhouettes. Screen interiors remain dark and opaque. The illustrated preview contains sample values and scenery; do not use it as the live gameplay foreground. The two grips are separate illustrations with their own framing; scale and anchor them using their visible-alpha bounds. The base's canvas is 1672 × 941. Suggested normalized screen centers are speed (0.50, 0.505), lap/time (0.307, 0.535), and boost/attack (0.701, 0.537). These are starting layout coordinates, not engine-calibrated measurements.

## Runtime behavior

Use current game state for speed, lap, elapsed time, boost charge, and attack availability/cooldown. Keep values as text and meters drawn by the game so that they update cleanly. `I` should toggle cockpit view and the existing exterior camera once per key press; repeated keydown events should not rapidly toggle it. Preserve the normal race controls while changing the view.

## Scope

These are rendered 2D components and a modeling reference, not a GLB interior mesh, calibrated CAD, normal maps, or a validated camera integration. The cutaway is an inferred interior design and needs geometry, attachment points, visibility, and scale checks when modeled against the existing Car02 mesh. The actual game repository was not modified as part of creating these assets. Root integration notes and the provided preview kit describe the package's additional interface behavior.

Each visual asset's exact generation prompt and reference lineage are in `prompts/`. `asset_records.json` records source paths, dimensions, alpha bounds, and hashes.
