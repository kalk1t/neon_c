# C05 — Start / finish arch

## Individual item

PNG: ../../Assets/Course/C05_start_finish_arch.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: three structural beams with instanced decorative panels and a separate sign.
- **Nominal dimensions:** 28 / 3 / 26 m (local Blender X / Y / Z)
- **Origin:** Center of the clear passage at the route flight height; local -Y forward.
- **Collision:** Separate posts and lintel only; no collider filling the opening.
- **LOD0 / LOD1 / LOD2 triangle targets:** 4,500 / 1,600 / 400. These are future production targets, not measurements of delivered meshes.

Author a clear passage at least 22 m wide by 22 m high. The source includes START lettering. Rebuild its sign as runtime text or a clean decal so it can display START/FINISH and localize; do not ask Meshy to reconstruct readable lettering.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Create one individually usable PNG concept reference for the user's flying-car combat racing game.
Reference image 1 is a STYLE REFERENCE only. Match ONLY the CENTER COLUMN labeled B — NEON CIRCUIT. Ignore the orange/green A column and the desert C column completely. Preserve its polished stylized 3D hard-surface rendering, black anodized metal and chrome, cyan architectural edge lights, magenta luminous accents, clean modular geometry, elegant proportions and readable silhouette. This is a new isolated single-asset render, not a cropped board, not a comparison sheet, not a collage. No brands or watermarks.
Asset: ONE modular floating START/FINISH GANTRY for an aerial racing course. A wide angular inverted-U frame of black anodized metal and chrome, heavy top corners and two slender side posts, cyan outer light strips, magenta inner accents. Centered on the top crossbar is the single exact word "START" in clean luminous cyan-white block letters. Underneath the header is a generous EMPTY open rectangular flight aperture wide enough for two flying cars, fully transparent; no force field, glass, energy wall, barrier, sign or road across that opening. Each post terminates in a compact integrated anti-gravity stabilizer pod; no terrain or supporting city building. Practical symmetrical reusable game prop, strong clean silhouette. Transparent PNG with genuine alpha outside the object, including any openings. One complete object, centered in a square frame, generous clear margin on every side, three-quarter product view, entire silhouette visible, no crop. Neutral soft studio highlights make black metal legible. Controlled emissive bloom with clean edges. No environment, floor, backdrop, checkerboard pattern, labels, captions, dimension lines, extra objects, or ground shadow. The word START is the only allowed text.
~~~
