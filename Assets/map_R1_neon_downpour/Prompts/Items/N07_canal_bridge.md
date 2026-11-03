# N07 — Short canal service bridge

## Scope

Individual transparent PNG modelling reference for **R1 — Neon Downpour**. This image is concept/reference art, not a mesh, UV atlas, measured elevation, PBR texture set or finished game object. Its appearance is based on the selected R1 map concept.

## Design targets

Target width × depth × height: 26 × 18 × 6 m. These are **design targets, not measurements recovered from the image**. Use Blender dimensions to establish exact scale; do not infer a guaranteed physical clearance from the raster.

- **Construction route:** Blender required for span, supports and route clearance. Meshy optional decorative panel study only; do not use an inferred solid hull as final bridge.
- **Origin / pivot:** Center of deck top at Z=0; local X along the 26 m span, Y width, structure extends below top.
- **Collision:** Separate deck and end-support boxes. Keep the underpass empty; final usable clearance must be checked against a 5.6 m car and game route. Do not use one overall hull.
- **LOD triangle targets:** LOD0 ≤ 7,000; LOD1 ≤ 2,400; LOD2 ≤ 400. Targets apply to the whole visible asset before instancing; adjust after profiling. These are budgets for future meshes, not measured counts.
- **Materials:** keep black/graphite body and glass separate from narrow emissive cyan/magenta inlays. Use shared materials/atlases where practical. Wetness belongs in roughness/normal treatment and lighting; avoid baking the map's rain and fog into the object texture.
- **Blender orientation:** values above use Z up. Check the exported glTF's up/forward convention against the game's existing importer rather than rotating it twice.

## Suggested Meshy instruction

No Meshy generation has been executed for this package. If Meshy is used for a decorative seed, submit only this isolated item image and use:

> Create one game environment prop matching the supplied isolated reference: short canal service bridge. Neutral graphite and black glass with dark chrome hardware, narrow cyan emissive strips and small magenta accents. Preserve the subject silhouette and all visible openings. No surrounding scene, no floor, no rain or fog, no text or logos. Use a clean separated hard-surface construction suitable for later Blender cleanup. Keep emission confined to lighting elements. Scale, pivots, mesh simplification and collision will be established manually in Blender.

Build accurate structural shapes and empty clearances in Blender even when using a generated seed. Remove hidden/interior debris, repair normals and reduce geometry; apply real-world size and pivot before export. Do not submit the complete map concept as one environment mesh.

## Exact image-generation prompt

Generation mode: built-in image generator, one call for this individual asset, transparent background enabled.

Reference: selected R1 Neon Downpour concept, `References/selected_map.png` in the final package (asset_records.json retains the exact source path).

```text
Use case: stylized-concept.
Asset type: individual 3D modelling reference PNG for R1 NEON DOWNPOUR, a flying-car racing game.
Input image 1 is a STYLE REFERENCE only: extract its graphite-and-glass architecture, dark chrome structures, cyan navigation strips and restrained magenta accents. Do not reproduce its scenery, typography, track, rain, cars, atmosphere or camera layout.
Primary request: ONE short futuristic industrial service bridge module, nominal 26 metres span, 18 metres deck width and 6 metres total height. Thick dark steel deck held by two short solid end supports, leaving a clearly open rectangular passage beneath the center. Low parapet edges with narrow cyan navigation lights, dark chrome bracing, small magenta end accents, subtle wet-look metal finish. Show the complete isolated bridge with both end supports and open space below. No canal water, ground, city, roads extending away, suspended cables, vehicles, race hoops or landscape.
Style/medium: polished stylized 3D game-asset studio render; physically readable hard-surface construction with neutral dark graphite, black glass and brushed chrome. Dark body material must stay predominantly neutral; emissive colour confined to thin inlays and small details. Moderate realistic material wear, clean silhouette and useful modelling detail.
Composition/framing: exactly ONE complete asset in a single elevated front three-quarter view, weak perspective, centered with at least 10% clear transparent margin on every side, entire silhouette visible, no crop. Match aspect ratio to subject. No exploded view, variants, collage or contact sheet.
Lighting: neutral soft studio light strong enough to read every dark surface. Restrained cyan and magenta emission, no bloom, no halo, no atmospheric light rays.
Background: genuine transparent RGBA, empty alpha outside the object and through openings. No floor, shadow, ground reflection, water plane, horizon, sky, fog, rain, city backdrop or unrelated scenery.
Text: none. No labels, title, logo, watermark, UI, dimension arrows or measurement graphics. No cars, people or weapons.
```

