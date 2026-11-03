# Materials and weather

## Material authoring

Materials/material_recipes.json contains proposed starting values, not measured physical constants and not generated PBR maps. Keep large surfaces neutral: black/graphite for the course and city, wet gray basalt and green foliage for the ravine. Cyan marks the route; magenta provides secondary accents.

Build thin light strips as their own material slots or masks. Use controlled emissive output and restrained bloom so the road edge, car silhouette and checkpoint opening remain readable. Avoid attaching one dynamic light to every glowing strip.

Use roughness variation to suggest rain-wet surfaces. The source cutout PNGs are lit three-quarter illustrations; do not wrap them around meshes as base-color textures. Rebuild materials from the recipe, bake authored detail, or use cleaned Meshy texture output. Keep highlights and rain out of albedo wherever practical.

For glTF's metal/rough workflow, base color and emissive textures are color data; packed metallic/roughness and normal inputs are data maps. The maintained Blender exporter guide describes the supported node connections and packing. Inspect those settings against the installed exporter before final export. The full material references are linked in SOURCES.md.

## Rain

Use C11 as a source for a small alpha rain sprite. Emit or recycle a bounded camera-local volume; animate velocity relative to the camera and car motion. Start with hundreds, not tens of thousands, of visible streaks and profile. The density settings are in Weather/weather_recipe.json as authoring targets.

Keep rain outside the cockpit interior. If the game supports switching the camera with I, test the weather from both views. Rain on a windshield should be a dedicated optional effect, separate from the exterior particle volume. Use the existing input/camera system rather than introducing a second I-key handler.

Use C12 for a short pooled splash animation only near visible wet surfaces. Grow and fade the sprite over roughly a fraction of a second. Disable depth writing for transparent particles and choose blending/depth sorting to match the engine. Check halos on black, gray and bright sky backgrounds; import alpha with the correct premultiplication settings.

## Mist, spray and waterfalls

Use C13 as a neutral mist source. Sparse cards near water impact zones and lower ravine pockets are sufficient. Fade them with camera distance; avoid large stacks of transparent full-screen cards.

For Monsoon Falls, F11 is a **waterfall look reference**, not a validated tiling texture, normal map, animation sheet or solid 3D model. Build the waterfall as one or several slightly curved vertical strips. Give each strip long UVs, scroll a suitable authored flow texture downward, vary speed/opacity slightly between layers and add narrow foam strips at the top and bottom. Use C13 for localized impact spray and C12 as a secondary ripple/splash source.

Start with simple translucent/unlit or lit water that the actual engine supports. Keep water collision separate from the visual curtain. Never block the race corridor with a solid Meshy-generated waterfall. Keep the route beside the waterfall until visibility and steering have been tested; any later pass-through section needs explicit gameplay validation.

## Water surfaces, fog and lighting

Create canal/reservoir water as simple geometry with inexpensive animated normal/roughness behavior if the engine already supports it. Avoid scene-wide real-time planar reflections by default; use environment lighting or the game's established water implementation.

The starter water plane is a placement/color reference. It has no runtime ripple shader or rain physics. Likewise, the Blender camera lighting is for blockout inspection; recreate the chosen light/fog mood in the game and verify the export.

Apply fog to support depth without hiding the next checkpoint. Lower-cost mode can reduce rain count, disable splashes and simplify mist while keeping the same course geometry. All values are suggested starting points; final tuning belongs in a real gameplay profile.
