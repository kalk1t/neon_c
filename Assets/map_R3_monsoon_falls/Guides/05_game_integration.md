# Game integration guide

## Inspect the real project first

The actual game repository was not supplied with this handoff. No file path, module name or engine API here claims to be an inspected project interface. Claude must locate the existing map registry/loader, render engine, coordinate convention, checkpoints/laps, collision system, reset logic, camera modes, asset packaging and disposal flow before editing code.

Read the project's AGENTS.md and existing map implementation. Reuse its conventions. Preserve the car model attachments, pods, nozzle effects, repaint behavior and input/camera system described by the user. The new maps should load through the same lifecycle as the existing map.

## Implement in small working stages

1. Add the authored centerline, gates and a plain deck using the game's current map system. Validate a full lap with ordered checkpoints and correct resets before adding scenery.
2. Import the Blender blockout/GLB using the established asset loader. Compare a known origin, up axis, forward marker and 5.6 m car against the data. Apply axis conversion exactly once.
3. Read the route and placement metadata through a small adapter. Keep the supplied files as source data; do not assume they can be dropped directly into an unknown engine API.
4. Replace placeholders with individual production meshes, sharing geometry/materials for repeated instances. Register collision using simple proxies that follow the visible walls and leave portals empty.
5. Add wet materials, canal/reservoir water, rain, splashes and mist through the existing update/render flow. Bound particle counts and dispose all resources on map change.
6. Add map-selection thumbnail/name and persist selection only if the game already has that UI. Use the selected scene as menu art if appropriate, not as the 3D map.

## Checkpoint and lap behavior

The ordered checkpoints in Layout/route.json include centers, forward normals, a clear opening and a conservative center-trigger radius. For a high-speed car, use a swept previous-to-current-position test against the next checkpoint plane. A simple distance check only at the current frame can miss a gate.

Let n be the checkpoint forward normal, c the center, and p0/p1 the previous/current car centers. Compute d0=dot(p0-c,n) and d1=dot(p1-c,n). A forward crossing has d0<0 and d1>=0. At that crossing, interpolate the point on the segment and test its radial distance in the gate plane. Award only the next required checkpoint.

Use tolerances/cooldown appropriate to the existing engine and avoid double awards while hovering on a plane. CP00 is start/finish: require the entire ordered sequence before incrementing a lap. Reverse crossings and skipped gates must not advance progress.

The initial trigger radius does not prove clearance for a tilted, yawed or rolling car. Check the actual collider envelope of all three cars. Keep the visual ring/rim collision separate from the scoring disc.

## Resets, boundaries and attacks

Reset to the last safe route position with a validated heading, zero or deliberately controlled velocity, and the correct next required gate. Preserve checkpoint progress correctly when resetting before a gate. Test the first start, every reset point and loop-wrap boundary.

Define an explicit recovery boundary or time-off-course rule using the game's existing design. A scenery water plane is not sufficient recovery logic. Keep reset points clear of towers, barriers, vegetation and other spawned racers.

Verify that attack rays/projectiles use the intended environment collision layer. Transparent water and mist should not accidentally absorb attacks as solid geometry. Keep visual neon and scenery decoration independent from hit detection.

## Rendering and lifecycle

Give the new map one owner that can dispose geometries, materials, textures, lights, weather emitters and event/update callbacks on unload. Reuse shared resources without disposing them while another instance still needs them. Test switching between both maps repeatedly and racing again.

Inspect the I-key cockpit view on every map: near planes, rain clipping, windshield visibility, signs and silhouettes. This pack does not redefine cockpit assets or I-key behavior.

## Definition of done in the actual game

- Both selected maps can be selected, loaded, raced, restarted and unloaded.
- All three cars complete multiple ordered laps in external and cockpit views.
- Track seams, gates, portals, starts and resets have measured clearance.
- No invisible collision spans the skybridge or natural arch opening.
- Effects do not obscure the next gate or cause unacceptable frame time.
- The final scene resembles the chosen reference's landmarks, palette and weather.
- Report the actual device/browser, render resolution, measured frame times, draw calls, triangles, texture memory and remaining defects. Do not replace measured results with the suggested budgets in this pack.
