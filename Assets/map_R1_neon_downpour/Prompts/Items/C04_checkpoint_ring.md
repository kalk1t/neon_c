# C04 — Checkpoint ring

## Individual item

PNG: ../../Assets/Course/C04_checkpoint_ring.png

Type: **modeling_reference**. Reused unchanged from the approved Neon Circuit B course asset collection.

## Build specification

- **Method:** Blender: torus or segmented extruded ring with separate emissive bands.
- **Nominal dimensions:** 26 / 2.2 / 26 m (local Blender X / Y / Z)
- **Origin:** Center of the empty ring opening; local -Y points through the gate.
- **Collision:** Optional ring rim segments only. The opening is a trigger volume and must remain empty.
- **LOD0 / LOD1 / LOD2 triangle targets:** 3,200 / 1,200 / 384. These are future production targets, not measurements of delivered meshes.

22 m clear inner diameter; 26 m approximate outer diameter. Derive gate plane position and normal from the flight path. Keep scoring trigger separate from visible geometry.

All dimensions are authored design targets. The image is a visual reference and is not calibrated CAD, a mesh, a collision shape, or a PBR texture set. Use the route and clearance requirements to determine final geometry. Keep narrow cyan/magenta lights separate from neutral structural materials.

## Meshy use

Build this repeatable structural object in Blender. Meshy is optional only for decorative detail that does not determine connection geometry, clear openings, collision, or sign text. If a seed is useful, use this one isolated PNG only; clean it, set scale/pivot, create LODs and separate collision in Blender.

No paid Meshy generation was executed for this pack.

## Exact image-generation prompt

The following is the exact prompt for the included image. For reused B assets, references named in the original prompt describe the original generation context; they are not additional required runtime assets.

~~~text
Use case: stylized-concept. Create one individually usable PNG concept reference for the user's flying-car combat racing game.
Reference image 1 is a STYLE REFERENCE only. Match ONLY the CENTER COLUMN labeled B — NEON CIRCUIT. Ignore the orange/green A column and the desert C column completely. Preserve its polished stylized 3D hard-surface rendering, black anodized metal and chrome, cyan architectural edge lights, magenta luminous accents, clean modular geometry, elegant proportions and readable silhouette. This is a new isolated single-asset render, not a cropped board, not a comparison sheet, not a collage. No brands or watermarks.
Asset: ONE standalone floating circular CHECKPOINT RING. A sturdy segmented annular frame in black metal and polished chrome, slim cyan strips along the outer circumference, a bright magenta luminous inner rim, a few clean technical panel joints. Large open center is completely empty and genuinely transparent, large enough for a flying car; absolutely no glass, grid, portal surface, energy screen or backdrop inside the ring. Two very compact stabilizer housings are integrated into the lower outer edge; no pedestal, ground or road. Mostly front-facing with a slight three-quarter angle to reveal thickness without hiding the circular opening. Professional modular racing prop. Transparent PNG with genuine alpha outside the object, including any openings. One complete object, centered in a square frame, generous clear margin on every side, three-quarter product view, entire silhouette visible, no crop. Neutral soft studio highlights make black metal legible. Controlled emissive bloom with clean edges. No environment, floor, backdrop, checkerboard pattern, labels, captions, dimension lines, extra objects, or ground shadow.
~~~
