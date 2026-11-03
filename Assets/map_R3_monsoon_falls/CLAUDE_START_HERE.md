# Claude: build R3 — Monsoon Falls

## Goal

Implement this selected map in the user's existing flying-car racing game using Blender for precise construction and Meshy where useful for individual scenery assets. Use the selected concept as the art-direction reference and the individual PNGs as modeling references.

A rainy tropical ravine with wet basalt cliffs, abundant green vegetation, a large hydro dam and reservoir, a natural rock arch, broad waterfalls and a glass observation outpost.

## Read in this order

1. The actual game repository's instructions and current map implementation.
2. README.md, ASSET_INDEX.md and References/selected_map.png.
3. Guides/01_art_direction.md, Guides/02_blender_build.md and Layout/route.json.
4. Layout/placements.json and the source kit's README inside Blender.
5. Guides/03_meshy_workflow.md and the per-item prompts you actually use.
6. Guides/04_materials_weather.md, Guides/05_game_integration.md and Guides/06_validation_performance.md.

## First working milestone

Run the source validator, build a plain Blender blockout, import it through the game's existing map loader and complete an ordered lap with every car. Only then replace placeholders and add weather. Use the supplied route as an initial design; adapt it only when a concrete gameplay or integration issue requires it and rerun its validation.

~~~bash
python3 Blender/validate_routes.py --route Layout/route.json --placements Layout/placements.json
blender --factory-startup --background --python Blender/build_map.py -- --config Layout/route.json --placements Layout/placements.json --output-dir Build --export-glb
~~~

Use the installed Blender's actual executable path if it is not on PATH.

## Build priorities

Ravine and mesa blockout → dam and reservoir → open rock arch → vegetation → waterfall flow and foam → rain and localized mist.

Assemble F01/F03 as large cliff masses around the loop; use mesh detail only where the camera comes close. F02 requires a hollow collision opening. Repeat F04 structural dam modules and add F05 gates at planned spillways. Place F06 outside the flight corridor. Instance F07/F08 trees in small groups and F09/F12 around ledges. F10 boulders break up regular terrain. F11 is a waterfall look reference; construct water strips, foam and mist independently.

Suggested first optional Meshy trial: Assets/Environment/F10_moss_boulder.png. Follow the task-ledger and budget guidance before paid generation. Do not generate the entire map from the scene image.

## Preserve the game contract

Car length target: about 5.6 m, width up to 3.92 m. Measure actual model/collider bounds and camera offsets. Keep the three cars' thruster-pod animation, rear nozzle attachments, dark body repaint behavior and attack handling working. Use the existing I-key cockpit interface and verify that map/weather changes work in both camera modes.

Game layout coordinates are X right / Y up / Z forward in meters. Blender uses (X, -Z, Y). Apply the conversion once; the source exporter is Y-up. Dimensions in item notes use local Blender axes and are explicitly labeled.

Keep portals genuinely hollow in both visible and collision geometry. Use simple collision proxies. The checkpoint scoring volume is separate from the ring mesh and rim collision.

## What this handoff already contains

27 separate item PNGs, the selected full-scene reference, exact prompts, a visual gallery, modeling/collision/LOD notes, route/placement data, precise route diagrams and Blender source. Route/pack validation is recorded in QA; no paid Meshy model was generated and no Blender execution or game integration is claimed.

## Finish with a concrete implementation report

Deliver the working map files, clean production GLBs, updated game integration and actual validation results. Report the installed tool versions, accepted Meshy task IDs/costs if used, changed game files, measured render budgets and any remaining defect. Do not label the map complete because the concept PNG exists or because a source-only check passes.
