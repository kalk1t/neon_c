# Claude: build R1 — Neon Downpour

## Goal

Implement this selected map in the user's existing flying-car racing game using Blender for precise construction and Meshy where useful for individual scenery assets. Use the selected concept as the art-direction reference and the individual PNGs as modeling references.

A rain-soaked night city with dark glass towers, cyan facade ribs, restrained magenta accents, elevated wet decks, a monumental twin-tower skybridge and a lower canal.

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

City massing → skybridge portal → canal walls and water → rooftop detail → distant skyline → rain and wet reflections.

Place N01/N02/N04 as simple building masses first, then replace or detail them. N03 is a three-part portal with two tower colliders and one bridge collider. N05 receives N08 roof vents. N09 antennas belong on roof planes. N06 walls repeat around the canal; N07 bridges cross below the flight path. N10 stays blank until game text is added. N11 sits at a service platform outside the racing line. N12 is a distant silhouette asset, never a collision wall across the course.

Suggested first optional Meshy trial: Assets/Environment/N11_energy_cabinet.png. Follow the task-ledger and budget guidance before paid generation. Do not generate the entire map from the scene image.

## Preserve the game contract

Car length target: about 5.6 m, width up to 3.92 m. Measure actual model/collider bounds and camera offsets. Keep the three cars' thruster-pod animation, rear nozzle attachments, dark body repaint behavior and attack handling working. Use the existing I-key cockpit interface and verify that map/weather changes work in both camera modes.

Game layout coordinates are X right / Y up / Z forward in meters. Blender uses (X, -Z, Y). Apply the conversion once; the source exporter is Y-up. Dimensions in item notes use local Blender axes and are explicitly labeled.

Keep portals genuinely hollow in both visible and collision geometry. Use simple collision proxies. The checkpoint scoring volume is separate from the ring mesh and rim collision.

## What this handoff already contains

27 separate item PNGs, the selected full-scene reference, exact prompts, a visual gallery, modeling/collision/LOD notes, route/placement data, precise route diagrams and Blender source. Route/pack validation is recorded in QA; no paid Meshy model was generated and no Blender execution or game integration is claimed.

## Finish with a concrete implementation report

Deliver the working map files, clean production GLBs, updated game integration and actual validation results. Report the installed tool versions, accepted Meshy task IDs/costs if used, changed game files, measured render budgets and any remaining defect. Do not label the map complete because the concept PNG exists or because a source-only check passes.
